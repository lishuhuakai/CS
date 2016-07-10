# Wes Weimer
#
# This is a grammar definition file for a simple subset of HTML with
# embedded JavaScript.
#

import ply.yacc as yacc
from Web.htmltokens import tokens

start = 'html' 

tag_stack = [] # for error reporting

def p_html(p):
        'html : element html'
        p[0] = [p[1]] + p[2] 
def p_html_empty(p):
        'html : '
        p[0] = [ ] 

def p_element_word(p): 
        'element : WORD'
        p[0] = ('word-element',p[1])

def p_element_word_eq(p): 
        'element : EQUAL'
        p[0] = ('word-element',p[1])

def p_element_word_string(p): 
        'element : STRING'
        p[0] = ('word-element',"``" + p[1] + "''")

def p_tagname(p): 
        'tagname : WORD'
        global tag_stack 
        tag_stack = [(p[1],p.lineno(1))] + tag_stack 
        p[0] = p[1] 

def p_tagnameend(p): 
        'tagnameend : WORD'
        global tag_stack 
        if (tag_stack[0])[0] != p[1]:
                print("HTML Syntax Error: <" + tag_stack[0][0] + "> on line " + str(tag_stack[0][1]) + " closed by </" + str(p[1]) + "> on line " + str(p.lineno(1)))
                exit(1) 
        tag_stack = tag_stack[1:] 
        p[0] = p[1] 

def p_element_tag_empty(p):
        # <br /> 
        'element : LANGLE tagname tag_arguments SLASHRANGLE'
        global tag_stack 
        tag_stack = tag_stack[1:] 
        p[0] = ('tag-element',p[2],p[3],[],p[2]) 

def p_element_tag(p):
        # <span color=red>Important text!</span> 
        'element : LANGLE tagname tag_arguments RANGLE html LANGLESLASH tagnameend RANGLE'
        p[0] = ('tag-element',p[2],p[3],p[5],p[7]) 

def p_tag_arguments(p):
        'tag_arguments : tag_argument tag_arguments'
        p[0] = p[1].copy()
        p[0].update(p[2])
        # p[0] = dict(p[1].items() + p[2].items())
        # 这里存在着比较严重的问题

def p_tag_arguments_empty(p):
        'tag_arguments : '
        p[0] = { } # 这里构成了一个dict

def p_tag_argument_word(p): 
        'tag_argument : WORD EQUAL WORD' 
        p[0] = { p[1].lower() : p[3] }  # 好吧，也是一个dict

def p_tag_argument_string(p): 
        'tag_argument : WORD EQUAL STRING' 
        p[0] = { p[1].lower() : p[3] } 

def p_element_javascript(p):
        'element : JAVASCRIPT'
        p[0] = ("javascript-element",p[1]) 

def p_error(t):
        if tag_stack[0] != []: 
                print("HTML Syntax Error: <" + tag_stack[0][0] + "> on line " + str(tag_stack[0][1]) + " never closed")
        else: 
                tok = yacc.token() 
                print("HTML Syntax Error: Near Token " + str(tok))
        exit(1) 
