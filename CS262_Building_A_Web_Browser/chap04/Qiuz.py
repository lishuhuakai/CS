# 现在的话，我来看一下代码！
def addtochart(theset, index, elt): # 将状态加入chart
    """

    :param theset: chart
    :param index:
    :param elt: elements
    :return: boolean
    """
    if not (elt in theset[index]): # elt表示元素elements，如果在theset的index项中已经包含了这个elt，不必再加入了
        theset[index] = [elt] + theset[index] # 不包含的话，自然要加入,加入的位置很有趣，这里是加入在最前方
        return True
    else:
        return False

"""
a = [x*x for x in (1,2,3,4,5)]
b = [x*x for x in (1,2,3,4,5) if x % 2 == 1]
print(a)
print(b)
"""

grammar = [
    ("S", ["P"]),             # S -> P
    ("P", ["(", "P", ")"]),   # P -> ( P )
    ("P", []),                # p ->
]

tokens = ["(", "(", ")", ")"]


# x -> a b . c d from j
# we can represent the state as follows:
# state = ("x", ["a", "b"], ["c", "d"], j)


def closure(grammar, i , x, ab, cd, j):
    """

    :param grammar: 语法
    :param i: index
    :param x:
    :param ab: 已经匹配过的elements
    :param cd: 尚未匹配的elements
    :param j: chart表的索引
    :return: tuple list
    """
    return [
        (rule[0], [], rule[1], i)
        for rule in grammar if cd != [] and rule[0] == cd[0] # 这里要求c是非终结符
    ]

def shift(tokens, i, x, ab, cd, j):
    """

    :param tokens: 输入的字符
    :param i: index,指示chart表的第index项
    :param x:
    :param ab:
    :param cd:
    :param j:
    :return: tuple
    """
    # x -> ab●cd from i  tokens[i] == c?
    if cd != [] and tokens[i] == cd[0]: # 这里要求c是终结符，并且c和第i个输入字符相匹配
        return (x, ab + [cd[0]], cd[1:], j) # 移动一个元素
    else:
        return None

def reductions(chart, i, x, ab, cd, j): # 规约
    """

    :param chart: chart表
    :param i: index,表示我们已经处理到了chart表的第i项啦!
    :param x: 要规约到的非终结符
    :param ab:
    :param cd:
    :param j: 从chart表的第item项规约而来
    :return: tuple list
    """
    # x -> ab● from j
    # char[j] has y -> ...●x... from k
    return [
        (jstate[0], jstate[1]+[x], jstate[2][1:], jstate[3])
        for jstate in chart[j]
        if cd == [] and jstate[2] != [] and (jstate[2])[0] == x
    ]

def parse(tokens, grammar): # 感觉这个算法简约而不简单
    tokens = tokens + ["end_of_input_marker"]
    chart = {}
    start_rule = grammar[0] # 起始的规则
    for i in range(len(tokens)+1): # chart表只会有这么大
        chart[i] = []
    start_state = (start_rule[0], [], start_rule[1], 0)
    chart[0] = [start_state]
    for i in range(len(tokens)): # 假设tokens有k个token，那么就处理7次即可
        while True:
            changes = False
            for state in chart[i]:
                # state === x -> a b ● c d, j
                x = state[0]
                ab = state[1]
                cd = state[2]
                j = state[3]
                # Current State == x -> a b ● c d, j
                # Option 1: For each grammar rule c -> p q r
                # (where the c's match)
                # make a next state     c -> ● p q r, i
                # English: We're about to start parsing a "c", but
                # "c" may be something like "exp" with its own
                # production rules. We'll bring those production rules in.
                # addtochart 返回的是bool值，如果加入成功，那么，返回True，否则返回false
                next_states = closure(grammar, i, x, ab, cd, j)
                for next_state in next_states: # 首先计算闭包
                    changes = addtochart(chart, i, next_state) or changes

                # Current State == x -> a b ● c d, j
                # Option 2: If tokens[i] == c,
                # make a next state  x -> a b c ● d, j
                # in chart[i+1]
                # English: We're looking for to parse token c next
                # and the current token is exactly c! Aren't we lucky!
                # So we can parse over it and move to j+1.

                next_state = shift(tokens, i, x, ab, cd, j) # 然后移进
                if next_state != None:
                    changes = addtochart(chart, i + 1, next_state) or changes

                # Current State == x -> a b ● c d , j
                # Option 3: If cd is [], the state is just x -> a b ● , j
                # for each p -> q ● x r, l in chart[j]
                # make a new state  p -> q x ● r, l
                # in chart[i]
                # English: We just finished parsing an "x" with this token,
                # but that may have been a sub-step (like matching "exp -> 2"
                # in "2+3"). We should update the higher-level rules as well.
                next_states = reductions(chart, i, x, ab, cd, j) # 然后规约
                for next_state in next_states:
                    changes = addtochart(chart, i, next_state) or changes

            # We're done if nothing changed!
            if not changes:
                break

    for i in range(len(tokens)): # print out the chart
        print("==> chart " + str(i))
        for state in chart[i]:
            x = state[0]
            ab = state[1]
            cd = state[2]
            j = state[3]
            print("   " + x + " ->", end='')
            for sym in ab:
                print(" " + sym, end='')
            print(" ●", end='')
            for sym in cd:
                print(" " + sym, end='')
            print(" from " + str(j))

    accepting_state = (start_rule[0], start_rule[1], [], 0)
    return accepting_state in chart[len(tokens) - 1]

print(parse(tokens, grammar))




