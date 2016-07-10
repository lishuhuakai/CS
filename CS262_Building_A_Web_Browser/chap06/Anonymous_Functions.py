# QUIZ
# Let's add support for anonymous functions
# (= function expression) to our JS interpreter
def eval_exp(tree, env):
    exptype = tree[0]
    # function(x, y) { return x + y; }
    if exptype == "function":
        # ('function', ['x', 'y'], [('return', ('binop', ...)])
        fparams = tree[1]
        fbody = tree[2]
        return ('function', fparams, fbody, env)
        # 'env' allows local functions to see local variables
        # can see variables that were in scope *when the function was defined*


