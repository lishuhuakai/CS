import ply.lex as lex

tokens = (
    "NUMBER",
    "PLUS",
    "MINUS",
    "TIMES",
    "DIVIDE",
)
t_ignore = ' '

t_PLUS = r'\+'
t_MINUS = r'-'
t_TIMES = r'\*'
t_DIVIDE = r'/'

def t_NUMBER(t):
    r'[0-9]+'
    t.value = int(t.value)
    return t

def t_newLine(token):
    r'\n'
    token.lexer.lineno += 1
    pass

def t_error(t):
    print("There is a error!")
    t.lexer.skip(1) # pass

if __name__ == "__main__":
    lexer = lex.lex()
    lexer.input("1+1")
while True:
    tok = lexer.token()
    if not tok: break
    print(tok)

