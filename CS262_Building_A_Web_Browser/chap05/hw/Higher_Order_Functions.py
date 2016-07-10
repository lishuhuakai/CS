# Higher-Order Functions
#
# Back in Unit 3 we introduced Python List Comprehensions -- a concise
# syntax for specifying a new list in terms of a transformation of an old
# one.
#
# For exmaple:
#
# numbers = [1,2,3,4,5]
# odds = [n for n in numbers if n % 2 == 1]
# squares = [n * n for n in numbers]
#
# That code assigns [1,3,5] to odds and [1,4,9,16,25] to squares. The first
# operation is sometimes called "filter" (because we are filtering out
# unwanted elements) and the second operation is sometimes called "map"
# (because we are mapping, or transforming, all of the elements in a list).
#
# Python also has functions behave similarly:
#
# odds = filter(lambda n : n % 2 == 1, numbers)
# squares = map(lambda n : n * n, numbers)
#
# The filter() and map() definitions for odds and squares produce the same
# results as the list comprehension approaches. In other words, we can
# define (or implement) list comprehensions in terms of map and filter.
#
# In this exercise we take that notion one step beyond, by making
# specialized maps and filters. For example, suppose that we know that we
# will be filtering many lists down to only their odd elements. Then we
# might want something like this:
#
# filter_odds = filter_maker(lambda n : n % 2 == 1)
# odds = filter_odds(numbers)
#
# In this example, "filter_maker()" is a function that takes a function as
# an argument and returns a function as its result. We say that
# filter_maker is a *higher order function*.
#
# Complete the code below with definitions for filter_maker() and
# map_maker().
#
# Hint: You can use either "lambda" or nested Python function definitions.
# Both will work. The function you return from filter_maker(f) will have to
# reference f, so you'll want to think about nested environments.

def filter_maker(f):
    def filter_helper(l):
        return list(filter(f, l))
    return filter_helper
        # Fill in your code here. You must return a function.

def map_maker(f):
    def map_helper(l):
        return list(map(f, l))
    return map_helper
        # Fill in your code here. You must return a function.

# We have included a few test cases. You will likely want to add your own.
numbers = [1,2,3,4,5,6,7]
filter_odds = filter_maker(lambda n : n % 2 == 1)
print(filter_odds(numbers))
print(filter_odds(numbers) == [1,3,5,7])

length_map = map_maker(len)
words = "Scholem Aleichem wrote Tevye the Milkman, which was adapted into the musical Fiddler on the Roof.".split()
print(length_map(words) == [7, 8, 5, 5, 3, 8, 5, 3, 7, 4, 3, 7, 7, 2, 3, 5])

string_reverse_map = map_maker(lambda str : str[::-1])
# str[::-1] is cute use of the Python string slicing notation that
# reverses str. A hidden gem in the homework!
print(string_reverse_map(words) == ['melohcS', 'mehcielA', 'etorw', 'eyveT', 'eht', ',namkliM', 'hcihw', 'saw', 'detpada', 'otni', 'eht', 'lacisum', 'relddiF', 'no', 'eht', '.fooR'])

square_map = map_maker(lambda n : n * n)
print([n*n for n in numbers if n % 2 == 1] == square_map(filter_odds(numbers)))
