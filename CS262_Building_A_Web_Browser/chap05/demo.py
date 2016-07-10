def map_demo(f):
    def helper(l):
        return map(f, l)
    return helper

mape = map_demo(lambda x: x + 1)
print(mape([1, 2]))