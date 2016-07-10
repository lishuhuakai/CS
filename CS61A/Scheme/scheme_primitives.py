"""This module implements the primitives of the Scheme language."""

import math
import operator
import sys
from scheme_reader import Pair, nil

try:
    import turtle
except:
    print("warning: could not import the turtle module.", file=sys.stderr)

class SchemeError(Exception):
    """Exception indicating an error in a Scheme program."""

class okay:
    """Signifies an undefined value."""
    def __repr__(self):
        return "okay"

# 全局只有一个okay实例
okay = okay() # Assignment hides the okay class; there is only one instance

########################
# Primitive Operations #
########################

class PrimitiveProcedure:
    """A Scheme procedure defined as a Python function."""

    def __init__(self, fn, use_env=False):
        self.fn = fn # fn代表的是一个函数么？
        self.use_env = use_env

    def __str__(self):
        return '#[primitive]'

_PRIMITIVES = []

# *names是python的一个语法糖，即可变参数
def primitive(*names):
    """An annotation to convert a Python function into a PrimitiveProcedure."""
    def add(fn):
        proc = PrimitiveProcedure(fn) # 先构造一个函数，是吧！这里默认是没有env的
        for name in names:
            _PRIMITIVES.append((name,proc))
        return fn
    return add

def add_primitives(frame):
    """Enter bindings in _PRIMITIVES into FRAME, an environment frame."""
    for name, proc in _PRIMITIVES:
        frame.define(name, proc)

def check_type(val, predicate, k, name): # 检查参数的类型么？
    """Returns VAL.  Raises a SchemeError if not PREDICATE(VAL)
    using "argument K of NAME" to describe the offending value."""
    if not predicate(val):
        msg = "argument {0} of {1} has wrong type ({2})"
        raise SchemeError(msg.format(k, name, type(val).__name__))
    return val

@primitive("boolean?") # 这里其实是一个映射的关系，将boolean?映射到python
def scheme_booleanp(x):
    return x is True or x is False

def scheme_true(val):
    """All values in Scheme are true except False."""
    return val is not False

def scheme_false(val):
    """Only False is false in Scheme."""
    return val is False

@primitive("not") # 原生的函数
def scheme_not(x):
    return not scheme_true(x)

@primitive("eq?", "equal?")
def scheme_eqp(x, y):
    return x == y

@primitive("pair?")
def scheme_pairp(x):
    return isinstance(x, Pair)

@primitive("null?")
def scheme_nullp(x):
    return x is nil

@primitive("list?")
def scheme_listp(x):
    """Return whether x is a well-formed list. Assumes no cycles."""
    while x is not nil:
        if not isinstance(x, Pair):
            return False
        x = x.second
    return True

@primitive("length")
def scheme_length(x):
    if x is nil:
        return 0
    check_type(x, scheme_listp, 0, 'length')
    return len(x)

@primitive("cons") # 居然有cons函数
def scheme_cons(x, y):
    return Pair(x, y)

@primitive("car")
def scheme_car(x):
    check_type(x, scheme_pairp, 0, 'car')
    return x.first

@primitive("cdr")
def scheme_cdr(x):
    check_type(x, scheme_pairp, 0, 'cdr')
    return x.second


@primitive("list") # 用于构建一个list
def scheme_list(*vals):
    result = nil
    for i in range(len(vals)-1, -1, -1):
        result = Pair(vals[i], result)
    return result

@primitive("append")
def scheme_append(*vals):
    if len(vals) == 0:
        return nil
    result = vals[-1]
    for i in range(len(vals)-2, -1, -1):
        v = vals[i]
        if v is not nil:
            check_type(v, scheme_pairp, i, "append")
            r = p = Pair(v.first, result)
            v = v.second
            while scheme_pairp(v):
                p.second = Pair(v.first, result)
                p = p.second
                v = v.second
            result = r
    return result

@primitive("string?")
def scheme_stringp(x):
    return isinstance(x, str) and x.startswith('"')

@primitive("symbol?")
def scheme_symbolp(x):
    return isinstance(x, str) and not scheme_stringp(x)

@primitive("number?")
def scheme_numberp(x):
    return isinstance(x, int) or isinstance(x, float)

@primitive("integer?")
def scheme_integerp(x):
    return isinstance(x, int) or (scheme_numberp(x) and round(x) == x)

def _check_nums(*vals): # 要求都是数字啦!
    """Check that all arguments in VALS are numbers."""
    for i, v in enumerate(vals):
        if not scheme_numberp(v):
            msg = "operand {0} ({1}) is not a number"
            raise SchemeError(msg.format(i, v))

def _arith(fn, init, vals):
    """Perform the fn fneration on the number values of VALS, with INIT as
    the value when VALS is empty. Returns the result as a Scheme value."""
    _check_nums(*vals) # 要求vals都是数字,由于vals已经是一个list或者tuple，而_check_nums的参数是一个可变参数，所以要将vals变为
    # 可变参数，所以添加一个*，语法糖啦！
    s = init # 初始的值
    for val in vals:
        s = fn(s, val) # 这个函数其实听类似于map的
    if round(s) == s:
        s = round(s)
    return s

@primitive("+")
def scheme_add(*vals):
    return _arith(operator.add, 0, vals)

@primitive("-")
def scheme_sub(val0, *vals):
    if len(vals) == 0:
        return -val0
    return _arith(operator.sub, val0, vals)

@primitive("*")
def scheme_mul(*vals):
    return _arith(operator.mul, 1, vals)

@primitive("/")
def scheme_div(val0, val1):
    try:
        return _arith(operator.truediv, val0, [val1])
    except ZeroDivisionError as err:
        raise SchemeError(err)

@primitive("quotient")
def scheme_quo(val0, val1):
    try:
        return _arith(operator.floordiv, val0, [val1])
    except ZeroDivisionError as err:
        raise SchemeError(err)

@primitive("modulo", "remainder")
def scheme_modulo(val0, val1):
    try:
        return _arith(operator.mod, val0, [val1])
    except ZeroDivisionError as err:
        raise SchemeError(err)

@primitive("floor")
def scheme_floor(val):
    _check_nums(val)
    return math.floor(val)

@primitive("ceil")
def scheme_ceil(val):
    _check_nums(val)
    return math.ceil(val)

def _numcomp(op, x, y):
    _check_nums(x, y)
    return op(x, y)

@primitive("=")
def scheme_eq(x, y):
    return _numcomp(operator.eq, x, y)

@primitive("<")
def scheme_lt(x, y):
    return _numcomp(operator.lt, x, y)

@primitive(">")
def scheme_gt(x, y):
    return _numcomp(operator.gt, x, y)

@primitive("<=")
def scheme_le(x, y):
    return _numcomp(operator.le, x, y)

@primitive(">=")
def scheme_ge(x, y):
    return _numcomp(operator.ge, x, y)

@primitive("even?")
def scheme_evenp(x):
    _check_nums(x)
    return x % 2 == 0

@primitive("odd?")
def scheme_oddp(x):
    _check_nums(x)
    return x % 2 == 1

@primitive("zero?")
def scheme_zerop(x):
    _check_nums(x)
    return x == 0

##
## Other operations
##

@primitive("atom?")
def scheme_atomp(x):
    if scheme_booleanp(x):
        return True
    if scheme_numberp(x):
        return True
    if scheme_symbolp(x):
        return True
    if scheme_nullp(x):
        return True
    return False

@primitive("display")
def scheme_display(val):
    if scheme_stringp(val):
        val = eval(val)
    print(str(val), end="")
    return okay

@primitive("print")
def scheme_print(val):
    print(str(val))
    return okay

@primitive("newline")
def scheme_newline():
    print()
    sys.stdout.flush()
    return okay

@primitive("error")
def scheme_error(msg = None):
    msg = "" if msg is None else str(msg)
    raise SchemeError(msg)

@primitive("exit")
def scheme_exit():
    raise EOFError

##
## Turtle graphics (non-standard)
##

_turtle_screen_on = False

def turtle_screen_on():
    return _turtle_screen_on

def _tscheme_prep():
    global _turtle_screen_on
    if not _turtle_screen_on:
        _turtle_screen_on = True
        turtle.title("Scheme Turtles")
        turtle.mode('logo')

@primitive("forward", "fd")
def tscheme_forward(n):
    """Move the turtle forward a distance N units on the current heading."""
    _check_nums(n)
    _tscheme_prep()
    turtle.forward(n)
    return okay

@primitive("backward", "back", "bk")
def tscheme_backward(n):
    """Move the turtle backward a distance N units on the current heading,
    without changing direction."""
    _check_nums(n)
    _tscheme_prep()
    turtle.backward(n)
    return okay

@primitive("left", "lt")
def tscheme_left(n):
    """Rotate the turtle's heading N degrees counterclockwise."""
    _check_nums(n)
    _tscheme_prep()
    turtle.left(n)
    return okay

@primitive("right", "rt")
def tscheme_right(n):
    """Rotate the turtle's heading N degrees clockwise."""
    _check_nums(n)
    _tscheme_prep()
    turtle.right(n)
    return okay

@primitive("circle")
def tscheme_circle(r, extent = None):
    """Draw a circle with center R units to the left of the turtle (i.e.,
    right if N is negative.  If EXTENT is not None, then draw EXTENT degrees
    of the circle only.  Draws in the clockwise direction if R is negative,
    and otherwise counterclockwise, leaving the turtle facing along the
    arc at its end."""
    if extent is None:
        _check_nums(r)
    else:
        _check_nums(r, extent)
    _tscheme_prep()
    turtle.circle(r, extent and extent)
    return okay

@primitive("setposition", "setpos", "goto")
def tscheme_setposition(x, y):
    """Set turtle's position to (X,Y), heading unchanged."""
    _check_nums(x, y)
    _tscheme_prep()
    turtle.setposition(x, y)
    return okay

@primitive("setheading", "seth")
def tscheme_setheading(h):
    """Set the turtle's heading H degrees clockwise from north (up)."""
    _check_nums(h)
    _tscheme_prep()
    turtle.setheading(h)
    return okay

@primitive("penup", "pu")
def tscheme_penup():
    """Raise the pen, so that the turtle does not draw."""
    _tscheme_prep()
    turtle.penup()
    return okay

@primitive("pendown", "pd")
def tscheme_pendown():
    """Lower the pen, so that the turtle starts drawing."""
    _tscheme_prep()
    turtle.pendown()
    return okay

@primitive("showturtle", "st")
def tscheme_showturtle():
    """Make turtle visible."""
    _tscheme_prep()
    turtle.showturtle()
    return okay

@primitive("hideturtle", "ht")
def tscheme_hideturtle():
    """Make turtle visible."""
    _tscheme_prep()
    turtle.hideturtle()
    return okay

@primitive("clear")
def tscheme_clear():
    """Clear the drawing, leaving the turtle unchanged."""
    _tscheme_prep()
    turtle.clear()
    return okay

@primitive("color")
def tscheme_color(c):
    """Set the color to C, a string such as '"red"' or '"#ffc0c0"' (representing
    hexadecimal red, green, and blue values."""
    _tscheme_prep()
    check_type(c, scheme_stringp, 0, "color")
    turtle.color(eval(c))
    return okay

@primitive("begin_fill")
def tscheme_begin_fill():
    """Start a sequence of moves that outline a shape to be filled."""
    _tscheme_prep()
    turtle.begin_fill()
    return okay

@primitive("end_fill")
def tscheme_end_fill():
    """Fill in shape drawn since last begin_fill."""
    _tscheme_prep()
    turtle.end_fill()
    return okay

@primitive("exitonclick")
def tscheme_exitonclick():
    """Wait for a click on the turtle window, and then close it."""
    global _turtle_screen_on
    if _turtle_screen_on:
        print("Close or click on turtle window to complete exit")
        turtle.exitonclick()
        _turtle_screen_on = False
    return okay

@primitive("speed")
def tscheme_speed(s):
    """Set the turtle's animation speed as indicated by S (an integer in
    0-10, with 0 indicating no animation (lines draw instantly), and 1-10
    indicating faster and faster movement."""
    check_type(s, scheme_integerp, 0, "speed")
    _tscheme_prep()
    turtle.speed(s)
    return okay
