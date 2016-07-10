print("hello"[1:3]) # not include 3

def myfirst_yoursecond(p, q):
    pindex = p.find(" ")
    qindex = q.find(" ")
    return p[:pindex] == q[qindex + 1:]

print(myfirst_yoursecond("hi girl", "girls hi"))

print("Jane Eyre".split())

import re
"[a-z]+((\s)?[0-9](\s)?)"
r"[a-z]+\( *[0-9] *\)"

"Quoted Strings"

"I said, \"hello .\""
# . matches any character but a newline
r"[0-9].[0-9]"
# ^x matches any character but not a x
"(?:xyz)+"

print(re.findall(r"do+|re+|mi+","mimi rere midore doo-wap"))
# (?: marks the beginning of such a group
print(re.findall(r"(?:do|re|mi)+","mimi rere midore doo-wap"))

# We must escape the (\\) to get Python to
# leave it alone!
s1 = '"You say, \\"yes\\", I say \\"no.\\""'
s2 = '"I dont know why you say, \\"goodbye.\\""'
s3 = '"I say, \\"hello.\\"'
print(re.findall(r'"(?:[^\\]|(?:\\.))*"', s1))

print(s1)
print(s2)
print(s3)
