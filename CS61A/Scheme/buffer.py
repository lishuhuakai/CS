"""The buffer module assists in iterating through lines and tokens."""

import math

class Buffer:
    """A Buffer provides a way of accessing a sequence of tokens across lines.

    Its constructor takes an iterator, called "the source", that returns the
    next line of tokens as a list each time it is queried, or None to indicate
    the end of data.

    The Buffer in effect concatenates the sequences returned from its source
    and then supplies the items from them one at a time through its pop()
    method, calling the source for more sequences of items only when needed.

    In addition, Buffer provides a current method to look at the
    next item to be supplied, without sequencing past it.

    The __str__ method prints all tokens read so far, up to the end of the
    current line, and marks the current token with >>.

    >>> buf = Buffer(iter([['(', '+'], [15], [12, ')']]))
    >>> buf.pop()
    '('
    >>> buf.pop()
    '+'
    >>> buf.current()
    15
    >>> print(buf)
    1: ( +
    2:  >> 15
    >>> buf.pop()
    15
    >>> buf.current()
    12
    >>> buf.pop()
    12
    >>> print(buf)
    1: ( +
    2: 15
    3: 12 >> )
    >>> buf.pop()
    ')'
    >>> print(buf)
    1: ( +
    2: 15
    3: 12 ) >>
    >>> buf.pop()  # returns None
    """
    def __init__(self, source):
        self.index = 0
        self.lines = []
        self.source = source
        self.current_line = ()
        self.current()

    def pop(self):
        """Remove the next item from self and return it. If self has
        exhausted its source, returns None."""
        current = self.current()
        self.index += 1 # 指示器向前推进一项
        return current # 返回pop而出的item

    @property # 这里实际上是一个装饰器
    def more_on_line(self): # 用于判断一行之中是否还有更多的item
        return self.index < len(self.current_line)

    def current(self):
        """Return the current element, or None if none exists."""
        while not self.more_on_line: # 如果一行之中没有了更多的item
            self.index = 0 # 那么将指示器置为0
            try:
                self.current_line = next(self.source) # source是一个可以迭代的对象，是吧！
                self.lines.append(self.current_line) # lines将下一行的数据记录下来
            except StopIteration:
                self.current_line = ()
                return None
        return self.current_line[self.index] # 否则的话，返回当前行指示器指示的位置的item

    def __str__(self): # __str__属性类似于Java里面的toString方法
        """Return recently read contents; current element marked with >>."""
        # Format string for right-justified line numbers
        n = len(self.lines)
        msg = '{0:>' + str(math.floor(math.log10(n))+1) + "}: "

        # Up to three previous lines and current line are included in output
        s = ''
        for i in range(max(0, n-4), n-1):
            s += msg.format(i+1) + ' '.join(map(str, self.lines[i])) + '\n'
        s += msg.format(n)
        s += ' '.join(map(str, self.current_line[:self.index]))
        s += ' >> '
        s += ' '.join(map(str, self.current_line[self.index:]))
        return s.strip()

# Try to import readline for interactive history
try:
    import readline
except:
    pass

class InputReader:
    """An InputReader is an iterable that prompts the user for input."""
    def __init__(self, prompt):
        self.prompt = prompt # prompt用于提示

    def __iter__(self): # 实现了__iter__方法的对象是可迭代的
        while True:
            yield input(self.prompt)
            self.prompt = ' ' * len(self.prompt)

class LineReader:
    """A LineReader is an iterable that prints lines after a prompt."""
    def __init__(self, lines, prompt, comment=";"):
        self.lines = lines
        self.prompt = prompt # 提示符
        self.comment = comment # 注释么

    def __iter__(self): # 可迭代的
        while self.lines:
            line = self.lines.pop(0).strip('\n') # pop出的是一个str对象，strip去除了最后的换行符
            if (self.prompt is not None and line != "" and
                not line.lstrip().startswith(self.comment)): # 指这一行不是注释啦
                print(self.prompt + line)
                self.prompt = ' ' * len(self.prompt)
            yield line
        raise EOFError
