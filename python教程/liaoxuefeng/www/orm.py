import asyncio, logging
import aiomysql

def log(sql, args=()):
    logging.info('SQL: %s' % sql)

async def create_pool(loop, **kw):
    logging.info('create database connection pool ...')
    global __pool # 一个全局的连接池
    __pool = await aiomysql.create_pool(
        host=kw.get('host', 'localhost'), # 如果找不到，就默认本地
        port=kw.get('port', 3306), # 如果找不到，用3306
        user=kw['user'],
        password=kw['password'],
        db=kw['db'],
        charset=kw.get('charset', 'utf8'),
        autocommit=kw.get('autocommit', True),
        maxsize=kw.get('maxsize', 10),
        minsize=kw.get('minsize', 3),
        loop=loop
    )
    print('pool.freesize = ', __pool.freesize)

async def select(sql, args, size=None):
    log(sql, args) # 先记录下来
    # 异步等待连接池对象返回可以连接线程，with语句则封装了清理（关闭conn）和处理异常的工作
    global __pool
    async with __pool.get() as conn: # 得到一个连接
        # DictCursor--字典游标，即返回的结果是一个字典
        async with conn.cursor(aiomysql.DictCursor) as cur:
            await cur.execute(sql.replace('?', '%s'), args or ())
            if size:
                rs = await cur.fetchmany(size) # rs 是一个字典的list，这样的话，会简单很多
            else:
                rs = await cur.fetchall()
        logging.info('rows returned: %s' % len(rs))
        return rs

# async def execute(sql, args, autocommit=True):
#     log(sql, args)
#     print('sql -> ' , sql)
#     async with __pool.get() as conn:
#         try:
#             cur = await conn.cursor()
#             # print(sql.replace('?', '%s'))
#             # print(sql)
#             await cur.execute(sql.replace('?', '%s'), args)
#             affected = cur.rowcount
#             print('affected -> ', affected)
#             await cur.close()
#         except BaseException as e:
#             raise
#         return affected

async def execute(sql, args, autocommit=True):
    log(sql, sql)
    async with __pool.get() as conn: # 得到一个连接
        if not autocommit:
            await conn.begin() # 这个玩意返回一个transaction，很明显，可能会有rollback
        try:
            async with conn.cursor(aiomysql.DictCursor) as cur:
                await cur.execute(sql.replace('?', '%s'), args) # 这个玩意实现替换，是吧！
                affected = cur.rowcount
            if not autocommit:
                await conn.commit()
        except BaseException as e:
            if not autocommit: # 如果不是自动提交
                await conn.rollback() # 要回滚
            raise e
        return affected

def create_args_string(num):
    L = []
    for n in range(num):
        L.append('?')
        # 我这里给出一个例子 ', '.join(L),如果L里面放入了3个'?'
        # 那么返回的结果是 ?, ?, ?
    return ', '.join(L)

class Field(object):
    def __init__(self, name, column_type, primary_key, default):
        self.name = name # 姓名
        self.column_type = column_type # 类型
        self.primary_key = primary_key # 是否为主键，bool类型
        self.default = default # 默认值

    def __str__(self):
        return '<%s, %s:%s>' % (self.__class__.__name__, self.column_type, self.name)

class StringField(Field):
    '''  字符域 '''
    def __init__(self, name=None, primary_key=False, default=None, ddl='varchar(100)'):
        #assert name==None
        super().__init__(name, ddl, primary_key, default)

class BooleanField(Field):
    ''' 布尔域 '''
    def __init__(self, name=None, default=False):
        super().__init__(name, 'boolean', False, default)


class IntegerField(Field):
    ''' 整数域 '''
    def __init__(self, name=None, primary_key=False, default=0.0):
        super().__init__(name, 'real', primary_key, default)

class FloatField(Field):
    ''' 浮点数域 '''
    def __init__(self, name=None, primary_key=False, default=0.0):
        super().__init__(name, 'real', primary_key, default)

class TextField(Field):
    ''' 文本域 '''
    def __init__(self, name=None, default=None):
        super().__init__(name, 'text', False, default)

# __new__：创建对象时调用，返回当前对象的一个实例
# __init__：创建完对象后调用，对当前对象的实例的一些初始化，无返回值
#
class ModelMetaclass(type):
    ''' 元类型 '''
    def __new__(cls, name, bases, attrs):
        if name == 'Model': # 模型
            return type.__new__(cls, name, bases, attrs)
        tableName = attrs.get('__table__', None) or name # 得到表单的名字
        logging.info('found model: %s (table: %s)' %(name, tableName))
        # 获取所有的Field和主键名
        mappings = dict()
        if name=='Blog':
            print('hi')
        fields = [] # 我们可以看到fields是list类型
        primaryKey = None
        for k, v in attrs.items():
            if isinstance(v, Field):
                logging.info('found mapping: %s ==> %s' % (k, v))
                mappings[k] = v # 这里我们可以看到，类型是Field类型，所以v的话，是应该有name这个属性的
                if v.primary_key: # 如果这个域是主键
                    # 并且已经存在了主键
                    if primaryKey:
                        raise RuntimeError('Duplicate primary key for fields: %s' % k)
                    primaryKey = k # 记录下主键，话说主键有两个也不奇怪吧？
                else: # 好吧，问题果然出现在这里
                    fields.append(k)

        if not primaryKey:
            raise RuntimeError('Primary key not found.')

        for k in mappings.keys():
            attrs.pop(k)

        escaped_fields = list(map(lambda f: '`%s`' %f, fields))
        attrs['__mappings__'] = mappings # 保存属性和列的映射关系
        attrs['__table__'] = tableName # 保存表的名称
        attrs['__primary_key__'] = primaryKey # 主键属性名
        attrs['__fields__'] = fields # 除主键外的属性名
        attrs['__select__'] = 'select `%s`, %s from `%s`' % (primaryKey, ', '.join(escaped_fields), tableName)
        attrs['__insert__'] =  'insert into `%s` (%s, `%s`) values (%s)' \
                               % (tableName, ', '.join(escaped_fields), primaryKey,
                                  create_args_string(len(escaped_fields) + 1))
        # 用于更新一条记录
        attrs['__update__'] = 'update `%s` set %s where `%s`=?' \
                              % (tableName,
                                 ', '.join(map(lambda f: '`%s`=?' % (f), fields)),
                                 primaryKey)

        attrs['__delete__'] = 'delete from `%s` where `%s`=?' \
                              % (tableName, primaryKey)
        return type.__new__(cls, name, bases, attrs) # 这个才是真正的构建一个类

class Model(dict, metaclass=ModelMetaclass):
    '''
    这里，我必须要说一句，那就是Model类型是dict的子类！
    '''
    def __init__(self, **kw):
        super().__init__(**kw)

    def __getattr__(self, key):
        try:
            return self[key]
        except KeyError:
            raise AttributeError(r"'model' object has no attribute '%s'" % key)

    def __setattr__(self, key, value):
        self[key] = value

    def getValue(self, key):
        return getattr(self, key, None)

    def getValueOrDefault(self, key):
        value = getattr(self, key, None)
        if value is None:
            field = self.__mappings__[key]
            if field.default is not None:
                # callable表示是否为函数调用
                value = field.default() if callable(field.default) else field.default
                logging.debug('using default value for %s: %s' % (key, str(value)))
                setattr(self, key, value)
        return value

    @classmethod
    async def findAll(cls, where=None, args=None, **kw):
        'find objects by where clause'
        sql = [cls.__select__] # 这是一个sql语句
        if where: # 如果存在where语句
            sql.append('where')
            sql.append(where)
        if args is None:
            args = []
        orderBy = kw.get('orderBy', None)
        if orderBy:
            sql.append('order by')
            sql.append(orderBy)
        limit = kw.get('limit', None) # 限制查询数据的条数
        if limit is not None:
            sql.append('limit')
            if isinstance(limit, int):
                sql.append('?')
                args.append(limit) # 参数列表
            elif isinstance(limit, tuple) and len(limit) == 2:
                sql.append('?, ?')
                args.extend(limit)
            else:
                raise ValueError('Invalid limit value: %s' % str(limit))
        rs = await select(' '.join(sql), args)
        return [cls(**r) for r in rs] # 返回一个list

    @classmethod
    async def findNumber(cls, selectField, where=None, args=None):
        'find number by select and where'
        sql = ['select %s _num_ from `%s`' % (selectField, cls.__table__)]
        if where:
            sql.append('where')
            sql.append(where)
        rs = await select(' '.join(sql), args, 1)
        if len(rs) == 0:
            return None
        return rs[0]['_num_'] # 返回一个整数

    @classmethod
    async def find(cls, pk): # pk指的是主键
        'find object by primary key.'
        rs = await select('%s where `%s`=?' % (cls.__select__, cls.__primary_key__), [pk], 1)
        if len(rs) == 0:
            return None
        return cls(**rs[0]) # 妈蛋，这个玩意包裹一层，到底返回了一个啥

    async def save(self):
        args = list(map(self.getValueOrDefault, self.__fields__))
        args.append(self.getValueOrDefault(self.__primary_key__))
        rows = await execute(self.__insert__, args) # 居然是执行插入命令
        if rows != 1:
            logging.warn('failed to insert record: affected rows: %s' % rows)

    async def update(self): # 更新命令
        args = list(map(self.getValueOrDefault, self.__fields__))
        args.append(self.getValue(self.__primary_key__))
        rows = await execute(self.__update__, args)

    async def remove(self): # 删除命令
        args = [self.getValueOrDefault(self.__primary_key__)]
        rows = await execute(self.__delete__, args)
        if rows != 1:
            logging.warn('failed to remove by primary key: affected rows: : %s' % rows)