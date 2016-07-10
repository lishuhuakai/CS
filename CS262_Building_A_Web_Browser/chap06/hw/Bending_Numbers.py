# Bending Numbers
#
# In class we discussed a number of arithmetic optimizations for
# JavaScript. In our approach to optimization, a sub-tree of the
# abstract syntax is replaced with a new abstract syntax tree.
#
# In addition to using arithmetic identities, such as X*0 == 0 for all X,
# we can also perform arithmetic operations on constants. For example, if
# a JavaScript loop or recursive procedure containts 1+2+3, we can just
# evaluate it to 6 once and then not perform the two additions again.
# This technique is called "constant folding".
#
# Write a procedure optimize(exp) that takes a JavaScript expression AST
# node and returns a new, simplified JavaScript expression AST. You must
# handle:
#
#       X * 1 == 1 * X == X     for all X
#       X * 0 == 0 * X == 0     for all X
#       X + 0 == 0 + X == X     for all X
#       X - X == 0              for all X
#
#       and constant folding for +, - and * (e.g., replace 1+2 with 3)
#
# To do constant folding, given a parse tree for X+Y we want to try to add
# the values for the parse trees of X and Y. If X and Y are both numbers,
# that will work. But if X or Y is an identifier, for example, that will
# not work, because the types will not match.

def optimize(exp):
    etype = exp[0]
    if etype == "binop":
        a = optimize(exp[1])
        op = exp[2]
        b = optimize(exp[3])

        # Try Arithmetic Laws
        if op == "*" and (a == ("number",0) or b == ("number",0)):
            return ("number",0)
        # Fill in more optimizations here ...
        elif op == '*' and a == one:
            return b
        elif op == '*' and b == one:
            return a
        elif op == '+' and a == zero:
            return b
        elif op == '+' and b == zero:
            return a
        elif op == '-' and a == b:
            return zero
        elif a[0] == 'number' and b[0] == 'number':
            if op == '+':
                return ('number', a[1] + b[1])
            elif op == '-':
                return ('number', a[1] - b[1])
            elif op == '*':
                return ('number', a[1] * b[1])
        else:
            return ('binop', a, op, b)
        # Try Constant Folding
        # Fill in more optimizations here ...

        # If all else fails, return something good here ...

    # leave this expression un-optimized
    return exp

# We have prepared some test cases. You may want to try your own.
zero            = ("number", 0.0)
one             = ("number", 1.0)
two             = ("number", 2.0)
xerxes          = ("var","xerxes") # Kings and Queens of Persia and Macedonia
darius          = ("var","darius")
antiochus       = ("var","antiochus")
musa            = ("var","musa")
def plus(a,b):
        return ("binop",a,"+",b)
def minus(a,b):
        return ("binop",a,"-",b)
def times(a,b):
        return ("binop",a,"*",b)

exp1 = times(two,zero)
print(optimize(exp1) == zero)

exp2 = times(darius,minus(two,two))
print(optimize(exp2) == zero)

exp3 = minus(plus(zero,plus(one,plus(two,zero))),two)
#  ((0 + (1 + (2 + 0))) - 2)
print(optimize(exp3) == one)

five = plus(two,plus(two,one))
exp4 = times(five,(plus(minus(musa,musa),plus(musa,zero))))
# 5 * ((musa - musa) + (musa + 0))
print(optimize(exp4))
print(optimize(exp4) == ('binop', ('number', 5.0), '*', ('var', 'musa')))

big_exp = zero
for i in range(10):
        big_exp = ("binop",big_exp,"+",("number",i))
print(optimize(big_exp) == ("number", 45.0)) # 0+1+2+3+4+5+6+7+8+9
