import logging; logging.basicConfig(level=logging.INFO)

import asyncio, os, json, time
from datetime import datetime

from aiohttp import web
from jinja2 import Environment, FileSystemLoader

import orm
from coroweb import add_routes, add_static
from handlers import cookie2user, COOKIE_NAME
'''
async web application.
'''

# 我这里有一点非常好奇的是，/api/这一类的函数究竟是如何被调用的。
def init_jinja2(app, **kw):
    '''
    初始化jinja2
    '''
    logging.info('init jinja2...')
    options = dict(
        autoescape = kw.get('autoescape', True),
        block_start_string = kw.get('block_start_string', '{%'),
        block_end_string = kw.get('block_end_string', '%}'),
        variable_start_string = kw.get('variable_start_string', '{{'),
        variable_end_string = kw.get('variable_end_string', '}}'),
        auto_reload = kw.get('auto_reload', True)
    )
    path = kw.get('path', None)
    if path is None:
        path = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'templates')
    logging.info('set jinja2 template path: %s' % path)
    env = Environment(loader=FileSystemLoader(path), **options) # options是一些选项
    filters = kw.get('filters', None)
    if filters is not None:
        for name, f in filters.items(): # filter是一个字典类型喽！
            env.filters[name] = f # 添加过滤器
    app['__templating__'] = env # 模版


async def logger_factory(app, handler):
    async def logger(request):
        logging.info('Request: %s %s' % (request.method, request.path))
        return (await handler(request))
    return logger # 返回一个装饰器

async def data_factory(app, handler):
    async def parse_data(request):
        if request.method == 'POST': # 需要提交数据
            if request.content_type.startswith('application/json'):
                request.__data__ = await request.json() # 记录下数据
                logging.info('request json: %s' % str(request.__data__))
            elif request.content_type.startswith('application/x-www-form-urlencoded'):
                request.__data__ = await request.post()
                logging.info('request form: %s' % str(request.__data__))
        return (await handler(request))
    return parse_data

async def response_factory(app, handler):
    '''
    中间件，我的感觉是handler会依次通过这些中间件函数，有中间件函数来对handler来做一些处理
    '''
    async def response(request):
        logging.info('Response handler...')
        r = await handler(request) # handler函数接收一个request，然后返回一个response对象
        # r是返回的response对象，事实上，这个函数是最后才调用
        if isinstance(r, web.StreamResponse):
            return r
        if isinstance(r, bytes):
            resp = web.Response(body=r)
            resp.content_type = 'application/octet-stream'
            return resp
        if isinstance(r, str): # 在这个程序中，重定向一般返回一段字符，以redirect：开头
            if r.startswith('redirect:'):
                return web.HTTPFound(r[9:]) # HTTPFound对应的状态码应该是302
            resp = web.Response(body=r.encode('utf-8')) # 如果不是以redirect：开头，那么，直接返回这段文本
            resp.content_type = 'text/html;charset=utf-8'
            return resp
        if isinstance(r, dict): # 如果返回了一个字典的话
            template = r.get('__template__')
            if template is None: # 并且r中并没有关于template的信息的话，就会将返回的dict数据以json的形式dump出去
                resp = web.Response(body=json.dumps(
                    r,
                    ensure_ascii=False,
                    default=lambda o: o.__dict__).encode('utf-8')
                )
                resp.content_type = 'application/json;charset=utf-8'
                return resp
            else: # 渲染页面
                # 首先加入用户的信息
                r['__user__'] = request.__user__
                resp = web.Response(body=app['__templating__'].get_template(template).render(**r).encode('utf-8'))
                resp.content_type = 'text/html;charset=utf-8'
                return resp
        if isinstance(r, int) and r >= 100 and r < 600:
            return web.Response(status=r) # 返回状态码
        if isinstance(r, tuple) and len(r) == 2: # 如果返回了一个元组
            t, m = r # t代表状态码，然后m代表body的信息
            if isinstance(t, int) and t >= 100 and t < 600:
                return web.Response(status=t, body=str(m))
            # default:
        resp = web.Response(body=str(r).encode('utf-8'))
        resp.content_type = 'text/plain;charset=utf-8'
        return resp
    return response

async def auth_factory(app, handler):
    async def auth(request):
        logging.info('check user: %s %s' % (request.method, request.path))
        request.__user__ = None
        cookie_str = request.cookies.get(COOKIE_NAME) # 得到cookies的值
        if cookie_str:
            user = await cookie2user(cookie_str) # 得到cookies里面存储的用户的信息
            if user:
                logging.info('set current user:%s' % user.email)
                request.__user__ = user # 存储用户的信息
        if request.path.startswith('/manage/') and \
            (request.__user__ is None or not request.__user__.admin):
            return web.HTTPFound('/signin') # 重定向至登陆界面
        return (await handler(request))
    return auth

def datetime_filter(t):
    delta = int(time.time() - t)
    if delta < 60:
        return u'1分钟前'
    if delta < 3600:
        return u'%s分钟前' % (delta // 60)
    if delta < 86400:
        return u'%s小时前' % (delta // 3600)
    if delta < 604800:
        return u'%s天前' % (delta // 86400)
    dt = datetime.fromtimestamp(t)
    return u'%s年%s月%s日' % (dt.year, dt.month, dt.day)

async def init(loop):
    await orm.create_pool(
        loop=loop,
        host='127.0.0.1',
        port=3306,
        user='root',
        password='1234',
        db='awesome'
    )

    app = web.Application(loop=loop,
                          middlewares=[logger_factory, auth_factory, response_factory]
    ) # 有一个疑问，那就是middlewares究竟是干什么用的？中间件，用于处理handle
    # 好似不是这样的。

    init_jinja2(app, filters=dict(datetime=datetime_filter))
    from coroweb import add_routes, add_static
    add_routes(app, 'handlers')
    add_static(app)
    srv = await loop.create_server(app.make_handler(), '127.0.0.1', 9001)
    logging.info('server started at http://127.0.0.1:9001...')
    return srv

loop = asyncio.get_event_loop()
loop.run_until_complete(init(loop))
loop.run_forever()
