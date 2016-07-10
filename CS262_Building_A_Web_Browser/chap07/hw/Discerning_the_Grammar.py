# Discerning the Grammar
# Challenge Problem
#
# Focus: Units 3 and 4: Parsing and Memoization
#
#
# In this problem you are provided with the final chart that results from
# applying the parsing algorithm from class (i.e., Earley's algorithm)
# to a particular string of tokens.
#
# Your task is to reverse-engineering the simplest grammar that would yield
# exactly the chart given.
#
# You should encode your answer using the grammar format from class. Assign
# your answer to a variable named "grammar".
#
# For example, if you think the answer is
#
#       S -> T
#       T -> p Q r
#       T -> p Q s
#       Q -> q
#
# Then you would write
#
#   grammar = [
#     ("S", ["T"]),
#     ("T", ["p","Q","r"]),
#     ("T", ["p","Q","s"]),
#     ("Q", ["q"]),
#   ]
#
# The actual input string of tokens is:
#
# ["id", "(", "(", "id", ")", ",", "id", ")"]
#
# That string was in the language of the grammar (i.e., the parse succeeded).
#
# The actual resulting chart was:
#       (Hint: although this output looks long, a correct answer from you
#       will only use about a dozen lines.)

# == chart[0]
#     E ->  .  (  E  )            from 0
#     E ->  .  E  -  E            from 0
#     E ->  .  E  +  E            from 0
#     E ->  .  id                 from 0
#     E ->  .  id  (  A  )        from 0
#     S ->  .  E                  from 0

# == chart[1]
#     E ->  E  .  -  E            from 0
#     E ->  E  .  +  E            from 0
#     E ->  id  .                 from 0
#     E ->  id  .  (  A  )        from 0
#     S ->  E  .                  from 0

# == chart[2]
#     A ->  .                     from 2
#     A ->  .  NA                 from 2
#     E ->  .  (  E  )            from 2
#     E ->  .  E  -  E            from 2
#     E ->  .  E  +  E            from 2
#     E ->  .  id                 from 2
#     E ->  .  id  (  A  )        from 2
#     E ->  id  (  .  A  )        from 0
#     E ->  id  (  A  .  )        from 0
#     NA ->  .  E                 from 2
#     NA ->  .  E  ,  NA          from 2

# == chart[3]
#     E ->  (  .  E  )            from 2
#     E ->  .  (  E  )            from 3
#     E ->  .  E  -  E            from 3
#     E ->  .  E  +  E            from 3
#     E ->  .  id                 from 3
#     E ->  .  id  (  A  )        from 3

# == chart[4]
#     E ->  (  E  .  )            from 2
#     E ->  E  .  -  E            from 3
#     E ->  E  .  +  E            from 3
#     E ->  id  .                 from 3
#     E ->  id  .  (  A  )        from 3

# == chart[5]
#     A ->  NA  .                 from 2
#     E ->  (  E  )  .            from 2
#     E ->  E  .  -  E            from 2
#     E ->  E  .  +  E            from 2
#     E ->  id  (  A  .  )        from 0
#     NA ->  E  .                 from 2
#     NA ->  E  .  ,  NA          from 2

# == chart[6]
#     E ->  .  (  E  )            from 6
#     E ->  .  E  -  E            from 6
#     E ->  .  E  +  E            from 6
#     E ->  .  id                 from 6
#     E ->  .  id  (  A  )        from 6
#     NA ->  .  E                 from 6
#     NA ->  .  E  ,  NA          from 6
#     NA ->  E  ,  .  NA          from 2

# == chart[7]
#     A ->  NA  .                 from 2
#     E ->  E  .  -  E            from 6
#     E ->  E  .  +  E            from 6
#     E ->  id  (  A  .  )        from 0
#     E ->  id  .                 from 6
#     E ->  id  .  (  A  )        from 6
#     NA ->  E  ,  NA  .          from 2
#     NA ->  E  .                 from 6
#     NA ->  E  .  ,  NA          from 6

# == chart[8]
#     E ->  E  .  -  E            from 0
#     E ->  E  .  +  E            from 0
#     E ->  id  (  A  )  .        from 0
#     S ->  E  .                  from 0

# What was the grammar?

# For this challenge problem, we do not provide any testing hints. However,
# you may use any code you like from any source (such as this class).

grammar = []