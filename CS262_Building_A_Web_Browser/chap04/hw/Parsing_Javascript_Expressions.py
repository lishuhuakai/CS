# Parsing JavaScript Expressions
#
# In this exercise you will write a Parser for a subset of JavaScript. This
# will invole writing parsing rewrite rules (i.e., encoding a context-free
# grammar) and building up a parse tree (also called a syntax tree) of the
# result. This question may seem long at first, but it can be answered in a
# little over 50 lines.
#
# We have split the parsing of JavaScript into two exercises so that you
# have a chance to demonstrate your mastery of the concepts independently
# (i.e., so that you can get one of them right even if the other proves
# difficult). We could easily make a full JavaScript parser by putting all
# of the rules together.
#
# In this second parse, we wil handle JavaScript expressions. The
# JavaScript tokens we use will be the same ones we defined together in the
# Homework for Unit 2. (Even if you did not complete Homework 2, the
# correct tokens will be provided here.)
#
# Let's walk through our JavaScript expression grammar. We'll describe it
# somewhat informally in text: your job for this homework problem is to
# translate this description into a valid parser!
#
# First, there are a number of "base cases" in our grammar -- simple
# expressions that are not recursive. In each case, the abstract syntax
# tree is a simple tuple.
#
#       exp -> IDENTIFIER       # ("identifier",this_identifier_value)
#       exp -> NUMBER           # ("number",this_number_value)
#       exp -> STRING           # ("string",this_string_value)
#       exp -> TRUE             # ("true","true")
#       exp -> FALSE            # ("false","false")
#
# There are also two unary expressions types -- expressions built
# recursively from a single child.
#
#       exp -> NOT exp          # ("not", child_parse_tree)
#       exp -> ( exp )          # child_parse_tree
#
# For NOT, the parse tree is a simple tuple. For parentheses, the parse
# tree is even simpler: just return the child parse tree unchanged!
#
# There are many binary expressions. To deal with ambiguity, we have to
# assign them precedence and associativity.  I will list the lowest
# predecence binary operators first, and then continue in order of
# increasing precedence:
#
#    exp ->   exp || exp        # lowest precedence, left associative
#           | exp && exp        # higher precedence, left associative
#           | exp == exp        # higher precedence, left associative
#           | exp < exp         # /---
#           | exp > exp         # | higher precedence,
#           | exp <= exp        # | left associative
#           | exp >= exp        # \---
#           | exp + exp         # /--- higher precedence,
#           | exp - exp         # \--- left associative
#           | exp * exp         # /--- higher precedence,
#           | exp / exp         # \--- left associative
#
# In each case, the parse tree is the tuple:
#
#       ("binop", left_child, operator_token, right_child)
#
# For this assignment, the unary NOT operator has the highest precedence of
# all and is right associative.
#
# Finally, it is possible to have a function call as an expression:
#
#       exp -> IDENTIFIER ( optargs )
#
# The parse tree is the tuple ("call", function_name, arguments).
#
#       optargs ->
#       optargs -> args
#       args -> exp , args
#       args -> exp
#
# It is also possible to have anonymous functions (sometimes called
# lambda expressions) in JavaScript.
#
#       exp -> function ( optparams ) compoundstmt
#
# However, for this assignment you are not responsible for lambda
# expressions.
#
# Arguments are comma-separated expressions. The parse tree for args or
# optargs is just the list of the parse trees of the component expressions.
#
# Recall the names of our tokens:
#
# 'ANDAND',       # &&          | 'LT',           # <
# 'COMMA',        # ,           | 'MINUS',        # -
# 'DIVIDE',       # /           | 'NOT',          # !
# 'ELSE',         # else        | 'NUMBER',       # 1234
# 'EQUAL',        # =           | 'OROR',         # ||
# 'EQUALEQUAL',   # ==          | 'PLUS',         # +
# 'FALSE',        # FALSE       | 'RBRACE',       # }
# 'FUNCTION',     # function    | 'RETURN',       # return
# 'GE',           # >=          | 'RPAREN',       # )
# 'GT',           # >           | 'SEMICOLON',    # ;
# 'IDENTIFIER',   # factorial   | 'STRING',       # "hello"
# 'IF',           # if          | 'TIMES',        # *
# 'LBRACE',       # {           | 'TRUE',         # TRUE
# 'LE',           # <=          | 'VAR',          # var
# 'LPAREN',       # (           |
import ply.yacc as yacc
import ply.lex as lex
import JS.jstokens                 # use our JavaScript lexer
from JS.jstokens import tokens     # use our JavaScript tokens

start = 'exp'    # we'll start at expression this time

precedence = (
        # Fill in the precedence and associativity. List the operators
        # in order of _increasing_ precedence (start low, go to high).

)

# Here's the rules for simple expressions.
def p_exp_identifier(p):
    'exp : IDENTIFIER'
    p[0] = ("identifier",p[1])

def p_exp_number(p):
    'exp : NUMBER'
    p[0] = ('number',p[1])

def p_exp_string(p):
    'exp : STRING'
    p[0] = ('string',p[1])

def p_exp_true(p):
    'exp : TRUE'
    p[0] = ('true','true')

def p_exp_false(p):
    'exp : FALSE'
    p[0] = ('false','false')

def p_exp_not(p):
    'exp : NOT exp'
    p[0] = ('not', p[2])

def p_exp_parens(p):
    'exp : LPAREN exp RPAREN'
    p[0] = p[2]

# This is what the rule for anonymous functions would look like, but since
# they involve statements they are not part of this assignment. Leave this
# commented out, but feel free to use it as a hint.
#
## def p_exp_lambda(p):
##         'exp : FUNCTION LPAREN optparams RPAREN compoundstmt'
##         p[0] = ("function",p[3],p[5])

######################################################################
# Fill in the rest of the grammar for expressions.
#
# This can be done in about 50 lines using about 12 p_Something()
# definitions. Remember that you can save time by lumping the binary
# operator rules together.
######################################################################
def p_exp_call(p):
    'exp : IDENTIFIER LPAREN optargs RPAREN'
    p[0] = ('call', p[1], p[3])

def p_optargs_empty(p):
    'optargs :'
    p[0] = []

def p_optargs_args(p):
    'optargs : args'
    p[0] = p[1]

def p_args_exp(p):
    'args : exp'
    p[0] = [p[1]]
def p_args(p):
    'args : exp COMMA args'
    p[0] = [p[1]] + p[3]



def p_exp_binop(p):
    """
    exp : exp ANDAND exp
        | exp OROR exp
        | exp EQUALEQUAL exp
        | exp LT exp
        | exp GT exp
        | exp LE exp
        | exp GE exp
        | exp PLUS exp
        | exp MINUS exp
        | exp TIMES exp
        | exp DIVIDE exp
    """
    p[0] = ("binop", p[1], p[2], p[3])


precedence = (
    ('left', 'OROR'),
    ('left', 'ANDAND'),
    ('left', 'EQUALEQUAL'),
    ('left', 'LT', 'GT', 'LE', 'GE'),
    ('left', 'PLUS', 'MINUS'),
    ('left', 'TIMES', 'DIVIDE'),
    ('right', 'NOT')
)
# We have included a few tests. You will likely want to write your own.

jslexer = lex.lex(module=JS.jstokens)
jsparser = yacc.yacc()

def test_parser(input_string):  # invokes your parser to get a tree!
        jslexer.input(input_string)
        parse_tree = jsparser.parse(input_string,lexer=jslexer)
        return parse_tree

# Simple binary expression.
jstext1 = "x + 1"
jstree1 = ('binop', ('identifier', 'x'), '+', ('number', 1.0))
print(test_parser(jstext1) == jstree1)

# Simple associativity.
jstext2 = "1 - 2 - 3"   # means (1-2)-3
jstree2 = ('binop', ('binop', ('number', 1.0), '-', ('number', 2.0)), '-',
('number', 3.0))
print(test_parser(jstext2) == jstree2)

# Precedence and associativity.
jstext3 = "1 + 2 * 3 - 4 / 5 * (6 + 2)"
jstree3 = ('binop', ('binop', ('number', 1.0), '+', ('binop', ('number', 2.0), '*', ('number', 3.0))), '-', ('binop', ('binop', ('number', 4.0), '/', ('number', 5.0)), '*', ('binop', ('number', 6.0), '+', ('number', 2.0))))
print(test_parser(jstext3) == jstree3)

# String and boolean constants, comparisons.
jstext4 = ' "hello" == "goodbye" || true && false '
jstree4 = ('binop', ('binop', ('string', 'hello'), '==', ('string', 'goodbye')), '||', ('binop', ('true', 'true'), '&&', ('false', 'false')))
#print(test_parser(jstext4))
print(test_parser(jstext4) == jstree4)

# Not, precedence, associativity.
jstext5 = "! ! tricky || 3 < 5"
jstree5 = ('binop', ('not', ('not', ('identifier', 'tricky'))), '||', ('binop', ('number', 3.0), '<', ('number', 5.0)))
print(test_parser(jstext5))
print(test_parser(jstext5) == jstree5)

# nested function calls!
jstext6 = "apply(1, 2 + eval(recursion), sqrt(2))"
jstree6 = ('call', 'apply', [('number', 1.0), ('binop', ('number', 2.0), '+', ('call', 'eval', [('identifier', 'recursion')])), ('call', 'sqrt', [('number', 2.0)])])
print(test_parser(jstext6))
