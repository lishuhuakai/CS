"""This module implements the built-in data types of the Scheme language, along
with a parser for Scheme expressions.

In addition to the types defined in this file, some data types in Scheme are
represented by their corresponding type in Python:
    number:       int or float
    symbol:       string
    boolean:      bool
    unspecified:  None

The __repr__ method of a Scheme value will return a Python expression that
would be evaluated to the value, where possible.

The __str__ method of a Scheme value will return a Scheme expression that
would be read to the value, where possible.
"""

from ucb import main, trace, interact
from scheme_tokens import tokenize_lines, DELIMITERS
from buffer import Buffer, InputReader, LineReader

# Pairs and Scheme lists

class Pair:
    """A pair has two instance attributes: first and second.  For a Pair to be
    a well-formed list, second is either a well-formed list or nil.  Some
    methods only apply to well-formed lists.

    >>> s = Pair(1, Pair(2, nil))
    >>> s
    Pair(1, Pair(2, nil))
    >>> print(s)
    (1 2)
    >>> len(s)
    2
    >>> s[1]
    2
    >>> print(s.map(lambda x: x+4))
    (5 6)
    """
    def __init__(self, first, second): # 一共有两个元素
        self.first = first
        self.second = second

    def __repr__(self): # 输出的字符串可以用eval方法来构建出这个object，是吧！
        return "Pair({0}, {1})".format(repr(self.first), repr(self.second))

    def __str__(self):
        s = "(" + str(self.first) # 这里实际上在递归调用
        second = self.second
        while isinstance(second, Pair): # 如果第二个元素仍然是一个Pair的话
            s += " " + str(second.first)
            second = second.second
        if second is not nil:
            s += " . " + str(second)
        return s + ")"

    def __len__(self): # 个人猜测，这个方法是个len调用的，用于输出Pair的长度
        n, second = 1, self.second
        while isinstance(second, Pair):
            n += 1
            second = second.second
        if second is not nil:
            raise TypeError("length attempted on improper list")
        return n

    def __getitem__(self, k): # 要取第k个元素么,这个魔法方法非常好用，因为它使得Pair变得如同数组一样
        if k < 0:
            raise IndexError("negative index into list")
        y = self
        for _ in range(k):
            if y.second is nil:
                raise IndexError("list index out of bounds")
            elif not isinstance(y.second, Pair):
                raise TypeError("ill-formed list")
            y = y.second
        return y.first

    def __eq__(self, p): # 这个方法用于两个对象的比较，判断他们是否相等。
        if not isinstance(p, Pair):
            return False
        return self.first == p.first and self.second == p.second # 如果second都是Pair的话，其实会引起递归的

    def map(self, fn): # 这个方法挺厉害的，说白了，其实就是对list中的每一个元素都要进行相应的fn的映射操作
        """Return a Scheme list after mapping Python function FN to SELF."""
        mapped = fn(self.first)
        if self.second is nil or isinstance(self.second, Pair):
            return Pair(mapped, self.second.map(fn))
        else:
            raise TypeError("ill-formed list")

class nil: # 居然还有nil这个玩意
    """The empty list"""

    def __repr__(self):
        return "nil"

    def __str__(self):
        return "()"

    def __len__(self):
        return 0

    def __getitem__(self, k):
        if k < 0:
            raise IndexError("negative index into list")
        raise IndexError("list index out of bounds")

    def map(self, fn):
        return self

nil = nil() # Assignment hides the nil class; there is only one instance

# Scheme list parser
def scheme_read(src):
    """Read the next expression from SRC, a Buffer of tokens.

    >>> lines = ["(+ 1 ", "(+ 23 4)) ("]
    >>> src = Buffer(tokenize_lines(lines))
    >>> print(scheme_read(src))
    (+ 1 (+ 23 4))
    >>> read_line("'hello")
    Pair('quote', Pair('hello', nil))
    >>> print(read_line("(car '(1 2))"))
    (car (quote (1 2)))
    """
    if src.current() is None:
        raise EOFError
    val = src.pop() # 从buf中读取一个元素
    if val == "nil":
        return nil # 这个nil可是全局独一份的
    elif val not in DELIMITERS: # val不是分隔符{'.', ',', ', '@', '(', ')', '`')
        return val
    elif val == "'":
        "*** YOUR CODE HERE ***"
        # rest = read_tail(src)
        first = scheme_read(src)
        return Pair('quote', Pair(first, nil))
        # '是什么表达式?
    elif val == "(": # 读到了左括号，肯定要读到右括号才行，是吧！
        return read_tail(src)
    else:
        raise SyntaxError("unexpected token: {0}".format(val))

def read_tail(src):
    """Return the remainder of a list in SRC, starting before an element or ).

    >>> read_tail(Buffer(tokenize_lines([")"])))
    nil
    >>> read_tail(Buffer(tokenize_lines(["2 3)"])))
    Pair(2, Pair(3, nil))
    >>> read_tail(Buffer(tokenize_lines(["2 (3 4))"])))
    Pair(2, Pair(Pair(3, Pair(4, nil)), nil))
    >>> read_line("(1 . 2)")
    Pair(1, 2)
    >>> read_line("(1 2 . 3)")
    Pair(1, Pair(2, 3))
    >>> read_line("(1 . 2 3)")
    Traceback (most recent call last):
        ...
    SyntaxError: Expected one element after .
    >>> scheme_read(Buffer(tokenize_lines(["(1", "2 .", "'(3 4))", "4"])))
    Pair(1, Pair(2, Pair('quote', Pair(Pair(3, Pair(4, nil)), nil))))
    """
    try:
        if src.current() is None:
            raise SyntaxError("unexpected end of file") # 为空的话，报异常
        if src.current() == ")":
            src.pop()
            return nil # 这里是返回了一个部分
        "*** YOUR CODE HERE ***"
        # 原来是要你在这里写代码呀！
        if src.current() == ".": #如果是.好的话，说明现在make pair了
            src.pop() # 弹出一个
            rest1 = scheme_read(src) # 继续往下读
            rest2 = read_tail(src)
            if rest2 == nil:
                return rest1
            else:
                raise SyntaxError("这里有问题啊！")
        first = scheme_read(src) # 读前面部分的token
        rest = read_tail(src)
        return Pair(first, rest)
    except EOFError:
        raise SyntaxError("unexpected end of file")

# Convenience methods
def buffer_input(prompt="scm> "): # 从输入获取一行，然后将这一行数据传递给tokenize_lines
    # tokenize_lines处理之后，输出串token，传递个buffer，buffer可是一个可以迭代的对象，所以，你懂的!
    """Return a Buffer instance containing interactive input."""
    return Buffer(tokenize_lines(InputReader(prompt)))

def buffer_lines(lines, prompt="scm> ", show_prompt=False):
    """Return a Buffer instance iterating through LINES."""
    if show_prompt:
        input_lines = lines
    else:
        input_lines = LineReader(lines, prompt) # LineReader是buf文件中的一个类
    return Buffer(tokenize_lines(input_lines))

def read_line(line): # 读入单独的一行，然后进行相应的处理
    """Read a single string LINE as a Scheme expression."""
    return scheme_read(Buffer(tokenize_lines([line])))

# Interactive loop

@main
def read_print_loop():
    """Run a read-print loop for Scheme expressions."""
    while True:
        try:
            src = buffer_input("read> ")
            while src.more_on_line:
                expression = scheme_read(src)
                print(expression)
        except (SyntaxError, ValueError) as err:
            print(type(err).__name__ + ":", err)
        except (KeyboardInterrupt, EOFError):  # <Control>-D, etc.
            return
