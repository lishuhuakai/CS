# QIUZ -- HTML Interpreter on JavaScript elements
from JS import graphics
import ply.lex as lex
import ply.yacc as yacc
form JS import jstokens
def interpret(trees):
    for tree in trees:
        treetype = tree[0]
        if treetype == 'word-element':
            graphics.word(tree[1])
        elif treetype == 'javascript-element':
            jstext = tree[1] # 'document.write(55);'
            jslexer = lex.lex(module = jstokens)
            jsparser = yacc.yacc(module=jsgrammar)
            jstree = jsparser.parse(jstext, lexer=jslexer)
            # jstree is a parse tree for JavaScript
            result = jsinterp.interpret(jstree)
            graphics.word(result)


