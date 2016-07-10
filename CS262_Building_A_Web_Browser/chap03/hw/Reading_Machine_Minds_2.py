# Reading Machine Minds 2
#
# We say that a finite state machine is "empty" if it accepts no strings.
# Similarly, we say that a context-free grammar is "empty" if it accepts no
# strings. In this problem, you will write a Python procedure to determine
# if a context-free grammar is empty.
#
# A context-free grammar is "empty" starting from a non-terminal symbol S
# if there is no _finite_ sequence of rewrites starting from S that
# yield a sequence of terminals.
#
# For example, the following grammar is empty:
#
# grammar1 = [
#       ("S", [ "P", "a" ] ),           # S -> P a
#       ("P", [ "S" ]) ,                # P -> S
#       ]
#
# Because although you can write S -> P a -> S a -> P a a -> ... that
# process never stops: there are no finite strings in the language of that
# grammar.
#
# By contrast, this grammar is not empty:
#
# grammar2 = [
#       ("S", ["P", "a" ]),             # S -> P a
#       ("S", ["Q", "b" ]),             # S -> Q b
#       ("P", ["P"]),                   # P -> P
#       ("Q", ["c", "d"]),              # Q -> c d
#
# And ["c","d","b"] is a witness that demonstrates that it accepts a
# string.
#
# Write a procedure cfgempty(grammar,symbol,visited) that takes as input a
# grammar (encoded in Python) and a start symbol (a string). If the grammar
# is empty, it must return None (not the string "None", the value None). If
# the grammar is not empty, it must return a list of terminals
# corresponding to a string in the language of the grammar. (There may be
# many such strings: you can return any one you like.)
#
# To avoid infinite loops, you should use the argument 'visited' (a list)
# to keep track of non-terminals you have already explored.
# visited这个字段能够干嘛，可以记录我已经探索过的非终结符，是吧！
#
# Hint 1: Conceptually, in grammar2 above, starting at S is not-empty with
# witness [X,"a"] if P is non-empty with witness X and is non-empty with
# witness [Y,"b"] if Q is non-empty with witness Y.
#
# Hint 2: Recursion! A reasonable base case is that if your current
# symbol is a terminal (i.e., has no rewrite rules in the grammar), then
# it is non-empty with itself as a witness.
#
# Hint 3: all([True,False,True]) = False
#         any([True,True,False]) = True

def isSentences(grammer, symbols):
    """
    这个函数主要是用来判断symbols是否为一个句子
    """
    for each_symbol in symbols:
        for each_rule in grammer:
            if each_rule[0] == each_symbol:
                return False
    return True
# elegant code
def cfgempty(grammar, symbol, visited):
    """
    非常漂亮的代码！
    """
    if symbol in visited: # 首先要求这个symbol不能是已经被访问过的，为啥？
    # no infinite loops
        return None
    elif not any([rule[0] == symbol for rule in grammar]): # 前面的两种情况，是递归的终止条件吧！
        return [symbol]  # 这里表示symbol是终结符
    else: # 这里表示symbol之前没有被使用过，并且symbol是非终结符
        new_visited = visited + [symbol]
        # 这里表示，我们要使用这个symbol进行推导了，从这个symbol出发，推导出子表达式
        # 后面的代码里递归，也就是说，如果字表达式里有一个symbol又可以推导出这个父symbol的话，这就构成了loop
        # 这样玩下去是没有出路的
        # conside every rewrite rule "Symbol => RHS"
        for rhs in [r[1] for r in grammar if r[0] == symbol]: # 如果symbol可以被替换掉
            # check if every part of RHS is non-empty
            if all([None != cfgempty(grammar, r, new_visited) for r in rhs]): # 也就是rhs的每一个元素都是终结符
                result = [] # gather up result
                for r in rhs:
                    result = result + cfgempty(grammar, r, new_visited)
                return result
    return None


# We have provided a few test cases for you. You will likely want to add
# more of your own.

grammar1 = [
      ("S", [ "P", "a" ] ),
      ("P", [ "S" ]) ,
      ]

print(cfgempty(grammar1,"S",[]) == None)

grammar2 = [
      ("S", ["P", "a" ]),
      ("S", ["Q", "b" ]),
      ("P", ["P"]),
      ("Q", ["c", "d"]),
      ]

print(cfgempty(grammar2,"S",[]) == ['c', 'd', 'b'])


grammar3 = [  # some Spanish provinces
        ("S", [ "Barcelona", "P", "Huelva"]),
        ("S", [ "Q" ]),
        ("Q", [ "S" ]),
        ("P", [ "Las Palmas", "R", "Madrid"]),
        ("P", [ "T" ]),
        ("T", [ "T", "Toledo" ]),
        ("R", [ ]) ,
        ("R", [ "R"]),
        ]

print(cfgempty(grammar3,"S",[]) == ['Barcelona', 'Las Palmas', 'Madrid', 'Huelva'])
