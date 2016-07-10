def p_exp_binop(p):
    """exp : exp PLUS exp
           | exp MINUS exp
           | exp TIMES exp
    """
    p[0] = ("binop", p[1], p[2], p[3])


precedence = (
    # lower precedence
    ("left", 'PLUS', 'MINUS'),
    ('left', 'TIMES', 'DIVIDE'),
    # higher precedence
)