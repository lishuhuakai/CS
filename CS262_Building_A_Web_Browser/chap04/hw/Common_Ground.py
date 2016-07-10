# Common Ground
#
# Consider the strings "Tapachula" and "Temapache", both of which name
# towns in the country of Mexico. They share a number of substrings in
# common. For exmaple, "T" and "pa" can be found in both. The longest
# common unbroken substring that they both share is "apach" -- it starts
# at position 1 in "Tapachula" and at position 3 in "Temapache". Note that
# "Tapach" is not a substring common to both: even though "Temapache"
# contains both "T" and "apach", it does not contain the unbroken substring
# "Tapach".
#
# Finding the longest common substring of two strings is an important
# problem in computer science. It is a building block to related problems,
# such as "sequence alignment" or "greatest common subsequence" that are
# used in problem domains as diverse as bioinformatics and making spelling
# checkers.
#
# We will use memoization, recursion and list comprehensions to tackle this
# problem. In particular, we will build up a chart that stores computed
# partial answers.
#
# To make the problem a bit easier, we'll start by computing the longest
# common *suffix* of two strings. For example, "Tapachula" and "Temapache"
# have no common suffix -- or a common suffix of length 0, if you prefer.
# By contrast, "Tapach" and "Temapach" have a longest common suffix of
# length 5 ("apach"). One way to reason to that is that their last letters
# match (an "h" in both cases) -- and if you peel those last letters off
# the smaller problem of "Temapac" vs. "Tapac" has a common suffix of size
# four.
#
# Write a memoized procedure csuffix(X,Y) that returns the size of the
# longest common suffix of two strings X and Y. This is 0 if X and Y have
# different final letters. Otherwise, it is 1 plus csuffix() of X and Y,
# each with that common last letter removed. Use a global dictionary chart
# that maps (X,Y) pairs to their longest common suffix values to avoid
# recomputing work. If either X or Y is the empty string, csuffix(X,Y) must
# return 0.
#
# Once we have csuffix(), we can find the largest common substring of X and
# Y by computing the csuffix() of all possible combinations of *prefixes*
# of X and Y and returning the best one. Write a procedure prefixes(X) that
# returns a list of all non-empty strings that are prefixes of X. For
# example, prefixes("cat") == [ "c", "ca", "cat" ].
#
# Finally, write a procedure lsubstring(X,Y) that returns the length of the
# longest common substring of X and Y by considering all prefixes x of X
# and all prefixes y of Y and returning the biggest csuffix(x,y)
# encountered.

# Hint 1. Reminder: "hello"[-1:] == "o"
#
# Hint 2. Reminder: "goodbye"[:-1] == "goodby"
#
# Hint 3. prefixes(X) can be written in one line with list comprehensions.
# Consider "for i in range(len(X))".
#
# Hint 4. max([5,-2,8,3]) == 8
#
# Hint 5. You can "comprehend" two lists at once!
#       [ (a,b) for a in [1,2] for b in ['x','y'] ]
#               == [(1, 'x'), (1, 'y'), (2, 'x'), (2, 'y')]
# This is by no means necessary, but it is convenient.

chart = { } # global value, used for memoriziation
# chart is a dict, It is very converinet to memeorize something
def csuffix(X,Y):
    if len(X) == 0 or len(Y) == 0:
        return 0
    elif (X,Y) in chart:
        return chart[(X,Y)]
    elif X[-1] == Y[-1]:
        result = 1 + csuffix(X[:-1], Y[:-1])
        chart[(X, Y)] = result # remeber it
        return result
    else:
        chart[(X, Y)] = 0
        return 0
    # Fill in your code here.

def prefixes(X):
    return [X[:i + 1] for i in range(len(X))]
    # Fill in your code here (can be done in one or two lines).

def lsubstring(X,Y):
    return max([csuffix(x, y) for x in prefixes(X) for y in prefixes(Y)])
    # Fill in your code here (can be done in one or two lines).

# We have included some test cases. You will likely want to write your own.
#print(prefixes("cat"))
#print(csuffix("aaa", "baa"))
#print(lsubstring("aaa", "baa"))

print(lsubstring("Tapachula", "Temapache") == 5)  # Mexico, "apach"
print(chart[("Tapach","Temapach")] == 5)
print(lsubstring("Harare", "Mutare") == 3)        # Zimbabwe, "are"
print(chart[("Harare","Mutare")] == 3)
print(lsubstring("Iqaluit", "Whitehorse") == 2)   # Canada, "it"
print(chart[("Iqaluit","Whit")] == 2)
print(lsubstring("Prey Veng", "Svay Rieng") == 3) # Cambodia, "eng"
print(chart[("Prey Ven","Svay Rien")] == 2)
print(chart[("Prey Veng","Svay Rieng")] == 3)
print(lsubstring("Aceh", "Jambi") == 0)           # Sumatra, ""
print(chart[("Aceh", "Jambi")] == 0)
