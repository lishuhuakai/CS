'url handlers'

import re, time, json, logging, hashlib, base64, asyncio

from coroweb import get , post
import markdown2
from model import User, Comment, Blog, next_id
from config import configs
from apis import APIValueError, APIResourceNotFoundError, APIPermissionError, Page
from aiohttp import web
# @get('/')
# async def index(request):
#     print('I am here!')
#     users = await User.findAll()
#     temp = users
#     return {
#         '__template__' : 'test.html',
#         'users' : users
#     }

@get('/')
async def index(*, page='1'):
    page_index = get_page_index(page)
    num = await Blog.findNumber('count(id)')
    page = Page(num)
    if num == 0:
        blogs = []
    else:
        blogs = await Blog.findAll(
            orderBy='created_at desc',
            limit=(page.offset, page.limit)
        )
    blogs = await Blog.findAll()
    return {
        '__template__': 'blogs.html',
        'blogs': blogs,
        'page' : page
    }

# @get('/api/users')
# async def api_get_users():
#     users = await User.findAll(orderBy='created_at desc')
#     for u in users:
#         u.passwd = '*******'
#     return dict(users=users)

_RE_EMAIL = re.compile(r'^[a-z0-9\.\-\_]+\@[a-z0-9\-\_]+(\.[a-z0-9\-\_]+){1,4}$')
_RE_SHA1 =  re.compile(r'^[0-9a-f]{40}$')

COOKIE_NAME = 'awesession'
_COOKIE_KEY = configs.session.secret

def user2cookie(user, max_age):
    '''
    Generate cookie str by user.
    将用户的数据转换成为cookie数据。
    '''
    # build cookie string by : id-expires-sha1
    expires = str(int(time.time() + max_age)) # 过期时间
    s = '%s-%s-%s-%s' % (user.id, user.passwd, expires, _COOKIE_KEY)
    L = [user.id, expires, hashlib.sha1(s.encode('utf-8')).hexdigest()]
    return '-'.join(L)

async def cookie2user(cookie_str):
    '''
    Parse cookie and load user if cookie is valid
    将cookie数据转换成为用户的数据。
    '''
    if not cookie_str:
        return None
    try:
        L = cookie_str.split('-')
        if len(L) != 3:
            return None
        uid, expires, sha1 = L
        if int(expires) < time.time():
            # cookie过期了
            return None
        user = await User.find(uid)
        if user is None:
            return None
        s = '%s-%s-%s-%s' % (uid, user.passwd, expires, _COOKIE_KEY)
        if sha1 != hashlib.sha1(s.encode('utf-8')).hexdigest():
            logging.info('invalid sha1')
            return None
        user.passwd = '******' # 这里面要隐藏用户的密码数据
        return user
    except Exception as e:
        logging.exception(e)
        return None

@get('/register')
def register():
    # 注册界面
    return {
        '__template__' : 'register.html'
    }

@get('/signin')
def signin():
    # 登陆界面
    return {
        '__template__' : 'signin.html'
    }

@post('/api/authenticate')
async def authenticate(*, email, passwd):
    '''
    这个函数主要是用于验证
    '''
    if not email:
        raise APIValueError('email', 'Invalid email')
    if not passwd:
        raise APIValueError('passwd', 'Invalid password')
    users = await User.findAll('email=?', [email])
    if len(users) == 0:
        raise APIValueError('email', 'Email not exist.')
    user = users[0]
    # check passwd:
    sha1 = hashlib.sha1()
    sha1.update(user.id.encode('utf-8'))
    sha1.update(b':')
    sha1.update(passwd.encode('utf-8'))
    if user.passwd != sha1.hexdigest():
        raise APIValueError('passwd', 'Invalid password')
    # authenticate ok, set cookie:
    r = web.Response()
    r.set_cookie(COOKIE_NAME, user2cookie(user, 86400), max_age=86400, httponly=True)
    user.passwd = '******'
    r.content_type = 'application/json'
    r.body = json.dumps(user, ensure_ascii=False).encode('utf-8')
    return r

@get('/signout')
def signout(request):
    ''' 退出界面 '''
    referer = request.headers.get('Referer')
    r = web.HTTPFound(referer or '/')
    r.set_cookie(COOKIE_NAME, '_deleted-', max_age=0, httponly=True) # 设置cookie
    logging.info('user signed out.')
    return r

@post('/api/users')
async def api_register_user(*, email, name, passwd):
    '''
    这个函数其实是用来注册用户的。
    '''
    if not name or not name.strip():
        raise APIValueError('name')
    if not email or not _RE_EMAIL.match(email):
        raise APIValueError('email')
    if not passwd or not _RE_SHA1.match(passwd):
        raise APIValueError('passwd')
    uid = next_id()
    sha1_passwd = '%s:%s' % (uid, passwd)
    user = User(
        id=uid,
        name=name.strip(),
        email=email,
        passwd=hashlib.sha1(sha1_passwd.encode('utf-8')).hexdigest(),
        image='http://www.gravatar.com/avatar/%s?d=mm&s=120' % hashlib.md5(email.encode('utf-8')).hexdigest()
    )

    await user.save()
    # make session cookie:
    r = web.Response()
    # 登陆的时候要创建cookie信息
    r.set_cookie(COOKIE_NAME, user2cookie(user, 86400), max_age=86400, httponly=True)
    user.passwd = '******'
    r.content_type = 'application/json'
    r.body = json.dumps(user, ensure_ascii=False).encode('utf-8')
    return r

@get('/manage/blogs/create')
def manage_create_blog():
    return {
        '__template__' : 'manage_blog_edit.html',
        'id' : '',
        'action' : '/api/blogs'
    }

@get('/api/blogs/{id}')
async def api_get_blog(*, id):
    blog = await Blog.find(id)
    # 通过id号寻找,需要注意的是blog虽然是Blog类型，但是Blog类型其实也是dict类型的子类。
    # 也就是说，blog其实也是一个字典
    return blog

def check_admin(request):
    if request.__user__ is None or not request.__user__.admin:
        # 错误
        raise APIPermissionError()

def get_page_index(page_str):
    p = 1
    try:
        p = int(page_str)
    except ValueError as e:
        pass
    if p < 1:
        p = 1
    return p

@post('/api/blogs')
async def api_create_blog(request, *, name, summary, content):
    '''处理新建的博文'''
    check_admin(request) # 首先要求是管理员
    if not name or not name.strip():
        raise APIValueError('name', 'name cannot be empty.')
    if not summary or not summary.strip():
        raise APIValueError('summary', 'summary cannot be empty.')
    if not content or not content.strip():
        raise APIValueError('content', 'content cannot be empty.')
    blog = Blog(
        user_id= request.__user__.id,
        user_name=request.__user__.name,
        user_image=request.__user__.image,
        name=name.strip(),
        summary=summary.strip(),
        content=content.strip()
    )
    await blog.save()
    return blog

def text2html(text):
    '''
    将文本内容转换成为html
    '''
    lines = map(lambda s: '<p>%s</p>' % s.
                replace('&', '&amp;').
                replace('<', '&lt;').
                replace('>', '&gt;'),
                filter(lambda s: s.strip() != '', text.split('\n'))
            )
    return ''.join(lines)

@get('/api/blogs')
async def api_blogs(*, page='1'):
    '''这个函数，主要是用来获取博文。'''
    page_index = get_page_index(page)
    num = await Blog.findNumber('count(id)')
    p = Page(num, page_index)
    if num == 0:
        return dict(page=p, blogs=())
    blogs = await Blog.findAll(
        orderBy='created_at desc',
        limit=(p.offset, p.limit)
    )
    return dict(page=p, blogs=blogs)

@get('/blog/{id}')
async def get_blog(id):
    blog = await Blog.find(id)
    comments = await Comment.findAll('blog_id=?', [id], orderBy='created_at desc')
    for c in comments:
        c.html_content = text2html(c.content)
    blog.html_content = markdown2.markdown(blog.content)
    return {
        '__template__' : 'blog.html',
        'blog': blog,
        'comments': comments
    }

@post('/api/blogs/{id}/comments')
async def api_create_comment(id, request, *, content):
    '''这个函数主要是用来管理创建的评论的'''
    user = request.__user__
    if user is None:
        raise APIPermissionError('Please signin first.')
    if not content or not content.strip():
        raise APIValueError('content')
    blog = await Blog.find(id)
    if blog is None:
        raise APIResourceNotFoundError('Blog')
    comment = Comment(
        blog_id=blog.id,
        user_id=user.id,
        user_name=user.name,
        user_image=user.image,
        content=content.strip()
    )
    await comment.save()
    return comment

@get('/manage/blogs')
def manage_blogs(*, page='1'):
    return {
        '__template__' : 'manage_blogs.html',
        'page_index' : get_page_index(page)
    }

@get('/manage/blogs/edit')
def manage_edit_blog(*, id):
    '''这个函数主要用来发送修改博文的页面'''
    return {
        '__template__' : 'manage_blog_edit.html',
        'id' : id,
        'action' : '/api/blogs/%s' % id
    }

@get('/manage/users')
def manage_users(*, page='1'):
    return {
        '__template__' : 'manage_users.html',
        'page_index' : get_page_index(page)
    }

@get('/api/users')
async def api_get_users(*, page='1'):
    '''这个函数主要用来获取用户的数据'''
    page_index = get_page_index(page)
    num = await User.findNumber('count(id)')
    p = Page(num, page_index)
    if num == 0:
        return dict(page=p, users=())
    users = await User.findAll(
        orderBy='created_at desc',
        limit=(p.offset, p.limit)
    )
    for u in users:
        u.passwd = '******'
    return dict(page=p, users=users)

@post('/api/blogs/{id}')
async def api_update_blog(id, request, *, name, summary, content):
    '''这个函数主要用来更新博文'''
    check_admin(request)
    blog = await Blog.find(id)
    if not name or not name.strip():
        raise APIValueError('name', 'name cannot be empty.')
    if not summary or not summary.strip():
        raise APIValueError('summary', 'summary cannot be empty.')
    if not content or not content.strip():
        raise APIValueError('content', 'content cannot be empty.')
    blog.name = name.strip()
    blog.summary = summary.strip()
    blog.content = content.strip()
    await blog.update()
    # 非常有意思的是，这个马上定位到了对应的页面
    return blog

@get('/manage/comments')
def manage_comments(*, page='1'):
    return {
        '__template__' : 'manage_comments.html',
        'page_index' : get_page_index(page)
    }

@get('/api/comments')
async def api_comments(*, page='1'):
    page_index = get_page_index(page)
    num = await Comment.findNumber('count(id)')
    p = Page(num, page_index)
    if num == 0:
        return dict(page=p, comments=())
    comments = await Comment.findAll(
        orderBy='created_at desc',
        limit=(p.offset, p.limit)
    )
    return dict(page=p, comments=comments)

@post('/api/blogs/{id}/delete')
async def api_delete_blog(request, *, id):
    check_admin(request)
    blog = await Blog.find(id)
    await blog.remove()
    return dict(id=id)

@post('/api/comments/{id}/delete')
async def api_delete_comments(id, request):
    check_admin(request)
    c = await Comment.find(id)
    if c is None:
        raise APIResourceNotFoundError('Comment')
    await c.remove()
    return dict(id=id)

@get('/manage/')
def manage():
    '''重定向'''
    return 'redirect:/manage/comments'