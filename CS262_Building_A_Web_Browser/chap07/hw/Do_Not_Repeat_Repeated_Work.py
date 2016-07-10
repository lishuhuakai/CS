# Do Not Repeat Repeated Work
#
# Focus: Units 5 and 6: Interpreting and Optimization
#
#
# In class we studied many approaches to optimizing away redundant
# computation. For example, "X * 0" can be replaced with "0", because we
# know in advance that the result will always be 0. However, even if we do
# not know the answer in advance, we can sometimes save work. Consider this
# program fragment:
#
#       x = a + b + c;
#       y = 2;
#       z = a + b + c;
#
# Even though we do not know what "a + b + c" will be, there is no reason
# for us to compute it twice! We can replace the program with:
#
#       x = a + b + c;
#       y = 2;
#       z = x;          # works since "x = a + b + c;" above
#                       # and neither a nor b nor c has been changed since
#
# ... and always compute the same answer. This family of optimizations is
# sometimes called "common expression elimination" -- the subexpression
# "a+b+c" was common to two places in the code, so we eliminated it in one.
#
# In this problem we will only consider a special case of this
# optimization. If we see the assignment statement:
#
#       var1 = right_hand_side ;
#
# Then all subsequent assignment statements:
#
#       var2 = right_hand_side ;
#
# can be replaced with "var2 = var1 ;" provided that the "right_hand_side"s
# match exactly and provided that none of the variables involved in
# "right_hand_Side" have changed. For example, this program cannot be
# optimized in this way:
#
#       x = a + b + c;
#       b = 2;
#       z = a + b + c;
#
# Even though the right-hand-sides are exact matches, the value of b has
# changed in the interim so, to be safe, we have to recompute "a + b + c" and
# cannot replace "z = a + b + c" with "z = x".
#
# For this problem we will use the abstract syntax tree format from our
# JavaScript interpreter. Your procedure will be given a list of statements
# and should return an optimized list of statements (using the optimization
# above). However, you will *only* be given statement of the form:
#
#       ("assign", variable_name, rhs_expression)
#
# No other types of statements (e.g., "if-then" statements) will be passed
# to your procedure. Similarly, the rhs_expression will *only* contain
# expressions of these three (nested) form:
#
#       ("binop", exp, operator, exp)
#       ("number", number)
#       ("identifier", variable_name)
#
# No other types of expressions (e.g., function calls) will appear.
#
# Write a procedure "optimize()" that takes a list of statements (again,
# only assignment statements) as input and returns a new list of optimized
# statements that compute the same value but avoid recomputing
# whole right-hand-side expressions. (If there are multiple equivalent
# optimizations, return whichever you like.)
#
# Hint: x = y + z makes anything involving y and z un-available, and
# then makes y + z available (and stored in variable x).

def markchanged(ast, var):
    # 用来标记 某些变量已经改变了
    # 这些变量其实非常简单，都是一些
    # ast -> ("binop", ("identifier","a"), "+", ("identifier","b")
    # var -> ('identifier', 'a')
    if len(ast) == 4:
        if (ast[1] == var) or (ast[3] == var):
          return True
    return False

def optimize(ast):
    # 这个玩意的主要职责是判断两棵树是不是相等，好伐！
    equal = {}
    new_ast = [] # 新的语法树

    for asign in ast:
        flag = True
        for i in equal:
            if equal[i] == asign[2]: # 两棵语法树相等
                new_ast += [(asign[0], asign[1], ('identifier', i))]
                equal[asign[1]] = asign[2] # 将这个玩意也添加进去
                flag = False
                break
        if flag:
            new_ast += [asign]
            equal[asign[1]] = asign[2]
            new_equal = {}
            for e in equal:
                if not markchanged(equal[e], ('identifier', asign[1])):
                    new_equal[e] = equal[e]
            equal = new_equal
    return new_ast
        # write your answer here

# We have included some testing code to help you check your work. Since
# this is the final exam, you will definitely want to add your own tests.

example1 = [ \
("assign", "x", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
("assign", "y", ("number", 2)) ,
("assign", "z", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
]
answer1 = [ \
("assign", "x", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
("assign", "y", ("number", 2)) ,
("assign", "z", ("identifier", "x")) ,
]

print((optimize(example1)) == answer1)

example2 = [ \
("assign", "x", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
("assign", "a", ("number", 2)) ,
("assign", "z", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
] # 这里之所以不能改变的原因是因为 变量 a 的值已经发生改变了

print((optimize(example2)) == example2)

example3 = [ \
("assign", "x", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
("assign", "y", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
("assign", "x", ("number", 2)) ,
("assign", "z", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
]
answer3 = [ \
("assign", "x", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
("assign", "y", ("identifier", "x")) ,
("assign", "x", ("number", 2)) ,
("assign", "z", ("identifier", "y")) , # cannot be "= x"
]

print((optimize(example3)) == answer3)

example4 = [ \
("assign", "x", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
("assign", "y", ("binop", ("identifier","b"), "+", ("identifier","c"))) ,
("assign", "z", ("binop", ("identifier","c"), "+", ("identifier","d"))) ,
("assign", "b", ("binop", ("identifier","c"), "+", ("identifier","d"))) ,
("assign", "z", ("number", 5)) ,
("assign", "p", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
("assign", "q", ("binop", ("identifier","b"), "+", ("identifier","c"))) ,
("assign", "r", ("binop", ("identifier","c"), "+", ("identifier","d"))) ,
] # 这里又是由于什么样的原因啊！

answer4 = [ \
("assign", "x", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
("assign", "y", ("binop", ("identifier","b"), "+", ("identifier","c"))) ,
("assign", "z", ("binop", ("identifier","c"), "+", ("identifier","d"))) ,
("assign", "b", ("identifier", "z")) ,
("assign", "z", ("number", 5)) ,
    ("assign", "p", ("identifier","x")),
    ("assign", "q", ("identifier","y")),
#("assign", "p", ("binop", ("identifier","a"), "+", ("identifier","b"))) ,
#("assign", "q", ("binop", ("identifier","b"), "+", ("identifier","c"))) ,
("assign", "r", ("identifier", "b")) ,
]
print(optimize(example4))
print(optimize(example4) == answer4)
