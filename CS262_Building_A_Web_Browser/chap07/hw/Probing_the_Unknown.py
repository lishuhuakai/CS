# Probing the Unknown
#
# Focus: Units 5 and 6: Interpreting and Debugging
#
#
# In software engineering it is common to "break" software when changing
# it. This often results in an "old, good" version of the program and a
# "new, buggy" version of the program. In this question you have two
# versions of a JavaScript interpreter (the same one we wrote in class, in
# fact!) -- one of which is buggy. You need to find the best description
# of the bug that you can. However, for the purposes of this problem, you
# cannot see either version of the interpreter code -- you can only run
# them on inputs (i.e., probe them) and compare the results.
#
# Pass JavaScript programs (using the simple subset of JavaScript we handle
# in class) to probe(). Programs that cause the good interpreter and the
# buggy interpreter to return different results are illuminating. Try to
# find as many of them as you can until you are certain about what the bug
# is.
#
# Once you are certain of the bug, uncomment _exactly one_ of the "bug =
# ..." lines and submit.

import ply.lex as lex
import ply.yacc as yacc
from Web import jstokens
from Web import jsgrammar
from Web import jsinterpgood
from Web import jsinterpbuggy

jslexer = lex.lex(module=jstokens)
jsparser = yacc.yacc(module=jsgrammar,tabmodule="parsetabjs")

# Use this function to try to determine the bug.
def probe(jstext):

        jsast = jsparser.parse(jstext,lexer=jslexer)
        try:    good_result = jsinterpgood.interpret(jsast)
        except: good_result = "error!"
        try:    buggy_result = jsinterpbuggy.interpret(jsast)
        except: buggy_result = "error!"

        print(jstext)
        if good_result == buggy_result: # not interesting
                print("\tgood = buggy = ", good_result)
        else: # very interesting!
                print("\tgood  = ", good_result)
                print("\tbuggy = ", buggy_result)

# This probe JavaScript string is intentionally long, but it does cause the
# good interpreter and the buggy interpreter to produce incorrect output.
# One way to approach this problem is to simplify the probe JavaScript
# to test various hypotheses you have about what the bug could be.
probe("""var a = 1;
var x = 2;
var y = 2;
function myfun(x) {
        var a = 3;
        x = x + y;
        y = x + y;
        var p = function(y,z) {
                var q = function(x,z) {
                        return x+a*y/z;
                } ;
                return q;
        } ;
        while (x < y && (x < 10)) {
                if (! (x < y)) {
                        x = x - 1;
                } else {
                        x = x + 1;
                }
                a = a + 1;
        }
        return p(a,y);
}
var f = myfun(y);
write( f(6,7) ) ;
""" )


# Uncomment _exactly one_ "bug = X" line below. If multiple explanations
# seem to fit, use probes to narrow it down.

# If you think the bug is that if-then-else conditionals are mistakently
# swapped, so that the else branch is evaluated when the then branch should
# be, and vice-versa, uncomment "bug = 1".
# bug = 1

# If you think the bug is that while loop bodies are evaluated the wrong
# number of times, uncomment "bug = 2".
# bug = 2

# If you think the bug is that return statements do not cause methods to
# exit immediately, uncomment "bug = 3".
# bug = 3

# If you think the bug is that "not" expressions are not evaluated
# correctly, uncomment "bug = 4".
# bug = 4

# If you think the bug is that "&&" and/or "||" expressions are not
# evaluated correctly, uncomment "bug = 5".
# bug = 5

# If you think the bug is that "var" statements do not correctly assign
# values to local variables, uncomment "bug = 6".
# bug = 6

# If you think the bug is that "function" call expressions do not
# evaluate their bodies in the correct environment, uncomment "bug =
# 7".
# bug = 7

# If you think the bug is that "function" call expressions evaluate their
# arguments in the wrong order, uncomment "bug = 8".
# bug = 8

# Remember, you should uncomment _exactly one_ "bug = X" line.
