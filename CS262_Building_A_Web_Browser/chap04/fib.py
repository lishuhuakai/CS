import timeit
t = timeit.Timer(stmt="""
chart = {}
def memofibo(n):
    if n in chart: # 好飘逸的写法
        return chart[n]
    elif n <= 2:
        chart[n] = 1
    else:
        chart[n] = memofibo(n-1) + memofibo(n-2)
    return chart[n]
memofibo(25)
"""
)
print(t.timeit(number=100))
