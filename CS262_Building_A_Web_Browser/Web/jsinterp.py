# Wes Weimer
# -- STUDENTS WRITE this file 
#
# This is an interpreter for a simple subset of JavaScript.
#
# Students would write a version of this file as part of the course. 
#
# Approach: Basic Scheme-style Apply/Eval mutual recursion with an
# Environment. Because closures are possible, each Environment has a parent
# pointer, etc. 
#
# Environment is a tuple: 
#       (parent-pointer,        -- can be None for global environment
#       string->value)          -- maps identifiers to values
#
# We'll use the special value "javascript output" in the global environment to
# record strings written by JavaScript programs. 
#
# Tricky Bits:
#       Environments With Parent Pointers
#       Making Functions ("\lambda" = "function")
#       Reporting String Output from JavaScript's "write" function
#       Using an Exception to simulate imperative "return" stmts
#
# Hardest Bit: 
#       Calling Functions (new frame, eval args, bind args to formals)

class JSReturn(Exception):
        """Phrasing "return" as an exception allows us to break out of
        any nested statement evaluation and return to the caller
        immediately."""
        def __init__(self, retval):
                self.retval = retval

# Could also be done with
# raise Exception(retval)
# exception Exception as retval:

def env_lookup(vname,env): 
        if vname in env[1]:
                return (env[1])[vname]
        elif env[0] == None:
                return None
        else:
                return env_lookup(vname,env[0])

def env_update(vname,value,env):
        if vname in env[1]:
                (env[1])[vname] = value
        elif not (env[0] == None):
                env_update(vname,value,env[0])

def global_env_update(vname,value,env):
        if vname in env[1] or (env[0] == None):
                (env[1])[vname] = value
        elif not (env[0] == None):
                global_env_update(vname,value,env[0])

# Not necessary, but handy for debugging.
def env_debug(env):
        print("Environment Debug:")
        for vname in env[1]:
                print("  env[" + vname + "] = ",)
                print((env[1])[vname])

def eval_elt(elt,env):
        if elt[0] == 'function':
                fname = elt[1] 
                fparams = elt[2]
                fbody = elt[3]
                fvalue = ("function",fparams,fbody,env) 
                (env[1])[fname] = fvalue
        elif elt[0] == 'stmt':
                eval_stmt(elt[1],env) 
        else:
                print("ERROR: eval_elt: unknown element " + elt)

def eval_stmts(stmts,env): 
        for stmt in stmts:
                eval_stmt(stmt,env) 

def eval_stmt(stmt,env): 
        stype = stmt[0] 
        if stype == "if-then":
                cexp = stmt[1]
                then_branch = stmt[2] 
                if eval_exp(cexp,env):
                        eval_stmts(then_branch,env) 
        elif stype == "while":
                cexp = stmt[1]
                while_body = stmt[2] 
                while eval_exp(cexp,env):
                  eval_stmts(while_body,env) 
        elif stype == "if-then-else":
                cexp = stmt[1]
                then_branch = stmt[2] 
                else_branch = stmt[3] 
                if eval_exp(cexp,env):
                        eval_stmts(then_branch,env) 
                else:
                        eval_stmts(else_branch,env) 
        elif stype == "var": 
                vname = stmt[1]
                rhs = stmt[2]
                (env[1])[vname] = eval_exp(rhs,env)
                # env_update(vname, eval_exp(rhs,env), env) 
        elif stype == "assign": 
                vname = stmt[1]
                rhs = stmt[2]
                global_env_update(vname, eval_exp(rhs,env), env)
                #if vname in env:
                #        env_update(vname, eval_exp(rhs,env), env)
                #else:
                        # JavaScript allows you to make a global
                        # variable by assigning to it. 
                        # print "ERROR: unknown variable assigned " + vname
        elif stype == "return": 
                retval = eval_exp(stmt[1],env) 
                raise JSReturn(retval) 
        elif stype == "exp": 
                eval_exp(stmt[1],env) 
        else:
                print("ERROR: unknown statement type ",)
                print(stype)

def eval_exp(exp,env): 
        etype = exp[0] 
#        print "eval_exp : " ,
#        print exp 

        if etype == "identifier":
                vname = exp[1]
                value = env_lookup(vname,env) 
                if value == None: 
                        print("ERROR: unbound variable " + vname)
                else:
                        return value
        elif etype == "number":
                return float(exp[1])
        elif etype == "string":
                return exp[1] 
        elif etype == "true":
                return True
        elif etype == "false":
                return False
        elif etype == "not":
                return not(eval_exp(exp[1],env))
        elif etype == "function":
                fparams = exp[1]
                fbody = exp[2] 
                return ("function",fparams,fbody,env) 
        elif etype == "binop":
                a = eval_exp(exp[1],env)
                op = exp[2]
                b = eval_exp(exp[3],env)
                if op == "+":
                        return a+b
                elif op == "-":
                        return a-b
                elif op == "/":
                        return a/b
                elif op == "*":
                        return a*b
                elif op == "%":
                        return a%b
                elif op == "==":
                        return a==b
                elif op == "<=":
                        return a<=b
                elif op == "<":
                        return a<b
                elif op == ">=":
                        return a>=b
                elif op == ">":
                        return a>b
                elif op == "&&":
                        return a and b
                elif op == "||":
                        return a or b
                else:
                        print("ERROR: unknown binary operator ",)
                        print(op)
                        exit(1)
        elif etype == "call": 
                fname = exp[1]
                args = exp[2] 
                fvalue = env_lookup(fname,env) 
                if fname == "write":
                        argval = eval_exp(args[0],env)
                        output_sofar = env_lookup("javascript output",env)
                        env_update("javascript output",output_sofar + str(argval),env)
                elif fvalue[0] == "function":
                        fparams = fvalue[1] 
                        fbody = fvalue[2] 
                        fenv = fvalue[3] 
                        if len(fparams) != len(args):
                                print("ERROR: wrong number arguments to " + fname)
                        else: 
                                # make a new environment frame
                                newenv = (fenv,{ }) 
                                for i in range(len(args)):
                                        argval = eval_exp(args[i],env)
                                        (newenv[1])[fparams[i]] = argval
                                # evaluate the body in the new frame
                                try:
                                        eval_stmts(fbody,newenv) 
                                        return None
                                except JSReturn as r:
#                                        print "Return: ", r.retval
#                                        print r.retval
                                        return r.retval
                else:
                        print("ERROR: call to non-function " + fname)

                # Complicated! 
        else:
                print("ERROR: unknown expression type ",)
                print(etype)
        return None

def interpret(ast):
        global_env = (None, {"javascript output" : ""}) 
        for elt in ast:
                eval_elt(elt,global_env) 
#               env_debug(global_env)
        return (global_env[1])["javascript output"]
