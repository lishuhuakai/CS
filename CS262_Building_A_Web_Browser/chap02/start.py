import ply.lex as lex
# Specify html + JavaScript

# LANGLE       <
# LANGLESLASH  </
# RANGLE       >
# EQUAL        =
# STRING       "google.com"
# WORD         welcome!

# We will use regular expressions to specify tokens
def t_LANGLE(token):
    r"</"
    return token

def t_STRING(token):
    pattern = r'"[^"]*"'
    return token

def t_WHITESPACE(token):
    pattern = r' '
    return token

def t_WORD(token):
    pattern = r'[^ <>]+'
    return token

def t_NUMBER(token):
    r'[0-9]+'
    token.value = int(token.value)
    return token