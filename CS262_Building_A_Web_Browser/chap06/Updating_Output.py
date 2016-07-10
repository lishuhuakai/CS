# Quiz -- JS Interpreter, 'write()'

def eval_exp(tree, env):
    exptype = tree[0]
    if exptype == 'call':
        fname = tree[1] # myfun in myfun(a, 3+4)
        fargs = tree[2] # [a, 3+4] in myfun(a, 3+4)
        fvalue = env_lookup(fname, env) # None for 'write'
        if fname == 'write':
            argval = eval_exp(fargs[0], env)
            output_sofar = env_lookup("javascript output", env)
            env_update('javascript output', output_sofar + str(argval), env)
