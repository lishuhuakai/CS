"""The scheme_tokens module provides functions tokenize_line and tokenize_lines
for converting (iterators producing) strings into (iterators producing) lists
of tokens.  A token may be:

  * A number (represented as an int or float)
  * A boolean (represented as a bool)
  * A symbol (represented as a string)
  * A delimiter, including parentheses, dots, and single quotes

This file also includes some features of Scheme that have not been addressed
in the course, such as quasiquoting and Scheme strings.
"""

from ucb import main
import itertools
import string
import sys
import tokenize
# string.digits 只是一个常量而已 '0123456789'
# | 代表两个set的合并
_NUMERAL_STARTS = set(string.digits) | set('+-.')
_SYMBOL_CHARS = (set('!$%&*/:<=>?@^_~') | set(string.ascii_lowercase) |
                 set(string.ascii_uppercase) | _NUMERAL_STARTS)
_STRING_DELIMS = set('"')
_WHITESPACE = set(' \t\n\r') # 空格
_SINGLE_CHAR_TOKENS = set("()'`")
_TOKEN_END = _WHITESPACE | _SINGLE_CHAR_TOKENS | _STRING_DELIMS | {',', ',@'}
DELIMITERS = _SINGLE_CHAR_TOKENS | {'.', ',', ',@'} # delimiters 是分隔符吗？

def valid_symbol(s):
    """Returns whether s is not a well-formed value."""
    if len(s) == 0:
        return False
    for c in s:
        if c not in _SYMBOL_CHARS: # s的每一个元素都应该在_SYMBOL_CHARS_中，这样才会返回True
            return False
    return True

def next_candidate_token(line, k): # 下一个候选的token
    """A tuple (tok, k'), where tok is the next substring of line at or
    after position k that could be a token (assuming it passes a validity
    check), and k' is the position in line following that token.  Returns
    (None, len(line)) when there are no more tokens."""
    while k < len(line):
        c = line[k]
        if c == ';':
            return None, len(line)
        elif c in _WHITESPACE:
            k += 1
        elif c in _SINGLE_CHAR_TOKENS: # c是单个字符
            return c, k+1
        elif c == '#':  # Boolean values #t and #f
            return line[k:k+2], min(k+2, len(line))
        elif c == ',': # Unquote; check for @
            if k+1 < len(line) and line[k+1] == '@':
                return ',@', k+2
            return c, k+1
        elif c in _STRING_DELIMS: # 表示这个玩意是分割符么？
            if k+1 < len(line) and line[k+1] == c: # No triple quotes in Scheme
                return c+c, k+2
            line_bytes = (bytes(line[k:], encoding='utf-8'),)
            gen = tokenize.tokenize(iter(line_bytes).__next__)
            next(gen) # Throw away encoding token
            token = next(gen)
            if token.type != tokenize.STRING:
                raise ValueError("invalid string: {0}".format(token.string))
            return token.string, token.end[1]+k
        else:
            j = k
            while j < len(line) and line[j] not in _TOKEN_END:
                j += 1
            return line[k:j], min(j, len(line))
    return None, len(line)

def tokenize_line(line):
    """The list of Scheme tokens on line.  Excludes comments and whitespace."""
    result = []
    text, i = next_candidate_token(line, 0)
    while text is not None:
        if text in DELIMITERS: # 如果text是分隔符
            result.append(text)
        elif text == '#t' or text.lower() == 'true':
            result.append(True)
        elif text == '#f' or text.lower() == 'false':
            result.append(False)
        elif text == 'nil':
            result.append(text)
        elif text[0] in _SYMBOL_CHARS:
            number = False
            if text[0] in _NUMERAL_STARTS:
                try:
                    result.append(int(text)) # 将text转换为int
                    number = True
                except ValueError:
                    try:
                        result.append(float(text)) # 转换为float
                        number = True
                    except ValueError:
                        pass
            if not number:
                if valid_symbol(text):
                    result.append(text.lower())
                else:
                    raise ValueError("invalid numeral or symbol: {0}".format(text))
        elif text[0] in _STRING_DELIMS:
            result.append(text)
        else:
            print("warning: invalid token: {0}".format(text), file=sys.stderr)
            print("    ", line, file=sys.stderr)
            print(" " * (i+3), "^", file=sys.stderr)
        text, i = next_candidate_token(line, i)
    return result

def tokenize_lines(input):
    """An iterator that returns lists of tokens, one for each line of the
    iterable input sequence."""
    return map(tokenize_line, input)

def count_tokens(input):
    """Count the number of non-delimiter tokens in input."""
    return len(list(filter(lambda x: x not in DELIMITERS,
                           itertools.chain(*tokenize_lines(input)))))

@main
def run(*args):
    file = sys.stdin
    if args:
        file = open(args[0], 'r')
    print('counted', count_tokens(file), 'non-delimiter tokens')
