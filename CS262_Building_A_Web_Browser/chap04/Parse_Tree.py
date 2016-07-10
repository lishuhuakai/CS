def t_STRING(t):
    r'"[^"]*"'
    t.value = t.value[1:-1]
    return t

# p - parsing
# exp when exp is a number
# p is a parse tree
def p_exp_number(p):
    'exp : NUMBER'
    p[0] = ("number", p[1])
# p[0] is our returned parse tree

def p_exp_not(p):
    'exp : NOT exp'
    p[0] = ("not", p[2])

def p_html(p):
    'html : elt html'
    p[0] = [p[1]] + p[2]

def p_html_empty(p):
    'html : '
    p[0] = []
def p_elt_word(p):
    'elt : WORD'
    p[0] = ("word-element", p[1])

def p_elt_tag(p):
    #'<span color="red">Text!</span>'
    'elt : LANGLE WORD tag_args RANGLE html LANGLESLASH WORD RANGLE'
    p[0] = ("tag-element", p[2], p[3], p[5], p[7])
# input = hello <b> baba </b> yaga
[ ("word-element", "hello")
  ("tag-element", "b", [],
   [("word-element", "baba")])
  ("word-element", "yaya")
]
