a = { 1 : 'hi'}
b = {2 : 'girls'}

#c = dict(a.items() + b.items())
c = a.copy()
c.update(b)
print(c)