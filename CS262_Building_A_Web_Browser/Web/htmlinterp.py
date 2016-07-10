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