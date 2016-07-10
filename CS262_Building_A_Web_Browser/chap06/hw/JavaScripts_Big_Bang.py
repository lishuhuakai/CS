# JavaScript's Big Bang
#
# In class we saw one way to integrate our HTML Interpreter and our
# JavaScript interpreter to make a web browser. Our JavaScript interpreter
# returned a string, which was then rendered unchanged on the webpage.
#
# In practice, however, JavaScript output may include HTML tags and should
# be lexed, parsed and interpreted again. For example, on modern web
# browsers the following webpage ...
#
# <html>
# <script type="text/javascript">
# document.write("Tags in <i>my</i> output should be processed.");
# </script>
# </html>
#
# Does not output the literal string "Tags in <i>my</i> output should be
# processed." Instead, the <i> tags are lexed, parsed and interpreted
# again, and the web page contains "Tags in my output should be processed."
# with the word "my" drawn in italics.
#
# This sort of recursive dependence -- in which intepreted HTML contains
# JavaScript which runs and creates new HTML which is then interpreted,
# and so on, is the heart of JavaScript's power. You can visualize it like
# a snake eating its own tail: http://en.wikipedia.org/wiki/Ouroboros
#
# In this assignment you will extend our web browser so that the string
# produced by JavaScript is not merely passed to the graphics library as a
# word, but is instead lexed, parsed and interpreted as HTML. (For the
# purposes of this assignment, if JavaScript creates HTML, it must created
# only well-balanced tags.)
#
# Below is the top-level HTML Interpreter code for the web browser. You
# will not need to change any lexer definitions, token definitions, or
# anything about the JavaScript interpreter.
#
# Hint: The required extension can be made by changin as few as three lines
# (because you already know so much about this topic)! It does require you
# to understand how lexers, parser and interpreters all fit together.

import ply.lex as lex
import ply.yacc as yacc
from Web import graphics as graphics
from Web import jstokens
from Web import jsgrammar
from Web import jsinterp
from Web import htmltokens
from Web import htmlgrammar

# Load up the lexers and parsers that you have already written in
# previous assignments. Do not worry about the "module" or
# "tabmodule" arguments -- they handle storing the JavaScript
# and HTML rules separately.
htmllexer  = lex.lex(module=htmltokens)
htmlparser = yacc.yacc(module=htmlgrammar,tabmodule="parsetabhtml")
jslexer    = lex.lex(module=jstokens)
jsparser   = yacc.yacc(module=jsgrammar,tabmodule="parsetabjs")

# The heart of our browser: recursively interpret an HTML abstract
# syntax tree.
def interpret(ast): # 用来解释语法树的一个玩意
        for node in ast:
                nodetype = node[0] # node的类型
                if nodetype == "word-element":
                        graphics.word(node[1])
                elif nodetype == "tag-element":
                        tagname = node[1]; # tag的名字
                        tagargs = node[2]; # tag的参数
                        subast = node[3]; # 子树
                        closetagname = node[4]; # 闭合的tag
                        if (tagname != closetagname): # 如果不匹配的话，就警告
                                graphics.warning("(mistmatched " + tagname + " " + closetagname + ")")
                        else:
                                graphics.begintag(tagname,tagargs);
                                interpret(subast)
                                graphics.endtag();
                elif nodetype == "javascript-element": # 如果是javascript-element，那么就要解释这个玩意了
                        jstext = node[1]; # 首先得到对应的代码段
                        jsast = jsparser.parse(jstext,lexer=jslexer)
                        result = jsinterp.interpret(jsast) # 这里通过解释得到js代码输出的结果
                        # 接下来，我们要继续对result进行parser
                        ht = htmlparser.parse(result,lexer=htmllexer)
                        interpret(ht)
                        #graphics.word(result) # 输出对应的word

# Here is an example webpage that includes JavaScript that generates HTML.
# You can use it for testing.
webpage = """<html>
<h1>JavaScript That Produces HTML</h1>
<p>
This paragraph starts in HTML ...
<script type="text/javascript">
write("<b>This whole sentence should be bold, and the concepts in this problem touch on the <a href='http://en.wikipedia.org/wiki/Document_Object_Model'>Document Object Model</a>, which allows web browsers and scripts to <i>manipulate</i> webpages.</b>");
</script>
... and this paragraph finishes in HTML.
</p>
<hr> </hr> <!-- draw a horizontal bar -->
<p>
Now we will use JavaScript to display even numbers in <i>italics</i> and
odd numbers in <b>bold</b>. <br> </br>
<script type="text/javascript">
function tricky(i) {
  if (i <= 0) {
    return i;
  } ;
  if ((i % 2) == 1) {
    write("<b>");
    write(i);
    write("</b>");
  } else {
    write("<i>");
    write(i);
    write("</i>");
  }
  return tricky(i - 1);
}
tricky(10);
</script>
</p>
</html>"""

webpage1 = """
<html>
<b>This whole sentence should be bold, and the concepts in this problem touch on the
<a href='http://en.wikipedia.org/wiki/Document_Object_Model'>Document Object Model</a>,
which allows web browsers and scripts to <i>manipulate</i> webpages.</b>
... and this paragraph finishes in HTML.
<p>
<hr> </hr> <!-- draw a horizontal bar -->
</p>
Now we will use JavaScript to display even numbers in <i>italics</i> and
odd numbers in <b>bold</b>. <br> </br>
<script type="text/javascript">
function tricky(i) {
  if (i <= 0) {
    return i;
  } ;
  if ((i % 2) == 1) {
    write("<b>");
    write(i);
    write("</b>");
  } else {
    write("<i>");
    write(i);
    write("</i>");
  }
  return tricky(i - 1);
}
tricky(10);
</script>
</html>
"""

htmlast = htmlparser.parse(webpage,lexer=htmllexer)
graphics.initialize() # let's start rendering a webpage
interpret(htmlast)
graphics.finalize() # we're done rendering this webpage
