"""This module implements the core Scheme interpreter functions, including the
eval/apply mutual recurrence, environment model, and read-eval-print loop.
"""

from scheme_primitives import *
from scheme_reader import *
from ucb import main, trace

##############
# Eval/Apply #
##############

def scheme_eval(expr, env):
    """Evaluate Scheme expression EXPR in environment ENV.

    >> expr = read_line("(+ 2 2)")
    >> expr
    Pair('+', Pair(2, Pair(2, nil)))
    >> scheme_eval(expr, create_global_frame())
    4
    """
    if expr is None:
        raise SchemeError("Cannot evaluate an undefined expression.")

    # Evaluate Atoms
    if scheme_symbolp(expr): # 如果exp是一个symbol，从env中寻找对应的值，然后返回来
        return env.lookup(expr)
    elif scheme_atomp(expr) or scheme_stringp(expr) or expr is okay: # okay 是个什么玩意？
        return expr

    # All non-atomic expressions are lists.
    if not scheme_listp(expr):
        raise SchemeError("malformed list: {0}".format(str(expr)))
    first, rest = expr.first, expr.second

    # Evaluate Combinations
    if (scheme_symbolp(first) # first might be unhashable
        and first in LOGIC_FORMS):
        # return scheme_eval(LOGIC_FORMS[first](rest, env), env)
        # 感觉这个玩意有问题，既然已经计算出了结果，就不必再计算了不是？
        return scheme_eval(LOGIC_FORMS[first](rest, env))
    elif first == "lambda":
        return do_lambda_form(rest, env)
    elif first == "mu":
        return do_mu_form(rest)
    elif first == "define":
        return do_define_form(rest, env)
    elif first == "quote":
        return do_quote_form(rest)
    elif first == "let":
        expr, env = do_let_form(rest, env)
        return scheme_eval(expr, env)
    else:
        procedure = scheme_eval(first, env)
        args = rest.map(lambda operand: scheme_eval(operand, env))
        return scheme_apply(procedure, args, env)


def scheme_apply(procedure, args, env):
    """Apply Scheme PROCEDURE to argument values ARGS in environment ENV."""
    if isinstance(procedure, PrimitiveProcedure):
        return apply_primitive(procedure, args, env)
    elif isinstance(procedure, LambdaProcedure):
        "*** YOUR CODE HERE ***"
        """
        new_env = Frame(procedure.env)
        formals = procedure.formals # 得到所谓的形式参数
        body = procedure.body
        for i in range(len(args)):
            new_env.define(formals[i], args[i])
        """
        formals = procedure.formals # 得到所谓的形式参数
        new_env = procedure.env.make_call_frame(formals, args)
        body = procedure.body
        return scheme_eval(body, new_env)
    elif isinstance(procedure, MuProcedure):
        "*** YOUR CODE HERE ***"
        formals = procedure.formals # 得到形式参数
        body = procedure.body
        new_env = env.make_call_frame(formals, args) # 得到一个新的env
        return scheme_eval(body, new_env)

    else:
        raise SchemeError("Cannot call {0}".format(str(procedure)))

def apply_primitive(procedure : PrimitiveProcedure, args : Pair, env):
    """Apply PrimitiveProcedure PROCEDURE to a Scheme list of ARGS in ENV.

    >> env = create_global_frame()
    >> plus = env.bindings["+"]
    >> twos = Pair(2, Pair(2, nil))
    >> apply_primitive(plus, twos, env)
    4
    """
    "*** YOUR CODE HERE ***"
    if procedure.use_env: # 好吧，其实env还是有作用的
        return procedure.fn(args[0], env)

    fargs = [] # fargs是一个list
    for i in range(len(args)):
        fargs.append(args[i])
    try:
         res = procedure.fn(*fargs)
    except TypeError: # 好吧，其实是要做错误处理的
         raise SchemeError("Incorrect number of arguments!")
    else:
        return res

################
# Environments #
################

class Frame:
    """An environment frame binds Scheme symbols to Scheme values."""

    def __init__(self, parent):
        """An empty frame with a PARENT frame (that may be None)."""
        self.bindings = {} # 这玩意其实将这个东西变得复杂了，bindings
        self.parent = parent # 还有父env

    def __repr__(self):
        if self.parent is None:
            return "<Global Frame>" # 我猜测全局的env估计就一个吧！
        else:
            s = sorted('{0}: {1}'.format(k,v) for k,v in self.bindings.items())
            return "<{{{0}}} -> {1}>".format(', '.join(s), repr(self.parent))

    def lookup(self, symbol): # 查找symbol，好吧，这里应该是填写自己代码的时候了
        """Return the value bound to SYMBOL.  Errors if SYMBOL is not found."""
        "*** YOUR CODE HERE ***"
        """递归版本的代码
        if symbol in self.bindings:
            return self.bindings[symbol]
        elif self.parent != None:
            return self.parent.lookup(symbol)
        else:
            raise SchemeError("unknown identifier: {0}".format(str(symbol)))
        """
        # 虽然说上面的代码看起来也挺不错的，但是，容易溢出，你可以想象一下，如果要查找的symbol位于最顶层的Frame，而这个Frame连起来
        # 有1000多个节点的话，堆栈会有多深
        if symbol in self.bindings:
            return self.bindings[symbol]
        parent = self.parent
        while (parent != None):
            if symbol in parent.bindings:
                return parent.bindings[symbol]
            parent = parent.parent
        raise SchemeError("unknown identifier: {0}".format(str(symbol)))

    def global_frame(self):
        """The global environment at the root of the parent chain."""
        e = self # 好吧，其实就是找最上层的那个env
        while e.parent is not None:
            e = e.parent
        return e

    def make_call_frame(self, formals, vals):
        """Return a new local frame whose parent is SELF, in which the symbols
        in the Scheme formal parameter list FORMALS are bound to the Scheme
        values in the Scheme value list VALS. Raise an error if too many or too
        few arguments are given.

        >> env = create_global_frame()
        >> formals, vals = read_line("(a b c)"), read_line("(1 2 3)")
        >> env.make_call_frame(formals, vals)
        <{a: 1, b: 2, c: 3} -> <Global Frame>>
        """
        # 在调用之前，应该检查一下参数的情况
        len_formals = len(formals)
        len_vals = len(vals)
        if len_formals != len_vals:
            raise SchemeError('The number of argument is not correct!')
        frame = Frame(self)
        "*** YOUR CODE HERE ***"
        for i in range(len_formals):
            frame.define(formals[i], vals[i])
        return frame

    def define(self, sym, val): # 也就是添加所谓的绑定吧！
        """Define Scheme symbol SYM to have value VAL in SELF."""
        self.bindings[sym] = val

class LambdaProcedure: # 这个类主要用于定义函数
    """A procedure defined by a lambda expression or the complex define form."""

    def __init__(self, formals, body, env): # 初始化函数, formal表示形式参数，貌似这些玩意
        # 并没有表示成为一棵抽象的语法树，是吧！
        """A procedure whose formal parameter list is FORMALS (a Scheme list),
        whose body is the single Scheme expression BODY, and whose parent
        environment is the Frame ENV.  A lambda expression containing multiple
        expressions, such as (lambda (x) (display x) (+ x 1)) can be handled by
        using (begin (display x) (+ x 1)) as the body."""
        self.formals = formals
        self.body = body
        self.env = env

    def __str__(self):
        return "(lambda {0} {1})".format(str(self.formals), str(self.body))

    def __repr__(self):
        args = (self.formals, self.body, self.env)
        return "LambdaProcedure({0}, {1}, {2})".format(*(repr(a) for a in args))

# 总算是让我找到了，是吧！
class MuProcedure:
    """A procedure defined by a mu expression, which has dynamic scope.
     _________________
    < Scheme is cool! >
     -----------------
            \   ^__^
             \  (oo)\_______
                (__)\       )\/\
                    ||----w |
                    ||     ||
    """

    def __init__(self, formals, body): # 构造函数，formals表示形式参数
        """A procedure whose formal parameter list is FORMALS (a Scheme list),
        whose body is the single Scheme expression BODY.  A mu expression
        containing multiple expressions, such as (mu (x) (display x) (+ x 1))
        can be handled by using (begin (display x) (+ x 1)) as the body."""
        self.formals = formals
        self.body = body

    def __str__(self):
        return "(mu {0} {1})".format(str(self.formals), str(self.body))

    def __repr__(self):
        args = (self.formals, self.body)
        return "MuProcedure({0}, {1})".format(*(repr(a) for a in args))


#################
# Special forms #
#################

def do_lambda_form(vals : Pair, env):
    """Evaluate a lambda form with parameters VALS in environment ENV."""
    check_form(vals, 2)
    formals = vals[0]
    check_formals(formals)
    "*** YOUR CODE HERE ***"
    # forms表示的是形式参数
    # 好吧，要将这里的body转换为begin表达式才行!
    if len(vals) > 2:
        body = Pair('begin', vals.second)
    else:
        body = vals[1]
    return LambdaProcedure(formals, body, env)

def do_mu_form(vals : Pair):
    """Evaluate a mu form with parameters VALS."""
    check_form(vals, 2)
    formals = vals[0]
    check_formals(formals)
    "*** YOUR CODE HERE ***"
    # 所谓的Mu，其实只是一个动态作用域的函数，什么意思呢？我们来玩一下吧！
    body = vals[1]
    return MuProcedure(formals, body)

def do_define_form(vals : Pair, env):
    """Evaluate a define form with parameters VALS in environment ENV."""
    check_form(vals, 2)
    target = vals[0] # 需要注意的是vals是Pair形式的
    if scheme_symbolp(target):
        check_form(vals, 2, 2)
        "*** YOUR CODE HERE ***"
        val = scheme_eval(vals[1], env)
        env.define(target, val)
        return target
    elif isinstance(target, Pair):
        "*** YOUR CODE HERE ***"
        # 如果是Pair类型，一定是这种玩意，那么要将这种形式转化为lambda表达式
        func_name = target[0] # 这个是函数的名字
        # 这里需要注意的是，函数的名字不应该是数字或者其他一类的东西，否则要报错
        if not scheme_symbolp(func_name):
            raise SchemeError('function name is not a symbol!')
        func_formals = target.second # 形式参数
        if (len(vals.second) >= 2): # 好吧，到了这里也要判断body里面的运算数的个数
            exp = Pair('lambda', Pair(func_formals, vals.second))
        else:
            exp = Pair('lambda', Pair(func_formals, Pair(vals[1], nil)))
        val = scheme_eval(exp, env)
        env.define(func_name, val)
        return func_name
    else:
        raise SchemeError("bad argument to define")

def do_quote_form(vals):
    """Evaluate a quote form with parameters VALS."""
    check_form(vals, 1, 1)
    "*** YOUR CODE HERE ***"
    return vals[0]

def do_let_form(vals, env):
    """Evaluate a let form with parameters VALS in environment ENV."""
    check_form(vals, 2)
    bindings = vals[0]
    exprs = vals.second
    if not scheme_listp(bindings):
        raise SchemeError("bad bindings list in let form")

    # Add a frame containing bindings
    names, values = nil, nil
    "*** YOUR CODE HERE ***"
    new_env = env.make_call_frame(names, values)
    # 好吧，果然又弄了一个新的env
    for exp in bindings:
        names = exp[0]
        values = scheme_eval(exp[1], env)
        new_env.define(names, values)
    # Evaluate all but the last expression after bindings, and return the last
    last = len(exprs) - 1
    for i in range(0, last):
        scheme_eval(exprs[i], new_env)
    return exprs[last], new_env


#########################
# Logical Special Forms #
#########################

def do_if_form(vals, env):
    """Evaluate if form with parameters VALS in environment ENV."""
    check_form(vals, 2, 3)
    "*** YOUR CODE HERE ***"
    condition = scheme_eval(vals[0], env)
    if isinstance(condition, Pair): # 好啦，我也不是故意的啦，如果condition是Pair类型的话，下面的判断会报TypeError的
        condition = True
    if condition: # condition
        return vals[1] # then branch
    elif (len(vals) > 2):
        return vals[2] # else branch
    else:
        return okay

def do_and_form(vals, env):
    """Evaluate short-circuited and with parameters VALS in environment ENV."""
    "*** YOUR CODE HERE ***"
    num_exp = len(vals)
    if vals is nil:
        return True
    elif num_exp == 1: # 如果仅有一个元素，那么and的值是这个元素的值
        return vals[0] # 一样的，为了避免堆栈太深，所以直接返回exp，这里也就不计算了
    for i in range(num_exp - 1):
        if scheme_eval(vals[i], env) == False:
            return False
    return vals[num_exp - 1] # 这种情况是前面的计算都不是False，那么只能够返回最后一个元素了
    # 当然，递归的版本更加优雅

def quote(value):
    """Return a Scheme expression quoting the Scheme VALUE.

    >> s = quote('hello')
    >> print(s)
    (quote hello)
    >> scheme_eval(s, Frame(None))  # "hello" is undefined in this frame.
    'hello'
    """
    return Pair("quote", Pair(value, nil))

def do_or_form(vals, env):
    """Evaluate short-circuited or with parameters VALS in environment ENV."""
    "*** YOUR CODE HERE ***"
    """ 递归的版本
    if vals is nil:
        return False
    elif len(vals) == 1:
        return scheme_eval(vals[0], env)
    else:
        val = scheme_eval(vals[0], env)
        if not isinstance(val, bool):
            # 如果计算出的val的类型不是bool类型的
            return val
        elif val != False: # val是bool类型的，且不是False
            return val
    return do_or_form(vals.second, env)
    """
    num_exp = len(vals)
    if num_exp == 0:
        return False
    elif num_exp == 1:
        return vals[0] # 不计算，直接返回表达式
    for i in range(num_exp - 1):
        val = scheme_eval(vals[i], env)
        if not isinstance(val, bool): # 如果计算出的val的类型不是bool类型
            return vals[i] # 如果vals[i]是quote类型的话，val会是一个str类型，会出错的
        elif val != False: # val是bool类型的，而且不是False
            return val
    return vals[num_exp - 1]


def do_cond_form(vals, env):
    """Evaluate cond form with parameters VALS in environment ENV."""
    num_clauses = len(vals) # 条件的数目
    for i, clause in enumerate(vals):
        check_form(clause, 1)
        if clause.first == "else":
            if i < num_clauses-1:
                raise SchemeError("else must be last")
            test = True
            if clause.second is nil:
                raise SchemeError("badly formed else clause")
        else:
            test = scheme_eval(clause.first, env)
        if scheme_true(test):
            "*** YOUR CODE HERE ***"
            begin_exp = clause.second
            if begin_exp is nil:
                return clause.first
            return Pair('begin', begin_exp) # 好的，我已经做好了一部分的优化，避免递归
    return okay

def do_begin_form(vals, env):
    """Evaluate begin form with parameters VALS in environment ENV."""
    check_form(vals, 1)
    "*** YOUR CODE HERE ***"
    # 每个式子都要计算啊！挺扯淡的。弄得我好像没有什么办法可以优化一样。
    num_exp = len(vals)
    for i in range(num_exp - 1): # 计算前面的n - 1个式子
        scheme_eval(vals[i], env) # 这里必须要计算，如果堆栈溢出了，那也没有什么好的办法！
    return vals[num_exp - 1] # 最后的一个式子不计算，直接返回


LOGIC_FORMS = {
        "and": do_and_form,
        "or": do_or_form,
        "if": do_if_form,
        "cond": do_cond_form,
        "begin": do_begin_form,
        }

# Utility methods for checking the structure of Scheme programs

def check_form(expr, min, max = None): # expr表示的是表达式吗？
    """Check EXPR (default SELF.expr) is a proper list whose length is
    at least MIN and no more than MAX (default: no maximum). Raises
    a SchemeError if this is not the case."""
    if not scheme_listp(expr):
        raise SchemeError("badly formed expression: " + str(expr))
    length = len(expr)
    if length < min: # 参数过少
        raise SchemeError("too few operands in form")
    elif max is not None and length > max: # 参数过多
        raise SchemeError("too many operands in form")

def check_formals(formals : Pair):
    """Check that FORMALS is a valid parameter list, a Scheme list of symbols
    in which each symbol is distinct. Raise a SchemeError if the list of formals
    is not a well-formed list of symbols or if any symbol is repeated.

    >> check_formals(read_line("(a b c)"))
    """
    "*** YOUR CODE HERE ***"
    def helper(item): # 好吧，定义一个辅助函数
        counter = 0
        for i in range(len(formals)):
            if item == formals[i]:
                counter += 1
                if counter > 1:
                    return False
        return True

    # 好吧，必须要判断，所谓的形式参数必须都是symbol
    # 另外有一点，那就是参数不能重复
    for i in range(len(formals)):
        if not scheme_symbolp(formals[i]):
            raise scheme_error('{0} is not a symbol!'.format(formals[i]))
        elif not helper(formals[i]):
            raise scheme_error('{0} is duplicated!'.format(formals[i]))
##################
# Tail Recursion #
##################

def scheme_optimized_eval(expr, env : Frame): # 居然还处理尾递归
    """Evaluate Scheme expression EXPR in environment ENV."""
    # 逆天了，还要处理尾递归的优化！
    while True: # 有意思，我们一定要注意到，这里是无穷的循环。
        if expr is None:
            raise SchemeError("Cannot evaluate an undefined expression.")

        # Evaluate Atoms
        if scheme_symbolp(expr):
            return env.lookup(expr)
        # 这里，我需要说一声的是，lookup 也应该需要优化，
        # 你想，如果env的lookup函数通过不断递归来搜寻对应的Symbol，而这个Symbol位于最顶层的Frame
        # 查找函数可能会查找超过1000个Frame
        # 才能找到所要的值，这个时候一定是超过了Python限定的堆栈深度的
        elif scheme_atomp(expr) or scheme_stringp(expr) or expr is okay:
            return expr

        # All non-atomic expressions are lists.
        if not scheme_listp(expr):
            raise SchemeError("malformed list: {0}".format(str(expr)))
        first, rest = expr.first, expr.second

        # Evaluate Combinations
        if (scheme_symbolp(first) # first might be unhashable
            and first in LOGIC_FORMS):
            "*** YOUR CODE HERE ***"
            # 这里说明是primitive的函数
            expr =  LOGIC_FORMS[first](rest, env) # 这里直接返回要计算的表达式
            # 原来是这样来调用的 return scheme_eval(LOGIC_FORMS[first](rest, env), env)
            # 按照上面的方法，我们可以看到，涉及到了递归，我们要做的，是消除这样的递归调用
        elif first == "lambda":
            return do_lambda_form(rest, env) # 这个玩意主要是在env中添加函数的定义
        elif first == "mu":
            return do_mu_form(rest)
        elif first == "define":
            return do_define_form(rest, env)
        elif first == "quote":
            return do_quote_form(rest)
        elif first == "let":
            "*** YOUR CODE HERE ***"
            expr, env = do_let_form(rest, env)
        else:
            "*** YOUR CODE HERE ***"
            procedure = scheme_optimized_eval(first, env) # 从env中找到对应的函数值
            # 接下来要判断这种式子是不是可以尾递归调用
            args = rest.map(lambda operand: scheme_optimized_eval(operand, env))
            if isinstance(procedure, LambdaProcedure):
                env = procedure.env.make_call_frame(procedure.formals, args) # 构建一个新的env，替代之前的env
            elif isinstance(procedure, PrimitiveProcedure):
                return apply_primitive(procedure, args, env)
            elif isinstance(procedure, MuProcedure):
                env = env.make_call_frame(procedure.formals, args)
            else:
                raise SchemeError("Cannot call {0}".format(str(procedure)))
            expr = procedure.body
            # return scheme_apply(procedure, args, env)


################################################################
# Uncomment the following line to apply tail call optimization #
################################################################
scheme_eval = scheme_optimized_eval


################
# Input/Output #
################

def read_eval_print_loop(next_line, env, quiet=False, startup=False,
                         interactive=False, load_files=()):
    """Read and evaluate input until an end of file or keyboard interrupt."""
    if startup:
        for filename in load_files:
            scheme_load(filename, True, env)
    while True:
        try:
            src = next_line()
            while src.more_on_line:
                expression = scheme_read(src)
                result = scheme_eval(expression, env)
                if not quiet and result is not None:
                    print(result)
        except (SchemeError, SyntaxError, ValueError, RuntimeError) as err:
            if (isinstance(err, RuntimeError) and
                'maximum recursion depth exceeded' not in err.args[0]):
                raise
            print("Error:", err)
        except KeyboardInterrupt:  # <Control>-C
            if not startup:
                raise
            print("\nKeyboardInterrupt")
            if not interactive:
                return
        except EOFError:  # <Control>-D, etc.
            return


def scheme_load(*args):
    """Load a Scheme source file. ARGS should be of the form (SYM, ENV) or (SYM,
    QUIET, ENV). The file named SYM is loaded in environment ENV, with verbosity
    determined by QUIET (default true)."""
    if not (2 <= len(args) <= 3):
        vals = args[:-1]
        raise SchemeError("wrong number of arguments to load: {0}".format(vals))
    sym = args[0]
    quiet = args[1] if len(args) > 2 else True
    env = args[-1]
    if (scheme_stringp(sym)):
        sym = eval(sym)
    check_type(sym, scheme_symbolp, 0, "load")
    with scheme_open(sym) as infile:
        lines = infile.readlines()
    args = (lines, None) if quiet else (lines,)
    def next_line():
        return buffer_lines(*args)
    read_eval_print_loop(next_line, env.global_frame(), quiet=quiet)
    return okay

def scheme_open(filename):
    """If either FILENAME or FILENAME.scm is the name of a valid file,
    return a Python file opened to it. Otherwise, raise an error."""
    try:
        return open(filename)
    except IOError as exc:
        if filename.endswith('.scm'):
            raise SchemeError(str(exc))
    try:
        return open(filename + '.scm')
    except IOError as exc:
        raise SchemeError(str(exc))

def create_global_frame(): # 构建一个全局的env
    """Initialize and return a single-frame environment with built-in names."""
    env = Frame(None)
    env.define("eval", PrimitiveProcedure(scheme_eval, True)) # 好吧，函数在这里
    env.define("apply", PrimitiveProcedure(scheme_apply, True))
    env.define("load", PrimitiveProcedure(scheme_load, True))
    add_primitives(env)
    return env

@main
def run(*argv):
    next_line = buffer_input
    interactive = True
    load_files = ()
    if argv:
        try:
            filename = argv[0]
            if filename == '-load':
                load_files = argv[1:]
            else:
                input_file = open(argv[0])
                lines = input_file.readlines()
                def next_line():
                    return buffer_lines(lines)
                interactive = False
        except IOError as err:
            print(err)
            sys.exit(1)
    read_eval_print_loop(next_line, create_global_frame(), startup=True,
                         interactive=interactive, load_files=load_files)
    tscheme_exitonclick()
