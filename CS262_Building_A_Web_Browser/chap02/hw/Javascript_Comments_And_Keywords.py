# JavaScript: Comments & Keywords
#
# In this exercise you will write token definition rules for all of the
# tokens in our subset of JavaScript *except* IDENTIFIER, NUMBER and
# STRING. In addition, you will handle // end of line comments
# as well as /* delimited comments */.
#
# We will assume that JavaScript is case sensitive and that keywords like
# 'if' and 'true' must be written in lowercase. There are 26 possible
# tokens that you must handle. The 'tokens' variable below has been
# initialized below, listing each token's formal name (i.e., the value of
# token.type). In addition, each token has its associated textual string
# listed in a comment. For example, your lexer must convert && to a token
# with token.type 'ANDAND' (unless the && is found inside a comment).
#
# Hint 1: Use an exclusive state for /* comments */. You may want to define
# t_comment_ignore and t_comment_error as well.

import ply.lex as lex

def test_lexer(lexer,input_string):
  lexer.input(input_string)
  result = [ ]
  while True:
    tok = lexer.token()
    if not tok: break
    result = result + [tok.type]
  return result

tokens = (
        'ANDAND',       # &&
        'COMMA',        # ,
        'DIVIDE',       # /
        'ELSE',         # else
        'EQUAL',        # =
        'EQUALEQUAL',   # ==
        'FALSE',        # false
        'FUNCTION',     # function
        'GE',           # >=
        'GT',           # >
#       'IDENTIFIER',   #### Not used in this problem.
        'IF',           # if
        'LBRACE',       # {
        'LE',           # <=
        'LPAREN',       # (
        'LT',           # <
        'MINUS',        # -
        'NOT',          # !
#       'NUMBER',       #### Not used in this problem.
        'OROR',         # ||
        'PLUS',         # +
        'RBRACE',       # }
        'RETURN',       # return
        'RPAREN',       # )
        'SEMICOLON',    # ;
#       'STRING',       #### Not used in this problem.
        'TIMES',        # *
        'TRUE',         # true
        'VAR',          # var
        'CM',           # //
)

#
# Write your code here.
#
states = (
    ('comment', 'exclusive'), # //
)


t_ignore = ' \t\v\r' # whitespace

"""
其实有更加简单的方法，你可以查看一下你后面的代码，可以发现，这些东西只是匹配一下就完事了，那么
为什么要大张旗鼓地全部写一个函数呢？没有必要的！
t_ANDAND = r'&&'
t_COMMA = r','
...

"""
def t_comment_error(t):
    #print("JavaScript Lexer: Illegal character " + t.value[0])
    t.lexer.skip(1)  # 直接跳过一个char

def t_CM(t):
    r'//[^\n]*'
    pass

def t_comment(t):
    r'/\*'
    t.lexer.begin('comment')

def t_comment_end(t):
    r'\*/'
    t.lexer.lineno += t.value.count('\n')
    t.lexer.begin('INITIAL')
    pass

def t_newline(t):
        r'\n'
        t.lexer.lineno += 1

def t_error(t):
        print("JavaScript Lexer: Illegal character " + t.value[0])
        t.lexer.skip(1)

def t_ANDAND(t):
    r'&&'
    return t

def t_COMMA(t):
    r','
    return t

def t_DIVIDE(t):
    r'/'
    return t

def t_ELSE(t):
    r'else'
    return t

def t_EQUALEQUAL(t):
    r'=='
    return t

def t_EQUAL(t):
    r'='
    return t



def t_FALSE(t):
    r'false'
    return t

def t_FUNCTION(t):
    r'function'
    return t

def t_GE(t):
    r'>='
    return t

def t_GT(t):
    r'>'
    return t

def t_IF(t):
    r'if'
    return t

def t_LBRACE(t):
    r'{'
    return t

def t_LE(t):
    r'<='
    return t

def t_LPAREN(t):
    r'\('
    return t

def t_LT(t):
    r'<'
    return t

def t_MINUS(t):
    r'-'
    return t

def t_NOT(t):
    r'!'
    return t

def t_OROR(t):
    r'\|\|'
    return t

def t_PLUS(t):
    r'\+'
    return t

def t_RBRACE(t):
    r'}'
    return t

def t_RETURN(t):
    r'return'
    return t

def t_RPAREN(t):
    r'\)'
    return t

def t_SEMICOLON(t):
    r';'
    return t

def t_TIMES(t):
    r'\*'
    return t

def t_TRUE(t):
    r'true'
    return t

def t_VAR(t):
    r'var'
    return t


# We have included two test cases to help you debug your lexer. You will
# probably want to write some of your own.

lexer = lex.lex()

def test_lexer(input_string):
  lexer.input(input_string)
  result = [ ]
  while True:
    tok = lexer.token()
    if not tok: break
    result = result + [tok.type]
  return result

input1 = """ - !  && () * , / ; { || } + < <= = == > >= else false function
if return true var """

output1 = ['MINUS', 'NOT', 'ANDAND', 'LPAREN', 'RPAREN', 'TIMES', 'COMMA',
'DIVIDE', 'SEMICOLON', 'LBRACE', 'OROR', 'RBRACE', 'PLUS', 'LT', 'LE',
'EQUAL', 'EQUALEQUAL', 'GT', 'GE', 'ELSE', 'FALSE', 'FUNCTION', 'IF',
'RETURN', 'TRUE', 'VAR']

print(test_lexer(input1) == output1)

input2 = """
if // else mystery
=/*=*/=
true /* false
*/ return"""

output2 = ['IF', 'EQUAL', 'EQUAL', 'TRUE', 'RETURN']

print(test_lexer(input2) == output2)