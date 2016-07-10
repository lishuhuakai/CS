# Underscoring the Magnitude
#
# Focus: Units 1 and 2, Regular Expressions and Lexical Analysis
#
# In this problem you will use regular expressions to specify tokens for a
# part of a new programming language. You must handle seven types of
# tokens:
#
#
#       PLUS            +
#       MINUS           -
#       TIMES           *
#       DIVIDE          /
#       IDENT           my_variable  Caps_Are_OK
#       STRING          'yes'  "also this"
#       NUMBER          123  123_456_789
#
# The last three merit a more detailed explanation.
#
# An IDENT token is a non-empty sequence of lower- and/or upper-case
# letters and underscores, but the first character cannot be an underscore.
# (Letters are a-z and A-Z only.) The value of an IDENT token is the string
# matched.
#
# A STRING token is zero or more of any character surrounded by 'single
# quotes' or "double quotes". In this language, there are no escape
# sequences, so "this\" is a string containing five characters. The value
# of a STRING token is the string matched with the quotes removed.
#
# A NUMBER is a a non-empty sequence of digits (0-9) and/or underscores,
# except that the first character cannot be an underscore. Many real-world
# languages actually support this, to make large number easier to read.
# All NUMBERs in this language are positive integers; negative signs and/or
# periods are not part of NUMBERs. The value of a NUMBER is the integer
# value of its digits with all of the underscores removed: the value of
# "12_34" is 1234 (the integer).
#
# For this problem we do *not* care about line number information. Only the
# types and values of tokens matter. Whitespace characters are ' \t\v\r'
# (and we have already filled them in for you below).
#
# Complete the lexer below.

import ply.lex as lex

tokens = ('PLUS', 'MINUS', 'TIMES', 'DIVIDE',
          'IDENT', 'STRING', 'NUMBER')

#####
#

# Place your token definition rules here.
t_PLUS = r'\+'
t_MINUS = r'-'
t_TIMES = r'\*'
t_DIVIDE = r'/'
t_IDENT = r'[a-zA-Z][a-zA-Z_]*'

def t_NUMBER(t):
    r'[0-9][0-9_]*'
    t.value = int(t.value.replace('_', ''))
    return t

# 这里的匹配需要注意一些问题，那就是，'要与'匹配，"要与"匹配
# r"'[^']*'"
def t_STRING(t):
    r'(?:\'[a-zA-Z\\ \"]*\')|(?:\"[a-zA-Z\\ \']*\")'
    t.value = t.value[1:-1]
    return t
#
#####

t_ignore = ' \t\v\r'

def t_error(t):
  print("Lexer: unexpected character " + t.value[0])
  t.lexer.skip(1)

# We have included some testing code to help you check your work. Since
# this is the final exam, you will definitely want to add your own tests.
lexer = lex.lex()

def test_lexer(input_string):
  lexer.input(input_string)
  result = [ ]
  while True:
    tok = lexer.token()
    if not tok: break
    result = result + [(tok.type,tok.value)]
  return result

question1 = " +   -   /   * "
answer1 = [('PLUS', '+'), ('MINUS', '-'), ('DIVIDE', '/'), ('TIMES', '*')]

print(test_lexer(question1) == answer1)

question2 = """ 'string "nested" \' "inverse 'nested'" """
answer2 = [('STRING', 'string "nested" '), ('STRING', "inverse 'nested'")]
print(test_lexer(question2))
print(test_lexer(question2) == answer2)

question3 = """ 12_34 5_6_7_8 0______1 1234 """
answer3 = [('NUMBER', 1234), ('NUMBER', 5678), ('NUMBER', 1), ('NUMBER', 1234)]
print(test_lexer(question3) == answer3)

question4 = """ 'he'llo w0rld 33k """
answer4 = [('STRING', 'he'), ('IDENT', 'llo'), ('IDENT', 'w'), ('NUMBER',
0), ('IDENT', 'rld'), ('NUMBER', 33), ('IDENT', 'k')]
print(test_lexer(question4) == answer4)
