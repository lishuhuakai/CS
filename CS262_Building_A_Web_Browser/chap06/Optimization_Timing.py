# Optimization Phase
def optimize(tree): # expression trees only
    etype = tree[0]
    if etype == 'binop':
        # Fix up this code so that it handles a + ( 5 * 0 )
        # recursively! QUIZ!
        a = optimize(tree[1])
        op = tree[2]
        b = optimize(tree[3])
        if op == '*' and b == ('number', '1'):
            return a
        # QUIZ: It only handles A * 1
        # Add in support for A * 0 and A + 0
        elif op == '*' and b == ('number', '0'):
            return ('number', 0)
        elif op == '+' and b == ('number', '0'):
            return a
        return tree # maybe t * x
    return tree
