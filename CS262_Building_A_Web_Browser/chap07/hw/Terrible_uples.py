# Terrible Tuples
#
# Focus: Units 3 and 4, Grammars and Parsing
#
# In this problem you will use context-free grammars to specify some
# expression for part of a new programming language. We will specify tuples
# and lists, as in Python. We will consider four types of expressions.
#
# 1. An expression can be a single NUMBER token. In this case, your parser
# should return ("number",XYZ) where XYZ is the value of the NUMBER
# token.
#
# 2. An expression can be LPAREN expression RPAREN . In this case, your
# parser should return the value of the expression inside the parentheses.
#
# 3. An expression can be LPAREN "a list of more than one comma-separated
# expressions" RPAREN. This should remind you of tuples in Python:
#
#               (1,2,3)
#
# The inner expressions are 1 2 and 3, and they are separated by commas.
# In this case, your parser should return ("tuple", ...) where ... is a
# list of the child expression values. For example, for (1,2) you should
# return ("tuple",[("number",2),("number",3)]).
#
# 4. An expression can be LBRACKET "a list of one or more comma-separated
# expressions" RBRACKET. This should remind you of lists in Python:
#
#               [7,8,9]
#
# These parse exactly like tuples, except that they use square brackets
# instead of parentheses, and singleton lists like [7] are valid. Your
# parser should return ("list", ...) as above, so [7,8] would return
# ("list",[("number",7),("number",8)]).
#
# Complete the parser below.

import ply.lex as lex
import ply.yacc as yacc

start = 'exp'    # the start symbol in our grammar

#####
#

# Place your grammar definition rules here.
# 这里好像没有制定所谓的规则
def p_exp_number(p):
    'exp : NUMBER'
    p[0] = ('number', p[1])

def p_exp_braket(p):
    'exp : LPAREN exp RPAREN'
    p[0] = p[2]

def p_exp_tuple(p):
    'exp : LPAREN exps RPAREN'
    p[0] = ('tuple', p[2])

# a list of more than one comma-separated expressions
def p_exps(p):
    'exps : exp COMMA exp expss'
    p[0] = [p[1], p[3]] + p[4]

# 这里要说明的一点是，确实要消除左递归
#def p_exps_exp(p):
#   'exps : exps COMMA exp'
#    p[0] = p[1] + p[3]

def p_expss(p):
    'expss : COMMA exp expss'
    p[0] = [p[2]] + p[3]

def p_expss_empty(p):
    'expss :'
    p[0] = []

def p_exp_list(p):
    'exp : LBRACKET one_or_more_exp RBRACKET'
    p[0] = ('list', p[2])

def p_one_or_more_exp(p):
    'one_or_more_exp : exp'
    p[0] = [p[1]]

def p_one_or_more_exp_ex(p):
    'one_or_more_exp : exp COMMA one_or_more_exp'
    p[0] = [p[1]] + p[3]

#
#####

def p_error(p):
        raise SyntaxError


# We have provided a lexer for you. You should not change it.

tokens = ('LPAREN', 'RPAREN', 'LBRACKET', 'RBRACKET', 'NUMBER', 'COMMA')

def t_NUMBER(token):
        r"[0-9]+"
        token.value = int(token.value)
        return token

t_ignore        = ' \t\v\r'
t_COMMA         = r','
t_LPAREN        = r'\('
t_RPAREN        = r'\)'
t_LBRACKET      = r'\['
t_RBRACKET      = r'\]'

def t_error(t):
  print("Lexer: unexpected character " + t.value[0])
  t.lexer.skip(1)

# We have included some testing code to help you check your work. Since
# this is the final exam, you will definitely want to add your own tests.
lexer = lex.lex()

def test(input_string):
  lexer.input(input_string)
  parser = yacc.yacc()
  try:
    parse_tree = parser.parse(input_string, lexer=lexer)
    return parse_tree
  except:
    return "error"

question1 = " 123 "
answer1 = ('number', 123)
print(test(question1) == answer1)

question2 = " (123) "
print(test(question2))
print(test(question2) == answer1)

question3 = " (1,2,3) "
print(test(question3))
answer3 = ('tuple', [('number', 1), ('number', 2), ('number', 3)])
print(test(question3) == answer3)

question4 = " [123] "
print(test(question4))
answer4 = ('list', [('number', 123)])
print(test(question4) == answer4)

question5 = " [1,2,3] "
answer5 = ('list', [('number', 1), ('number', 2), ('number', 3)])
print(test(question5) == answer5)

question6 = " [(1,2),[3,[4]]] "
answer6 = ('list', [('tuple', [('number', 1), ('number', 2)]), ('list', [('number', 3), ('list', [('number', 4)])])])
print(test(question6) == answer6)

question7 = " (1,2) [3,4) "
answer7 = "error"
print(test(question7) == answer7)


