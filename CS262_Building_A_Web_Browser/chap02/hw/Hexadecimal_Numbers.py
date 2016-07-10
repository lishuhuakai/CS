# Hexadecimal Numbers
#
# In this exercise you will write a lexical analyzer that breaks strings up
# into whitespace-separated identifiers and numbers. An identifier is a
# sequence of one or more upper- or lower-case letters. In this exercise,
# however, there are two types of numbers: decimal numbers, and
# _hexadecimal_ numbers.
#
# Humans usually write numbers using "decimal" or "base 10" notation. The
# number# 234 means 2*10^2 + 3*10 + 4*1.
#
# It is also possible to write numbers using other "bases", like "base 16"
# or "hexadecimal". Computers often use base 16 because 16 is a convenient
# power of two (i.e., it is a closer fit to the "binary" system that
# computers use internally). A hexadecimal number always starts with the
# two-character prefix "0x" so that you know not to mistake it for a binary
# number. The number 0x234 means
#       2 * 16^2
#     + 3 * 16^1
#     + 4 * 16^0
# = 564 decimal.
#
# Because base 16 is larger than base 10, the letters 'a' through 'f' are
# used to represent the numbers '10' through '15'. So the hexadecimal
# number 0xb is the same as the decimal number 11. When read out loud, the
# "0x" is often pronounced like "hex". "0x" must always be followed by at
# least one hexadecimal digit to count as a hexadecimal number.
#
# Modern programming languages like Python can understand hexadecimal
# numbers natively! Try it:
#
# print 0x234  # uncomment me to see 564 printed
# print 0xb    # uncomment me to see 11 printed
#
# This provides an easy way to test your knowledge of hexadecimal.
#
# For this assignment you must write token definition rules (e.g., t_ID,
# t_NUM_hex) that will break up a string of whitespace-separated
# identifiers and numbers (either decimal or hexadecimal) into ID and NUM
# tokens. If the token is an ID, you should store its text in the
# token.value field. If the token is a NUM, you must store its numerical
# value (NOT a string) in the token.value field. This means that if a
# hexadecimal string is found, you must convert it to a decimal value.
#
# Hint 1: When presented with a hexadecimal string like "0x2b4", you can
# convert it to a decimal number in stages, reading it from left to right:
#       number = 0              # '0x'
#       number = number * 16
#       number = number + 2     # '2'
#       number = number * 16
#       number = number + 11    # 'b'
#       number = number * 16
#       number = number + 4     # '4'
# Of course, since you don't know the number of digits in advance, you'll
# probably want some sort of loop. There are other ways to convert a
# hexadecimal string to a number. You may use any way that works.
#
# Hint 2: The Python function ord() will convert a single letter into
# an ordered internal numerical representation. This allows you to perform
# simple arithmetic on numbers:
#
# print ord('c') - ord('a') == 2

import ply.lex as lex

tokens = ('NUM', 'ID')

####
# Fill in your code here.
####

def t_ID(token):
    r'[a-zA-Z]+'
    return token

def t_NUM_hex(token):
    r'0x[0-9a-f]+'
    """
    hexnumber = token.value[2:]
    number = 0
    for i in range(len(hexnumber)):
        number = number * 16
        if hexnumber[i] >= '0' and hexnumber <= '9'
            number = number + ord(hexnumber[i]) - ord('9')
        else:
            number = number + 10 + ord(hexnumber[i]) - ord('a')
    """
    token.value = token.value[2:]
    sum = 0
    for each_char in token.value:
        sum *= 16
        if (ord(each_char) > ord('9')):
            sum += ord(each_char) - ord('a') + 10
        else:
            sum += ord(each_char) - ord('0')
    token.value = sum
    token.type = 'NUM'
    return token


def t_NUM_decimal(token):
  r'[0-9]+'
  token.value = int(token.value) # won't work on hex numbers!
  token.type = 'NUM'
  return token

t_ignore = ' \t\v\r'

def t_error(t):
  print("Lexer: unexpected character " + t.value[0])
  t.lexer.skip(1)

# We have included some testing code to help you check your work. You will
# probably want to add your own additional tests.
lexer = lex.lex()

def test_lexer(input_string):
  lexer.input(input_string)
  result = [ ]
  while True:
    tok = lexer.token()
    if not tok: break
    result = result + [(tok.type, tok.value)]
  return result

question1 = "0x19 equals 25" # 0x19 = (1*16) + 9
answer1 = [('NUM', 25), ('ID', 'equals'), ('NUM', 25) ]

print(test_lexer(question1) == answer1)

question2 = "0xfeed MY 0xface"
answer2 = [('NUM', 65261), ('ID', 'MY'), ('NUM', 64206) ]

print(test_lexer(question2) == answer2)

question3 = "tricky 0x0x0x"
answer3 = [('ID', 'tricky'), ('NUM', 0), ('ID', 'x'), ('NUM', 0), ('ID', 'x')]
print(test_lexer(question3) == answer3)


question4 = "in 0xdeed"
print(test_lexer(question4))

question5 = "where is the 0xbeef"
print(test_lexer(question5))

