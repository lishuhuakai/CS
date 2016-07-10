import ply.lex as lex

tokens = (
        'LANGLE',       # <
        'LANGLESLASH',  # </
        'RANGLE',       # >
        'SLASHRANGLE',  # />
        'EQUAL',        # =
        'STRING',       # "144"
        'WORD',         # 'Welcome' in "Welcome to my webpage."
)

t_ignore                = ' \t\v\r' # shortcut for whitespace

states = (
    ('htmlcomment', 'exclusive'), # <!--
)

def t_htmlcomment(t):
    r'<!--'
    t.lexer.begin('htmlcomment')

def t_htmlcomment_end(t):
    r'-->'
    t.lexer.lineno += t.value.count('\n')
    t.lexer.begin('INITIAL')
    pass

def t_htmlcomment_error(t):
    t.lexer.skip(1)

def t_LANGLESLASH(t):
        r'</'
        return t

def t_LANGLE(t):
        r'<'
        return t

def t_SLASHRANGLE(t):
        r'/>'
        return t

def t_RANGLE(t):
        r'>'
        return t

def t_EQUAL(t):
        r'='
        return t

def t_STRING(t):
        r'"[^"]*"'
        t.value = t.value[1:-1] # drop "surrounding quotes"
        return t

def t_WORD(t):
        r'[^ <>\n]+'
        return t

webpage = '"This" is <b>my</b> webpage!'
webpage = """Tricky "string" <i>output<i/>!"""
htmllexer = lex.lex()
htmllexer.input(webpage)
while True:
        tok = htmllexer.token()
        if not tok: break
        print(tok)