# Bank Heist
#
# Suppose you are a daring thief who has infiltrated the palace of an evil
# dictator. You need to be quick when making your escape, so you can only
# carry 20 kilograms of unbroken goods out with you. Despite this, you want
# to escape with items worth as much money as possible. Suppose further
# that there are three artifacts available:
#
#       a 1502 ACE Incan statue massing 15 kilograms, valued at $30000
#       a 1499 ACE Aztec jade mask massing 9 kilograms, valued at $16000
#       a 300 ACE Mayan urn massing 8 kilograms, valued at $15000
#
# It is not possible to take all three, and even though the Incan statue
# has the highest value-to-mass ratio, given a 20 kilogram limit the best
# choice is to take the Aztec mask and the Mayan urn.
#
# This is the setup for the "0-1 Knapsack Problem", an important task in
# theoretical computer science. In general, deciding what to take given a
# list of items and a mass limit is believed to be very difficult. This
# question does not ask you to come up with a solution, however -- instead,
# you will evaluate solutions.
#
# We can encode a problem instance as follows. We'll use a dictionary
# mapping item names to (mass,value) pairs.
#
available = {
        "statue" : (15, 30000) ,
        "mask" : (9, 16000) ,
        "urn" : (8, 15000) ,
}
#
# Then the mass limit and the taken objects are just a number and a
# string list respectively:
limit = 20
taken = [ "mask", "urn" ]
#
# Write a Python procedure heistvalid(available,limit,taken) that returns
# True (the boolean value True, not the string "True") if the objects
# in the list `taken' have a total mass less than or equal to the limit.
#
# In addition, write a Python procedure heisttotal(available,limit,taken)
# that returns the total value (as a number) of the objects in the list
# `taken'.
#
# This problem is meant to provide practice for *list comprehensions*. Make
# the body each of your procedures *at most one line long*.
#
# Hint: sum([1,2,3]) == 6

def heistvalid(available, limit, taken):
    cost = limit
    for each_item in taken:
        cost -= available[each_item][0]
    return cost > 0

    # Replace this line with your one-line answer.



def heisttotal(available, limit, taken):
    return sum(available[each_item][1] for each_item in taken)

    # Replace this line with your one-line answer.

# We have provided some test cases. You will likely want to make others.
print(heistvalid(available, limit, taken) == True)
#print(heisttotal(available, limit, taken))
print(heisttotal(available, limit, taken) == 31000)
allthree = ["statue", "mask", "urn"]
print(heistvalid(available, limit, allthree) == False)
print(heisttotal(available, limit, allthree) == 61000)
