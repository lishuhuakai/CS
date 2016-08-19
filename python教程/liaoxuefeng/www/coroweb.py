
import asyncio, os, inspect, logging, functools
from urllib import parse
from aiohttp import web
from apis import APIError

def get(path):
    '''
    定义装饰器wrapper @get('/path')
    '''
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kw):
            return func(*args, **kw)
        wrapper.__method__ = 'GET'
        wrapper.__route__ = path
        return wrapper
    return decorator

def post(path):
    '''
    定义装饰器 @post('/path')
    '''
    def decorator(func):
        @functools.wraps(func)
        def wrapper(*args, **kw):
            return func(*args, **kw)
        wrapper.__method__ = 'POST' # post方法
        wrapper.__route__ = path
        return wrapper
    return decorator

def get_required_kw_args(fn):
    '''
    返回一个元组，里面装满了参数的名称,需要注意的是，这里的参数是要求我们赋值的
    这也是为什么函数的名字是required的原因
    '''
    args = []
    params = inspect.signature(fn).parameters # parameters是一个字典
    for name, param in params.items():
        if param.kind == inspect.Parameter.KEYWORD_ONLY \
                and param.default == inspect.Parameter.empty:
            # 这里在搜集这样的形参，首先，只能通过key=value这种形式给形参赋值
            # 然后，他们的默认值都为空
            args.append(name)
    return tuple(args)

def get_named_kw_args(fn):
    '''
    得到参数的名称,就算这些实参有默认值也没有什么关系。
    '''
    args = []
    params = inspect.signature(fn).parameters
    for name, param in params.items():
        if param.kind == inspect.Parameter.KEYWORD_ONLY:
            args.append(name)
    return tuple(args)

def has_named_kw_args(fn):
    '''
    判断参数列表中是否存在有名参数，如key='id'
    '''
    params = inspect.signature(fn).parameters
    for name, param in params.items():
        if param.kind == inspect.Parameter.KEYWORD_ONLY:
            # KEYWORD_ONLY类型的参数，只能通过key=value这种形式来赋值
            return True

def has_var_kw_arg(fn):
    '''
    任意数量的关键字参数是否存在？说白了，就是**kw这种形参是否存在？
    '''
    params = inspect.signature(fn).parameters
    for name, param in params.items():
        if param.kind == inspect.Parameter.VAR_KEYWORD:
            return True

def has_request_arg(fn):
    '''
    判断函数是否有request参数
    '''
    sig = inspect.signature(fn)
    params = sig.parameters
    found = False
    for name, param in params.items():
        if name == 'request': # 请求？
            found = True
            continue
        if found and \
                (param.kind != inspect.Parameter.VAR_POSITIONAL and \
                 param.kind != inspect.Parameter.KEYWORD_ONLY and \
                 param.kend != inspect.Parameter.VAR_KEYWORD):
            raise ValueError('request parameter must be the last named parameter'
                             ' in function: %s%s'  % (fn.__name__, str(sig)))
    return found

# RequestHandler目的是从URL函数中分析出其要接收的参数，从request中获取必要的参数
# URL函数不移动是一个coroutine,因此我们用RequestHandler()来封装一个URL处理函数
# 调用URL函数，然后将结果转化为web.Response对象，这样就晚会符合aiohttp框架的需求了.

class RequestHandler(object):

    def __init__(self, app, fn):
        self._app = app
        self._func = fn # 对应的处理函数
        self._has_request_arg = has_request_arg(fn) # 是否存在request参数
        self._has_var_kw_arg = has_var_kw_arg(fn) # 是否存在**kw这种类型，也就是字典类型的参数
        self._has_named_kw_args = has_named_kw_args(fn) # 是否存在有名参数
        self._named_kw_args = get_named_kw_args(fn) # 有名参数中有默认值或者没有默认值的参数的list
        self._required_kw_args = get_required_kw_args(fn) # 有名参数中没有默认值的参数的list
    # __call__方法可以将类变为函数，即RequestHandler(request)是合法的，实际上调用的是
    # RequestHandler的__call__方法
    async def __call__(self, request):
        kw = None
        if self._has_var_kw_arg or \
            self._has_named_kw_args or \
            self._required_kw_args:
            if request.method == 'POST':
                if not request.content_type:
                    # 丢失了编码的信息
                    return web.HTTPBadRequest('Missing Contend-type!')
                ct = request.content_type.lower() # 将content_type转化为小写
                if ct.startswith('application/json'):
                    params = await request.json()
                    if not isinstance(params, dict): # params必须是字典类型
                        return web.HTTPBadRequest('JSON body must be object!')
                    kw = params
                elif ct.startswith('application/x-www-form-urlencoded') or \
                    ct.startswith('multipart/form-data'):
                    params = await request.post()
                    kw = dict(**params)
                else:
                    return web.HTTPBadRequest('Unsupported Content-Type: %s' % request.content_type)

            if request.method == 'GET': # get方法
                qs = request.query_string
                if qs:
                    kw = dict()
                    for k, v in parse.parse_qs(qs, True).items():
                        kw[k] = v[0]
        if kw is None:
            kw = dict(**request.match_info) # 首先，你得明白，match_info是一个什么东西
        else:
            if not self._has_var_kw_arg and self._named_kw_args:
                # remove all unamed kw:
                copy = dict()
                for name in self._named_kw_args:
                    if name in kw:
                        copy[name] = kw[name]
                kw = copy
            # check named arg:
            for k, v in request.match_info.items():
                if k in kw:
                    logging.warning('Duplicate arg name in named arg and kw args: %s' %k)
                kw[k] = v

        if self._has_request_arg: # 如果存在**kw这种类型，也就是字典类型的参数
            kw['request'] = request
        # check required kw:
        if self._required_kw_args: # 检查需要赋值的参数列表
            for name in self._required_kw_args:
                if not name in kw:
                    return web.HTTPBadRequest('Missing argument: %s' % name)
        logging.info('call with args: %s' %str(kw))

        try:
            r = await self._func(**kw) # 执行函数
            return r # 返回对应的值
        except APIError as e:
            return dict(error=e.error, data=e.data, message=e.message)

def add_static(app):
    '''
    这个函数其实最为简单了，就是添加
    '''
    path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'static')
    app.router.add_static('/static/', path)
    logging.info('add static %s => %s' % ('/static/',  path))

def add_route(app, fn):
    method = getattr(fn, '__method__', None) # 得到调用的方法
    path = getattr(fn, '__route__', None) # 得到路由？
    if path is None or method is None:
        raise ValueError('@get or @post not defined in %s.' % str(fn))
    if not asyncio.iscoroutinefunction(fn) and not \
            inspect.isgeneratorfunction(fn):
        fn = asyncio.coroutine(fn)
    logging.info('add route %s %s => %s (%s)' \
                     % (method, path, fn.__name__, ', '.join(inspect.signature(fn).parameters.keys())))
    app.router.add_route(method, path, RequestHandler(app, fn)) # 这里应该有一个要求，那就是RequestHandler应该是可以直接调用的

def add_routes(app, module_name):
    # str.rfind(sub[, start[, end]])
    # Return the highest index in the string where substring sub is found,
    # such that sub is contained within s[start:end].
    # Optional arguments start and end are interpreted as in slice notation.
    # Return -1 on failure.
    n = module_name.rfind('.') # 逆向查找，返回下标值
    if n == (-1):
        mod = __import__(module_name, globals(), locals()) # 导入模块
    else:
        name = module_name[n+1:]
        mod = getattr(__import__(module_name, globals(), locals(), [name]), name)
    for attr in dir(mod):
        if attr.startswith('_'): # 不要那些内置的方法吧。
            continue
        fn = getattr(mod, attr)
        if callable(fn):
            method = getattr(fn, '__method__', None)
            path = getattr(fn, '__route__', None)
            if method and path:
                add_route(app, fn)



