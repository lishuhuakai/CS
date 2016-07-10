# JavaScript: Numbers & Strings
#
# In this exercise you will finish out the token definitions for JavaScript
# by handling Numbers, Identifiers and Strings.
#
# We have split the lexing of JavaScript into two exercises so that
# you have a chance to demonstrate your mastery of the concepts
# independently (i.e., so that you can get one of them right even if the
# other proves difficult). We could easily make a full JavaScript lexer by
# putting all of the rules together.
#
# For this assignment, a JavaScript IDENTIFIER must start with an upper- or
# lower-case character. It can then contain any number of upper- or
# lower-case characters or underscores. Its token.value is the textual
# string of the identifier.
#       Yes:    my_age
#       Yes:    cRaZy
#       No:     _starts_with_underscore
#
# For this assignment, a JavaScript NUMBER is one or more digits. A NUMBER
# can start with an optional negative sign. A NUMBER can contain a decimal
# point, which can then be followed by zero or more additional digits. Do
# not worry about hexadecimal (only base 10 is allowed in this problem).
# The token.value of a NUMBER is its floating point value (NOT a string).
#       Yes:    123
#       Yes:    -456
#       Yes:    78.9
#       Yes:    10.
#       No:     +5
#       No:     1.2.3
#
# For this assignment, a JavaScript STRING is zero or more characters
# contained in double quotes. A STRING may contain escaped characters.
# Notably, \" does not end a string. The token.value of a STRING is
# its contents (not including the outer double quotes).
#       Yes:    "hello world"
#       Yes:    "this has \"escaped quotes\""
#       No:     "no"t one string"
#
# Hint: float("2.3") = 2.3

import ply.lex as lex

tokens = (
        'IDENTIFIER',
        'NUMBER',
        'STRING',
)

#
# Write your code here.
#


t_ignore                = ' \t\v\r' # whitespace

def t_IDENTIFIER(t):
    r'[a-zA-Z][a-zA-Z_]*'
    return t

def t_NUMBER(t):
    r'-?[0-9]+(?:\.[0-9]+)?'
    if t.value.find('.') != -1:
        t.value = float(t.value)
    else:
        t.value = int(t.value)
    return t

def t_STRING(t): # 关键在于如何匹配这样的字符串"this has \"escaped quotes\""
    # 这些确实是一个很大的问题啊！
    r'"(?:[^"\\]|(?:\\.))*"'
    t.value = t.value[1 : -1]
    return t

def t_newline(t):
        r'\n'
        t.lexer.lineno += 1

def t_error(t):
        print("JavaScript Lexer: Illegal character " + t.value[0])
        t.lexer.skip(1)

# We have included two test cases to help you debug your lexer. You will
# probably want to write some of your own.

lexer = lex.lex()

def test_lexer(input_string):
  lexer.input(input_string)
  result = [ ]
  while True:
    tok = lexer.token()
    if not tok: break
    result = result + [tok.type,tok.value]
  return result

input1 = 'some_identifier -12.34 "a \\"escape\\" b"'
output1 = ['IDENTIFIER', 'some_identifier', 'NUMBER', -12.34, 'STRING',
'a \\"escape\\" b']
print(test_lexer(input1) == output1)


input2 = '-12x34'
output2 = ['NUMBER', -12.0, 'IDENTIFIER', 'x', 'NUMBER', 34.0]
print(test_lexer(input2) == output2)
