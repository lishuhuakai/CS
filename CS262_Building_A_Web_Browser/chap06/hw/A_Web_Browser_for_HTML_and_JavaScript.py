# A Web Browser For HTML + JavaScript
#
# Congratulations! You have written:
#
#       regular expressions defining HTML tokens
#       and thus an HTML lexical analyzer
#
#       regular expressions defining JavaScript tokens
#       and thus a JavaScript lexical analyzer
#
#       a context-free grammar defining well-formed HTML
#       and an HTML parser that makes abstract syntax trees
#
#       a context-free grammar defining well-formed JavaScript
#       and an HTML parser than makes abstract syntax trees
#
#       a recursive tree-walking interpreter for HTML
#               that calls a graphics library and a JavaScript interpreter
#
#       a recursive tree-walking interpreter for JavaScript
#               that handles statements, expressions, variables,
#               environments and functions
#
#       an optional interpreter for simplifying JavaScript
#
#       glue code that follows software design decisions to link them all
#       together
#
# Those are the major components of our web browser. This class did not
# support all of JavaScript, and you did not write the graphics library,
# but those are minor concerns. We could easily imagine adding "for loops"
# or other JavaScript syntax as needed, now that you have the base
# language.
#
# For this problem, you must submit "your favorite" webpage text that our
# web browser can handle.
#
# The only official requirement is that your webpage text contain
# correctly-nested tags, start with <html>, and contain <script
# type="text/javascript"> somewhere inside that HTML.
#
# Beyond that, you will receive full credit for whatever you submit. Our
# web browser is not as "forgiving" as commercial browsers, but it is still
# possible to make interesting webpages with it. Use your creativity!

webpage = """<html>
Put something here. You must include embedded JavaScript for full credit,
but beyond that, do whatever you like.
</html>
"""

import ply.lex as lex
import ply.yacc as yacc
from Web import htmltokens
from Web import htmlgrammar
from Web import htmlinterp
from Web import graphics as graphics
from Web import jstokens


htmllexer = lex.lex(module=htmltokens)
htmlparser = yacc.yacc(module=htmlgrammar,tabmodule="parsetabhtml")
ast = htmlparser.parse(webpage,lexer=htmllexer)
jslexer = lex.lex(module=jstokens)
graphics.initialize() # Enables display of output.
htmlinterp.interpret(ast)
graphics.finalize() # Enables display of output.

# We would also like to take this opportunity to thank everyone in Udacity for
# their support in making this class happen, especially those behind the scenes.
# Check out team.jpeg for a group photo!