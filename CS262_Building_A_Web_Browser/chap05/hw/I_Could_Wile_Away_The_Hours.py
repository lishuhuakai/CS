# "I Could Wile Away The Hours"
#
#
# Although our HTML and JavaScript interpreters are not yet integrated into
# a single browser, we can still extend our JavaScript interpreter
# independently. We already have support for recursive functions and "if"
# statements, but it would be nice to add support for "while".
#
# Consider the following two JavaScript fragments:
#
#    var i = 0;
#    while (i <= 5) {
#      document.write(i);
#      i = i + 2;
#    };
#
# And:
#
#    function myloop(i) {
#      if (i <= 5) {
#         document.write(i);
#         myloop(i + 2);
#      } ;
#    }
#    myloop(0);
#
# They both have the same effect: they both write 0, 2 and 4 to the
# webpage. (In fact, while loops and recursion are equally powerful! You
# really only need one in your language, but it is very convenient to have
# them both.)
#
# We can extend our lexer to recognize 'while' as a keyword. We can extend
# our parser with a new statement rule like this:
#
#    def p_stmt_while(p):
#        'stmt : WHILE exp compoundstmt'
#         p[0] = ("while",p[2],p[3])
#
# Now we just need to extend our interpreter to handle while loops. The
# meaning of a while loop is:
#
#       1. First, evaluate the conditional expression in the current
#       environment. If it evaluates to false, stop.
#
#       2. Evaluate the body statements in the current environment.
#
#       3. Go to step 1.
#
# Recall that our JavaScript interpreter might have functions like:
#
#       eval_stmts(stmts,env)
#       eval_stmt(stmt,env)
#       eval_exp(exp,env)
#
# For this assignment, you should write a procedure:
#
#       eval_while(while_stmt,evn)
#
# Your procedure can (and should!) call those other procedures. Here is
# how our interpreter will call your new eval_while():
#
# def eval_stmt(stmt,env):
#         stype = stmt[0]
#         if stype == "if-then":
#                 cexp = stmt[1]
#                 then_branch = stmt[2]
#                 if eval_exp(cexp,env):
#                         eval_stmts(then_branch,env)
#         elif stype == "while":
#                 eval_while(stmt,env)
#         elif stype == "if-then-else":
#                 ...
#
# Hint 1: We have structured this problem so that it is difficult for you
# to test (e.g., because we have not provided you the entire JavaScript
# interpreter framework). Thus, you should think carefully about how to
# write the code correctly. Part of the puzzle of this exercise is to
# reason to the correct answer without "guess and check" testing.
#
# Hint 2: It is totally legal to define JavaScript's while using a Python
# while statement. (Remember, an interpreter is like a translator.) You
# could also define JavaScript's while using recursion in Python.
#
# Hint 3: Extract the conditional expression and while loop body statements
# from while_stmt first.

def eval_while(while_stmt, env):
    # while_stmt : ('while', exp, compoundstmt)
    conditional_exp = while_stmt[1]
    compound_stmt = while_stmt[2]
    while (eval_exp(conditional_exp, env)):
        eval_stmts(compound_stmt, env)


