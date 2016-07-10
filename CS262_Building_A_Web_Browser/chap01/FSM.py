# FSM Simulation Quiz
edge = {
    (1, 'a') : 2,
    (2, 'a') : 2,
    (2, '1') : 3,
    (3, '1') : 3
}
accepting = [ 3 ]
# corresponds to r"a+1+"
def fsmim(string,current,edges,accepting):
    if string == "":
        return current in accepting
    else:
        letter = string[0]
        # QUIZ: You fill this out!
        # Is there a valid edge?
        # if so, take it.
        # if not, return False.
        # Hint: recursion.
        try:
            next = edges[(current, letter)]
        except Exception:
            return False
        else:
            return fsmim(string[1:], next, edges, accepting)
print(fsmim("aaa1", 1, edge, accepting))


def fsmimEx(string,current,edges,accepting):
    if string == "":
        return current in accepting
    else:
        letter = string[0]
        # QUIZ: You fill this out!
        # Is there a valid edge?
        # if so, take it.
        # if not, return False.
        # Hint: recursion.
       if (current, letter) in edges:
            destination = edges[(current, letter)]
            remaning_string = string[1:]
            return fsmimEx(remaning_string, destination, edges, accepting)
       else:
            return False



