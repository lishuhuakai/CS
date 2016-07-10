# A Gruesome Problem
#
# Focus: Units 1 and 2, Regular Expressions and Lexical Analysis
#
#
# In this problem you will use re.findall() to find all words matching a
# certain property. Taking liberties with Nelson Goodman's paradox, we say
# that a word is "blue" if it contains a lowercase "b" before a lowercase
# "t" and a word is "green" if it contains a lowercase "g" after a
# lowercase "t". For example:
#
#       "wombat"        is blue (b before t)
#       "fabricate"     is blue (b before t)
#       "habitat"       is blue (b before t)
#       "waftage"       is green (g after t)
#       "rackateering"  is green (g after t)
#       "abating"       is blue and gree (b ... t and t ... g)
#
# We say that a word is "grue" if it is EITHER "green" OR "blue" (or
# both). For this problem, words are non-empty sequences of the
# English letters a-z and/or A-Z. Write a procedure gruesome() that
# takes a string as an argument and returns a list of all "grue"some
# words (i.e., words that contain either a b before a t or that
# contain a g after a t) in that given string.

import re

def gruesome(str):
    blue = re.findall(r'[ac-zA-Z]*b[a-su-zA-Z]*t[a-zA-Z]*', str)
    green = re.findall(r'[a-su-zA-Z]*t[a-fh-zA-Z]*g[a-zA-Z]*', str)
    grue = blue + [e for e in green if e not in blue]
    return grue
        # write your code here


# We have included some testing code to help you check your work. Since
# this is the final exam, you will definitely want to add your own tests.

test1 = "A gruesome wombat practiced rackateering!"
answer1 = [ "wombat", "rackateering" ]
print(gruesome(test1) == answer1)

test2 = """Sir Walter Elliot, of Kellynch Hall, in Somersetshire, was
a man who, for his own amusement, never took up any book but the
Baronetage; there he found occupation for an idle hour, and
consolation in a distressed one; there his faculties were roused into
admiration and respect, by contemplating the limited remnant of the
earliest patents; there any unwelcome sensations, arising from
domestic affairs changed naturally into pity and contempt as he turned
over the almost endless creations of the last century; and there, if
every other leaf were powerless, he could read his own history with an
interest which never failed. This was the page at which the favourite
volume always opened:""" # Jane Austen

print(gruesome(test2) == ['but', 'Baronetage', 'contemplating'])

test3 = """Every inch of wall space is covered by a bookcase. Each
bookcase has six shelves, going almost to the ceiling. Some
bookshelves are stacked to the brim with hardcover books: science,
mathematics, history, and everything else. Other shelves have two
layers of paperback science fiction, with the back layer of books
propped up on old tissue boxes or two-by-fours, so that you can see
the back layer of books above the books in front. And it still isn't
enough. Books are overflowing onto the tables and the sofas and making
little heaps under the windows.""" # E. Yudkowsky
print(gruesome(test3) == ['everything'])

test4 = """The reign of terror caused by wombats sseems to be abating."""
print(gruesome(test4) == ['wombats', 'abating'])
