# Wes Weimer
# 
# This is a set of regular expressions defining a lexer for HTML with
# embedded JavaScript fragments. 
#

import ply.lex as lex

tokens = (
        'LANGLE',       # < 
        'LANGLESLASH',  # </
        'RANGLE',       # > 
        'SLASHRANGLE',  # /> 
        'EQUAL',        # = 
        'STRING',       # "144" 
        'WORD',         # 'Welcome' in "Welcome to my webpage."
        'JAVASCRIPT',   # embedded JavaScript Fragment
) 

states = (
        ('javascript', 'exclusive'),    # <script type="text/javascript"> 
        ('htmlcomment', 'exclusive'),   # <!-- 
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

def t_javascript(token): 
        r'\<script\ type=\"text\/javascript\"\>'
        token.lexer.code_start = token.lexer.lexpos
        token.lexer.level = 1
        token.lexer.begin('javascript') 

def t_javascript_end(t):
        r'</script>'
        t.value = t.lexer.lexdata[t.lexer.code_start:t.lexer.lexpos-9]
        t.type = "JAVASCRIPT"
        t.lexer.lineno += t.value.count('\n') 
        t.lexer.begin('INITIAL') 
        return t 

def t_javascript_error(t):
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
        r'(?:"[^"]*"|\'[^\']*\')'
        t.value = t.value[1:-1] # drop "surrounding quotes" 
        return t

def t_WORD(t):
        r'[^ \t\v\r\n<>=]+' 
        return t

t_ignore                = ' \t\v\r'
t_htmlcomment_ignore    = ' \t\v\r'
t_javascript_ignore     = ' \t\v\r'

def t_newline(t):
        r'\n'
        t.lexer.lineno += 1

def t_error(t):
        print("HTML Lexer: Illegal character " + t.value[0])
        t.lexer.skip(1) 
