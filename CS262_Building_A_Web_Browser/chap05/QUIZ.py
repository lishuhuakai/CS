# QUIZ
# Let's write an interpreter together.
def interpret(trees): # Hello, Friend
    for tree in trees: # Hello
        # ("word-element", "Hello,")
        nodetype = tree[0] # "word-element"
        if nodetype == "word-element":
            graphics.word(tree[1])
        elif nodetype = "tag-element":
            # <b>Strong text!</b>
            tagname = tree[1] # b
            tagargs = tree[2] # []
            subtrees = tree[3] # ...Strong text!...
            closetagname = tree[4] # b
            # QUIZ: (1) check that the tags match!
            # if not, use graphics.warning()
            # (2) interpret the subtree!
            if tagname != closetagname:
                graphics.warning("not equal")
            graphics.begintag(tagname, tagargs)
            interpret(subtrees)
            graphics.endtag()

# QUIZ
# Write an eval_exp procedure to interpret JavaScript arithemetic
# expression. Only handle +, - and numbers for now.
def eval_exp(tree):
    # ('number', '5')
    # ('binop', ...,'+', ...)
    nodetype = tree[0]
    if nodetype == "number":
        return int(tree[1])
    elif nodetype == 'binop':
        left_child = tree[1]
        operator = tree[2]
        right_child = tree[3]
        # QUIZ: (1) evaluate left and right child
        # (2) perform "operator"'s work
        left_value = eval_exp(left_child)
        right_value = eval_exp(right_child)
        if operator == '+':
            return left_value + right_value
        elif operator == '-':
            return left_value - right_value
        elif operator == '*':
            return left_value * right_value
        elif operator == '/':
            return left_value / right_value
# QUIZ
# Adding variable lookup to our interpreter!
def env_lookup(environment,):
    pass

def eval_exp(tree, environment):
    nodetype = tree[0]
    if nodetype == "number":
        return int(tree[1])
    elif nodetype == 'binop':
        left_child = tree[1]
        operator = tree[2]
        right_child = tree[3]
        left_value = eval_exp(left_child, environment)
        right_value = eval_exp(right_child, environment)
        if operator == '+':
            return left_value + right_value
        elif operator == '-':
            return left_value - right_value
        elif operator == '*':
            return left_value * right_value
        elif operator == '/':
            return left_value / right_value
    elif nodetype == 'identifier':
        # ('binop', ('identifier', 'x'), '+', ('number', 2)))
        #QUIZ: (1) find the identifier name
        # (2) look it up in the environment and return it
        variable_name = tree[1]
        return env_lookup(environment, variable_name)

# QUIZ
# Evaluating Statements
def eval_stmt(tree, environment):
    stmttype = tree[0]
    if stmttype == 'assign':
        # ('assign', 'x', ('number', '3'))
        variable_name = tree[1]
        right_child = tree[2]
        new_value = eval_exp(right_child, environment)
        env_update(environment, variable_name, new_value)
    elif stmttype == "if-then-else":
        conditional_exp = tree[1]
        then_stmt = tree[2]
        else_stmt = tree[3]
        # QUIZ: Complete this code
        # Assume 'eval_stmts(stmts, environment)' exists
        conditional_value = eval_exp(conditional_exp, environment)
        if conditional_value:
            eval_stmts(then_stmt, environment)
        else:
            eval_stmts(else_stmt, environment)


def env_lookup(var_name, environment):
    # env = (parent, dictionary)
    if var_name in environment[1]:
        return (environment[1])[var_name]
    elif environment[0] == None:
        return None
    else:
        return env_lookup(var_name, environment[0])
def env_update(vname, val, env):
    if vname in env[1]:
        (env[1])[vname] = val
    elif not (env[0] == None):
        env_update(vname, val, env)

# QUIZ
# Return will Throw an Exception
# Function Calls: new environments, catch return value
def eval_stmt(tree, environment):
    stmttype = tree[0]
    if stmttype == 'call': # ('call', 'sqrt', [('number', 2)])
        fname = tree[1] # 'sqrt'
        args = tree[2] # [('number', '2')]
        fvalue = env_lookup(fname, environment)
        if fvalue[0] == 'function':
            # We'll make a promise to ourselves:
            # ('function', params, body, env)
            if len(parmas) != len(args):
                print("Error: wrong number of args")
            else:
                # Make a new environment frame
                # QUIZ (evaluate actual args, ...)
                newenv = (fenv, {})
                for i in range(len(args)):
                    argval = eval_exp(args[i], environment)
                    (newenv[1])(fparams[1]) = argval # add to env
                # evaluate the body in the new frame
                try:
                    #QUIZ: eval the body
                    eval_stmts(fbody, newenv)
                    return None
                except Exception as return_val:
                    return return_val

