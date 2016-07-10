import ply.lex as lex
import ply.yacc as yacc

# Fill in your code here.
####################################################
## lexer
tokens = (
    'LPAREN', 'RPAREN', 'STAR', 'OR', 'LETTER'
)

t_LPAREN = r'\('
t_RPAREN = r'\)'
t_STAR = r'\*'
t_OR = r'\|'
t_LETTER = r'[a-zA-Z]' # 用于匹配一个字母

def t_error(t):
  print("Lexer: unexpected character " + t.value[0])
  t.lexer.skip(1)

def interpret(p):
    pass
####################################################

####################################################
## parse

start = 're' # the start symbol in our grammar
# re表达式是一系列exp的组合
def p_RE(p):
    """
    re : exp exps
    """
    #print('p_RE ' + p[0])
    p[0] = [p[1], p[2]]


def p_exp(p): # 这些exp包括
    """
    exp : closedexp
        | orexp
        | starexp
    """
    p[0] = p[1]

def p_exp_letter(p):
    'exp : LETTER letters'
    p[0] = ('concat', ('letter', p[1]), p[2])

def p_str(p):
    'letters : LETTER letters'
    p[0] = ('letter', p[1], p[2])

def p_str_e(p):
    'letters : '
    p[0] = ('empty',)

def p_closedexp(p):
    'closedexp : LPAREN exp RPAREN'
    p[0] = p[2]

def p_exps_empty(p):
    'exps :'
    p[0] = ('empty',)

def p_exps(p):
    'exps : exp exps'
    p[0] = ('concat', p[1], p[2])

def p_orexp(p):
    'orexp : exp OR exp'
    p[0] = ('or', p[1], p[3])



def p_STAR(p):
    'starexp : exp STAR'
    p[0] = ('star', p[1]) # 构建一个star

####################################################
lexer = lex.lex() # 需要构建一个词法分析器
parser = yacc.yacc() # 语法分析器


def lexing(str):
    # 做词法分析
    lexer.input(str)
    while True:
        tok = lexer.token()
        if not tok: break
        print(tok)

def parsing(input_string):
    try:
        parse_tree = parser.parse(input_string, lexer=lexer)
        return parse_tree
    except:
        return "error"

# lexing('a(b*)cd')
print(parsing('a(b*)c'))
print(parsing('((ab)|(cd))*'))
# 最起码，现在解析出来的树是正确的

# 构建一棵语法树

def re_to_nfsm(re_string):
        # Feel free to overwrite this with your own code.
        lexer.input(re_string)
        parse_tree = parser.parse(re_string, lexer=lexer)
        return interpret(parse_tree)

current_state = 1
next_state = 2

def interpret(ast):
    new_edges = {}
    def helper(edges, ast):
        for i in ast:
            global current_state, next_state
            k = i[0]
            if i[0] == 'letter': # 如果是一个字母的话
                edges[(current_state, i[1])] = [next_state]
                current_state += 1
                next_state += 1
            elif i[0] == 'concat': # 如果是连接操作的话
                for e in i[1:]:
                    helper(edges, [e]) # 每一个连接操作都要构建
            elif i[0] == 'star': # 来构建star的状态
                mark_cur_stat = current_state # 记录下当前的状态
                helper(edges, [i[1]]) # 先对子树构建
                if (mark_cur_stat, None) in edges:
                    edges[(mark_cur_stat, None)] += [next_state]
                else:
                    edges[(mark_cur_stat, None)] = [next_state]
                edges[(current_state, None)] = [mark_cur_stat, next_state]
                current_state += 1
                next_state += 1
            elif i[0] == 'or':
                mark_cur_stat = current_state
                first_branch_start_stat = next_state
                edges[(mark_cur_stat, None)] = [first_branch_start_stat]
                current_state += 1
                next_state += 1
                helper(edges, [i[1]]) # 先构建第一个分支
                first_branch_end_state = current_state # 记录下第一个分支结束的状态
                second_branch_start_stat = next_state
                current_state += 1
                next_state += 1
                edges[(mark_cur_stat, None)] = edges[(mark_cur_stat, None)] + [second_branch_start_stat]
                helper(edges, [i[2]]) # 构建第二个分支
                second_branch_end_stat = current_state # 记录下第二个分支结束额状态
                edges[(first_branch_end_state, None)] = [next_state]
                edges[(second_branch_end_stat, None)] = [next_state]
                current_state += 1
                next_state += 1
                # 然后构建第二个分支
        return edges

    return helper(new_edges, ast)

print(re_to_nfsm("((ab)|(cd))*"))
print(' ')
current_state = 1
next_state = 2
print(re_to_nfsm('a(b*)c'))


