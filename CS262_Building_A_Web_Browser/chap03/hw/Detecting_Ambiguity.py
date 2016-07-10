# Detecting Ambiguity
#
# A grammar is ambiguous if there exists a string in the language of that
# grammar that has two (or more) parse trees. Equivalently, a grammar is
# ambiguous if there are two (or more) different sequences of rewrite rules
# that arrive at the same final string.
#
# Ambiguity is a critical concept in natural languages and in programming
# languages. If we are not careful, our formal grammars for languages like
# JavaScript will have ambiguity.
#
# In this problem you will write a procedure isambig(grammar,start,tokens)
# that takes as input a grammar with a finite number of possible
# derivations and a string and returns True (the value True, not the string
# "True") if those tokens demonstrate that the grammar is ambiguous
# starting from that start symbol (i.e., because two different sequences of
# rewrite rules can arrive at those tokens).
#
# For example:
#
# grammar1 = [                  # Rule Number
#       ("S", [ "P", ] ),       # 0
#       ("S", [ "a", "Q", ]) ,  # 1
#       ("P", [ "a", "T"]),     # 2
#       ("P", [ "c" ]),         # 3
#       ("Q", [ "b" ]),         # 4
#       ("T", [ "b" ]),         # 5
#       ]
#
# In this grammar, the tokens ["a", "b"] do demonstrate that the
# grammar is ambiguous because there are two difference sequences of
# rewrite rules to obtain them:
#
#       S  --0->  P  --2->  a T  --5->  a b
#
#       S  --1->  a Q  --4->  a b
#
# (I have written the number of the rule used inside the arrow for
# clarity.) The two sequences are [0,2,5] and [1,4].
#
# However, the tokens ["c"] do _not_ demonstrate that the grammar is
# ambiguous, because there is only one derivation for it:
#
#       S  --0->  P  --3->  c
#
# So even though the grammar is ambiguous, the tokens ["c"] do not
# demonstrate that: there is only one sequence [0,3].
#
# Important Assumption: In this problem the grammar given to you will
# always have a finite number of possible derivations. So only a
# finite set of strings will be in the language of the grammar. (You could
# test this with something like cfginfinite, so we'll just assume it.)
#
# Hint 1: Consider something like "expand" from the end of the Unit, but
# instead of just enumerating utterances, enumerate (utterance,derivation)
# pairs. For a derivation, you might use a list of the rule indexes as we
# did in the example above.
#
# Hint 2: Because the grammar has only a finite number of derivations, you
# can just keep enumerating new (utterance,derivation) pairs until you
# cannot find any that are not already enumerated.

def expand(tokens_and_derivation, grammar):
    (tokens, derivation) = tokens_and_derivation
    for token_pos in range(len(tokens)): # for each token
        for rule_index in range(len(grammar)): # for each rule
            rule = grammar[rule_index]
            if tokens[token_pos] == rule[0]:
                yield ((tokens[0:token_pos] + rule[1] + tokens[token_pos+1:]), derivation + [rule_index])

def isambigEx(grammar, start, utterance):
    enumerated = [([start],[])]
    while True:
        # ENUMERATE
        new_enumerated = enumerated

        for u in enumerated:
            for i in expand(u, grammar):
                if not i in new_enumerated:
                    new_enumerated = new_enumerated + [i]

        if new_enumerated != enumerated:
            # Found something new keep going!
            enumerated = new_enumerated
        else:
            break
    return len([x for x in enumerated if x[0] == utterance]) > 1

# 这里有一个重要的假设，那就是，这种文法产生的语句是有限的，所以不用担心无限递归的问题
def isambig(grammar, start, utterance):
    """我觉得需要优化，那就是添加一个长度的信息，如果长度大于该值，直接掐掉"""
    ulen = len(utterance)
    # 这里面应该构成了闭包吧！
    def helper(tokens):
        if len(tokens) > ulen:
            return None
        for pos in range(len(tokens)):
            for rule in grammar:
                if (tokens[pos] == rule[0]):
                    yield tokens[0:pos] + rule[1] + tokens[pos+1:]
    tmp = [[start]]
    all_sentences = tmp
    while len(tmp) != 0 :
        new_tmp = []
        for sentence in tmp:
            new_tmp += [i for i in helper(sentence)]
        tmp = new_tmp
        all_sentences += new_tmp
        if all_sentences.count(utterance) > 1:
            return True
    return False

# We have provided a few test cases. You will likely want to add your own.

grammar1 = [
       ("S", [ "P", ]),
       ("S", [ "a", "Q", ]) ,
       ("P", [ "a", "T"]),
       ("P", [ "c" ]),
       ("Q", [ "b" ]),
       ("T", [ "b" ]),
       ]
print(isambig(grammar1, "S", ["a", "b"]) == True)
print(isambig(grammar1, "S", ["c"]) == False)

grammar2 = [
       ("A", [ "B", ]),
       ("B", [ "C", ]),
       ("C", [ "D", ]),
       ("D", [ "E", ]),
       ("E", [ "F", ]),
       ("E", [ "G", ]),
       ("E", [ "x", "H", ]),
       ("F", [ "x", "H"]),
       ("G", [ "x", "H"]),
       ("H", [ "y", ]),
       ]
print(isambig(grammar2, "A", ["x", "y"]) == True)
print(isambig(grammar2, "E", ["y"]) == False)

grammar3 = [ # Rivers in Kenya
       ("A", [ "B", "C"]),
       ("A", [ "D", ]),
       ("B", [ "Dawa", ]),
       ("C", [ "Gucha", ]),
       ("D", [ "B", "Gucha"]),
       ("A", [ "E", "Mbagathi"]),
       ("A", [ "F", "Nairobi"]),
       ("E", [ "Tsavo" ]),
       ("F", [ "Dawa", "Gucha" ])
       ]
print(isambig(grammar3, "A", ["Dawa", "Gucha"]) == True)
print(isambig(grammar3, "A", ["Dawa", "Gucha", "Nairobi"]) == False)
print(isambig(grammar3, "A", ["Tsavo"]) == False)
