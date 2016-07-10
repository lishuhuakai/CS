# QUIZ -- Function Definitions
def eval_elt(tree, env):
    elttype = tree[0]
    if elttype == 'function':
        fname = tree[1]
        fparams = tree[2]
        fbody = tree[3]
        fvalue = ('function', fparams, fbody, env)
        add_to_env(env, fname, fvalue)
