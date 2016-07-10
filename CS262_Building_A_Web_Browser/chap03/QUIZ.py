# Wirite a python function called small_words that accept a
# list of strings as input and yields those that art at most 4 letters long

def at_most_three(strings):
    for each_string in strings:
        if len(each_string) <= 3:
            yield each_string

print([x for x in at_most_three(["2", "3", "dsdf"])])

# Grammer
grammer = [
    ('exp', ['exp', '+', 'exp']),
    ('exp', ['exp', '-', 'exp']),
    ('exp', ['(', 'exp', ')']),
    ('exp', ['num'])
]
# "a exp" -> "a exp + exp"
# "a exp - exp"
# "a ( exp )"
# "a num"

def expand(tokens, grammer):
    for pos in range(len(tokens)):
        for rule in grammer:
            if rule[0] == tokens[pos]:
                yield tokens[0:pos] + rule[1] + tokens[pos+1:] # 替换掉一个非终结符，就会产生一条新的语句



depth = 2
utterances = [['exp']]
for x in range(depth):
    for sentence in utterances:
        utterances = utterances + \
            [i for i in expand(sentence, grammer)]

for sentence in utterances:
    print(sentence)