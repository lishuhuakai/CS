import ply.lex as lex
import ply.yacc as yacc

# Fill in your code here.
####################################################
## lexer
tokens = (
    'LPAREN', 'RPAREN', 'STAR', 'BAR', 'LETTER'
)

# what the tokens look like, escaped for safety
t_LPAREN = r'\('
t_RPAREN = r'\)'
t_STAR = r'\*'
t_BAR = r'\|'

def t_LETTER(t):
    r'[A-Za-z0-9]'
    return t

t_ignore = ' \t\r\v'

def t_error(t):
  print("Lexer: unexpected character " + t.value[0])
  t.lexer.skip(1)

####################################################

####################################################
## parse

start = 're' # the start symbol in our grammar

# precedenct ordering
precedence = (
    ('left', 'BAR'),
    ('left', 'CONCAT'),
    ('left', 'STAR')
)

def p_re_letter(p):
    're : LETTER %prec CONCAT'
    p[0] = ('letter', p[1])
def p_re_concat(p):
    're : re re %prec CONCAT'
    p[0] = ('concat', p[1], p[2])
def p_re_bar(p):
    're : re BAR re'
    p[0] = ('bar', p[1], p[3])
def p_re_star(p):
    're : re STAR'
    p[0] = ('star', p[1])
def p_re_paren(p):
    're : LPAREN re RPAREN'
    p[0] = p[2]
def p_error(p):
    raise SyntaxError
state_counter = 3

def interpret(ast): # goal here is to create a nfsm out of an ast
    global state_counter
    start_state = 1
    accepting = [2]
    state_counter = 3
    edges = {}
    def add_edge(a, b, l): # helper function to add edges
        if (a, l) in edges:
            edges[(a, l)] = [b] + edges[(a, l)]
        else:
            edges[(a, l)] = [b]
    def new_state():
        global state_counter
        x = state_counter
        state_counter = state_counter + 1
        return x
    def walk(re, here, goal): # helper fuction to walk the ast
        retype = re[0]
        if retype == 'letter':
            add_edge(here, goal, re[1])
        elif retype == 'concat':
            mid = new_state()
            walk(re[1], here, mid)
            walk(re[2], mid, goal)
        elif retype == 'bar':
            walk(re[1], here, goal)
            walk(re[2], here, goal)
        elif retype == 'star':
            walk(re[1], here, here)
            add_edge(here, goal, None)
        else:
            print('OOPS' + re)
    walk(ast, start_state, accepting[0])
    return (edges, accepting, start_state)


####################################################
lexer = lex.lex() # 需要构建一个词法分析器
parser = yacc.yacc() # 语法分析器

def lexing(str):
    # 做词法分析
    lexer.input(str)
    while True:
        tok = lexer.token()
        if not tok: break
        print(tok)

def parsing(input_string):
    try:
        parse_tree = parser.parse(input_string, lexer=lexer)
        return parse_tree
    except:
        return "error"

# lexing('a(b*)cd')
print(parsing('abc(de*)*c')) # 毫无疑问，这个式子解释得非常完美
# 构建一棵语法树



