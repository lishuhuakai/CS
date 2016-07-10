import ply.lex as lex
import ply.yacc as yacc

tokens = (
    'LANGLE', # <
    'LANGLESLASH', # </
    'RANGLE', # >
    'EQUAL', # =
    'STRING', # "hello"
    'WORD', # Welcome!
)

states = (
    ('htmlcomment', 'exclusive'),
)

t_ignore = ' ' # shortcut for whitespace

def t_htmlcomment(token):
    r'<!--'
    token.lexer.begin('htmlcomment')

def t_htmlcomment_end(token):
    r'-->'
    token.lexer.lineno += token.value.count('\n')
    token.lexer.begin('INITAL')

def t_htmlcomment_error(token):
    token.lexer.skip(1) # pass

def t_newLine(token):
    r'\n'
    token.lexer.lineno += 1
    pass

def t_LANGLESLASH(token):
    r'</'
    return token

def t_LANGLE(token):
    r'<'
    return token

def t_RANGLE(token): #右括号
    r'>'
    return token

def t_NUMBER(token):
    r'[0-9]+'
    token.value = int(token.value)
    return token

def t_WHITESPACES(token):
    r" "
    pass


def t_EQUAL(token):
    r'='
    return token

def t_STRING(token):
    r'"[^"]*"'
    token.value = token.value[1 : -1]
    return token

def t_WORD(token):
    r'[^ <>\n]+'
    return token

webpage = "This is <b>my</b> webpage"

htmllexer = lex.lex()
htmllexer.input(webpage)
while True:
    tok = htmllexer.token()
    if not tok: break
    print(tok)



