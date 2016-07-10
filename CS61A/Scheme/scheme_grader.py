"""Automatic grading script for the Scheme project.

Expects the following files in the current directory.

scheme.py
scheme_reader.py
scheme_primitives.py, scheme_tokens.py, scheme_test.py
buffer.py, ucb.py
autograder.py
"""

__version__ = '1.4'

from autograder import test, run_tests, check_func, check_doctest, test_eval

try:
    import scheme, scheme_reader # 也就是说，导入这些模块，很可能引起错误，是吧！
except (SyntaxError, IndentationError) as e:
    import traceback
    print('Unfortunately, the autograder cannot run because ' +
          'your program contains a syntax error:')
    traceback.print_exc(limit=1)
    exit(1)

import sys
from ucb import main
import scheme_primitives
from scheme import read_line, Pair, nil, LambdaProcedure, create_global_frame

@test('1') # 装饰器，将这个玩意包裹了一层
def problem_1(grades):
    tests1 = [
        ('3', 3),
        ('-123', -123),
        ('1.25', 1.25),
        ('true', True),
        (')', 'Error'),
        ("'x", pairify(['quote', 'x'])),
        ('(quote x)', pairify(['quote', 'x'])),
        ("'(a b)", pairify(['quote', ['a', 'b']])),
        ("'(a (b c))", pairify(['quote', ['a', ['b', 'c']]])),
        ("(a (b 'c))", pairify(['a', ['b', ['quote', 'c']]])),
        ("(a (b '(c d)))", pairify(['a', ['b', ['quote', ['c', 'd']]]])),
        ("')", 'Error')
    ]
    # catch_syntax_error是什么玩意
    if check_func(catch_syntax_error(read_line), tests1, comp=scheme_equal):
        return True

@test('2')
def problem_2(grades):
    tests1 = [
        ('(a . b)', Pair('a', 'b')),
        ('(a)', Pair('a', nil)),
        ('(a b . c)', Pair('a', Pair('b', 'c')))
    ]
    tests2 = [
        ('(a b . c d)', 'Error'),
        ('((a b) (c (d (e f))))',
            pairify([['a', 'b'], ['c', ['d', ['e', 'f']]]])),
        ('(a . (b . (c . (d . ()))))',
            pairify(['a', 'b', 'c', 'd'])),
        ('(. . 2)', 'Error'),
        ('(2 . 3 4 . 5)', 'Error'),
        ('(2 (3 . 4) 5)', Pair(2, Pair(Pair(3, 4), Pair(5, nil)))),
    ]
    if check_func(catch_syntax_error(read_line), tests1, comp=scheme_equal):
        return True
    if check_func(catch_syntax_error(read_line), tests2, comp=scheme_equal):
        return True

@test('3')
def problem_3(grades):
    return check_doctest('apply_primitive', scheme)


@test('4')
def problem_4(grades):
    tests1 = [
        ('(+ 2 3)', 5),
        ('(+ 2 3 4 5 6 7)', 27),
        ('(+ 2)', 2),
        ('(+)', 0),
        ('(* (+ 3 2) (+ 1 7))', 40),
        ('(+ (* 3 (+ (* 2 4) (+ 3 5))) (+ (- 10 7) 6))', 57),
    ]
    tests2 = [
        ('(odd? 13)', True),
        ('(car (list 1 2 3 4))', 1),
        ('hello', 'SchemeError'),
        ('(car car)', 'SchemeError'),
        ('(odd? 1 2 3)', 'SchemeError'),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests2, comp=scheme_equal):
        return True

@test('5')
@test('A5')
def problem_A5(grades):
    tests1 = [
        ('(define size 2) size', 2),
        ('(define size 2) (* 5 size)', 10),
        ('(define pi 3.14159) (define radius 10) (* pi (* radius radius))',
                                  314.159),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True

@test('6')
@test('B6')
def problem_B6(grades):
    tests1 = [
        ("(list 'a 'b)", pairify(['a', 'b'])),
        ("(define a 1) (list a 'b)", pairify([1, 'b'])),
        ("(car '(a b c))", 'a'),
        ("(car (car '((a))))", 'a'),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True

@test('7')
def problem_7(grades):
    tests1 = [
        ('(begin (+ 2 3) (+ 5 6))', 11),
        ('(begin (display 3) (newline) (+ 2 3))', 5),
    ]
    tests2 = [
        ('(begin (define x 3) x)', 3),
        ('(define 0 1)', 'SchemeError'),
        ("(begin 30 'hello)", 'hello'),
        ("(begin (define x 3) (cons x '(y z)))", pairify([3, 'y', 'z'])),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests2, comp=scheme_equal):
        return True

@test('8')
def problem_8(grades):
    tests1 = [
        ('(lambda (x y) (+ x y))',
         LambdaProcedure(pairify(['x', 'y']),
                         pairify(['+', 'x', 'y']),
                         create_global_frame()))
    ]
    tests2 = [
        ('(lambda (x) (+ x) (+ x x))',
         LambdaProcedure(pairify(['x']),
                         pairify(['begin', ['+', 'x'], ['+', 'x', 'x']]),
                         create_global_frame())),
        ('(begin (define x (lambda () 2)) x)',
         LambdaProcedure(nil,
                         2,
                         create_global_frame())),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests2, comp=scheme_equal):
        return True

@test('9')
@test('A9')
def problem_A9(grades):
    tests1 = [
        ('(begin (define (f x y) (+ x y)) f)',
            LambdaProcedure(pairify(['x', 'y']),
                            pairify(['+', 'x', 'y']),
                            create_global_frame())),
        ('(begin (define (f) (+ 2 2)) f)',
            LambdaProcedure(nil,
                            pairify(['+', 2, 2]),
                            create_global_frame())),
        ('(begin (define (f x) (* x x)) f)',
            LambdaProcedure(pairify(['x']),
                            pairify(['*', 'x', 'x']),
                            create_global_frame())),
        ('(begin (define (f x) 1 2) f)',
            LambdaProcedure(pairify(['x']),
                            pairify(['begin', 1, 2]),
                            create_global_frame())),
        ('(define (0 x) (* x x))', 'SchemeError'),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True

@test('10')
def problem_10(grades):
    gf = create_global_frame()

    formals, vals = read_line('(a b c)'), read_line('(1 2 3)')
    call_frame = gf.make_call_frame(formals, vals)
    doctest = [
        (set(call_frame.bindings), {'a', 'b', 'c'}),
        (len(call_frame.bindings), 3),
        (call_frame.bindings['a'], 1),
        (call_frame.bindings['b'], 2),
        (call_frame.bindings['c'], 3),
        (call_frame.parent, gf),
        ]
    if check_func(lambda x: x, doctest, comp=scheme_equal):
        return True

    formals = read_line('(a b c)')
    args = read_line('(2 #t a)')
    lf = gf.make_call_frame(formals, args)
    tests1 = [
        (lf.bindings['a'], 2),
        (lf.bindings['b'], True),
        (lf.bindings['c'], 'a'),
        (lf.parent, gf),
    ]
    if check_func(lambda x: x, tests1, comp=scheme_equal):
        return True

    formals = read_line('(a)')
    args = read_line('(seven)')
    lf2 = lf.make_call_frame(formals, args)
    tests2 = [
        (lf.bindings['a'], 2),
        (lf.parent, gf),
    ]
    if check_func(lambda x: x, tests2, comp=scheme_equal):
        return True

@test('11')
@test('B11')
def problem_B11(grades):
    # Note: Doesn't check well-formed but unrequired list matching.
    # E.g., (lambda (a . b) 2) and (lambda x 2)
    tests1 = [
        ('(lambda (x y z) x)',
            LambdaProcedure(pairify(['x', 'y', 'z']),
                            'x',
                            create_global_frame())),
        ('(lambda (0 y z) x)', 'SchemeError'),
        ('(lambda (x y nil) x)', 'SchemeError'),
        ('(lambda (x y (and z)) x)', 'SchemeError'),
        ('(lambda (x #t z) x)', 'SchemeError'),
    ]
    tests2 = [
        ("(lambda (h e l l o) 'world)", 'SchemeError'),
        ("(lambda (c s 6 1 a) 'yay)", 'SchemeError'),
        ]

    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests2, comp=scheme_equal):
        return True

@test('12')
def problem_12(grades):
    tests1 = [
        ('(define (square x) (* x x)) (square 21)',
            441),
        ('(define square (lambda (x) (* x x))) (square (square 21))',
            194481),
    ]
    tests2 = [
        ("""(define square (lambda (x) (* x x)))
                 (define (sum-of-squares x y)
                   (+ (square x) (square y)))
                 (sum-of-squares 3 4)""",
            25) ,
        ("""(define double (lambda (x) (* 2 x)))
                 (define compose (lambda (f g) (lambda (x) (f (g x)))))
                 (define apply-twice (lambda (f) (compose f f)))
                 ((apply-twice double) 5)""",
            20),
        ("""(define (outer x y)
                  (define (inner z x)
                    (list x y z))
                  (inner x 10))
                 (outer 1 2)""",
            pairify([10, 2, 1])),
        ("""(define (outer-func x y)
                  (define (inner z x)
                    (list x y z))
                  inner)
                 ((outer-func 1 2)  1 10)""",
            pairify([10, 2, 1])),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests2, comp=scheme_equal):
        return True

@test('13')
@test('A13')
def problem_A13(grades):
    tests1 = [
        ('(if #t 1 0)', 1),
        ('(if #f 1 0)', 0),
        ('(if 1 1 0)', 1),
        ("(if 'a 1 0)", 1),
        ('(if (cons 1 2) 1 0)', 1),
        ('(if #t 1)', 1),
        ('(if #f 1)', scheme.okay),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True

@test('14')
@test('B14')
def problem_B14(grades):
    tests1 = [
        ('(and)', True),
        ('(and 1 #f)', False),
        ('(and 2 1)', 1),
        ('(and #f 5)', False),
        ('(and 3 2 #f)', False),
        ('(and 3 2 1)', 1),
        ('(and 3 #f 5)', False),
    ]
    tests2 = [
        ('(or)', False),
        ('(or 1)', 1),
        ('(or #f)', False),
        ("(or 0 1 2 'a)", 0),
        ('(or #f #f)', False),
        ("(or 'a #f)", 'a'),
        ("(or (< 2 3) (> 2 3) 2 'a)", True),
        ('(or (< 2 3) 2)', True),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests2, comp=scheme_equal):
        return True

@test('15')
@test('A15')
def problem_A15(grades):
    tests1 = [
        ("""(cond ((> 2 3) 5)
                  ((> 2 4) 6)
                  ((< 2 5) 7)
                  (else 8))""",
           7),
        ("""(cond ((> 2 3) 5)
                  ((> 2 4) 6)
                  ((< 2 5) 7))""",
           7),
        ("""(cond ((> 2 3) 5)
                  ((> 2 4) 6)
                  (else 8))""",
           8),
        ("""(cond ((> 2 3) 4 5)
                  ((> 2 4) 5 6)
                  ((< 2 5) 6 7)
                  (else 7 8))""",
           7),
        ("""(cond ((> 2 3) (display 'oops) (newline))
                  (else 9))""",
           9),
        ("""(cond ((< 2 1))
                   ((> 3 2))
                   (else 5))""",
            True),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True

@test('16')
@test('A16')
def problem_A16(grades):
    tests1 = [
        ("""(define (square x) (* x x))
                 (define (f x y)
                  (let ((a (+ 1 (* x y)))
                        (b (- 1 y)))
                    (+ (* x (square a))
                       (* y b)
                       (* a b))))
                 (f 3 4)""",
            456),
    ]
    tests2 = [
        ("""(define x 3)
                 (define y 4)

                 (let ((x (+ y 2))
                      (y (+ x 1)))
                  (cons x y))""",
            Pair(6, 4)),
        ("""(let ((x 'hello)) x)""",
            'hello'),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests2, comp=scheme_equal):
        return True

@test('17')
@test('B17')
def problem_B17(grades):
    tests1 = [
        ("""(define f (mu (x) (+ x y)))
                 (define g (lambda (x y) (f (+ x x))))
                 (g 3 7)""",
            13),
        ("""(define g (mu () x))
                 (define (high f x)
                   (f))
                 (high g 2)""",
            2),
    ]
    tests2 = [
        ("""(define (f x) (mu () (lambda (y) (+ x y))))
                 (define (g x) (((f (+ x 1))) (+ x 2)))
                 (g 3)""",
            8),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests2, comp=scheme_equal):
        return True


@test('22')
def problem_22(grades):
    scheme.scheme_eval = scheme.scheme_optimized_eval
    tests1 = [
        ("""(define (sum n total)
                   (if (zero? n) total
                     (sum (- n 1) (+ n total))))
                 (sum 1001 0)""",
            501501),
    ]
    tests2 = [
        ("""(define (sum n total)
                   (if (zero? n) total
                     (if #f 42 (sum (- n 1) (+ n total)))))
                 (sum 1001 0)""",
            501501),
    ]
    tests3 = [
        ("""(define (sum n total)
                   (cond ((zero? n) total)
                         ((zero? 0) (sum (- n 1) (+ n total)))
                         (else 42)))
                 (sum 1001 0)""",
            501501),
    ]
    tests4 = [
        ("""(define (sum n total)
                   (if (zero? n) total
                     (add n (+ n total))))
                 (define add (lambda (x+1 y) (sum (- x+1 1) y)))
                 (sum 1001 0)""",
            501501),
    ]
    tests5 = [
        ("""(define (sum n total)
                   (if (zero? n) total
                     (let ((n-1 (- n 1)))
                       (sum n-1 (+ n total)))))
                 (sum 1001 0)""",
            501501),
    ]
    if check_func(scheme_eval, tests1, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests2, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests3, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests4, comp=scheme_equal):
        return True
    if check_func(scheme_eval, tests5, comp=scheme_equal):
        return True

#############
# UTILITIES #
#############

def pairify(lst): # parify从函数的名字就可以听出，是将lst Pair化
    if not lst:
        return nil # 这个nil是全局唯一一份的nil
    if type(lst) is not list:
        return lst
    return Pair(pairify(lst[0]), pairify(lst[1:]))

def scheme_equal(x, y): # 判断scheme的两个值是否相等
    """Are Scheme values x and y equal, even if they use different classes?"""
    if hasattr(x, 'first') and hasattr(y, 'first'):
        return scheme_equal(x.first, y.first) and scheme_equal(x.second, y.second)
    if type(x).__name__ == 'nil' and type(y).__name__ == 'nil':
        return True
    elif type(x).__name__ == 'LambdaProcedure' and type(y).__name__ == 'LambdaProcedure':
        if not environments_equal(x.env, y.env):
            return False
        return scheme_equal(x.formals, y.formals) and scheme_equal(x.body, y.body)
    else:
        return x == y

def environments_equal(env1, env2): # 居然还有这样的一个函数，有意思
    """Are environments env1 and env2 equal, even using different classes?"""
    if type(env1).__name__ != 'Frame' or type(env2).__name__ != 'Frame':
        return False
    if env1.parent is None and env2.parent is None:
        # Assume that all global frames are the same
        return True
    if set(env1.bindings) != set(env2.bindings):
        # The two environments have different bindings
        return False
    for binding, value in env1.bindings.items():
        if not scheme_equal(value, env2.bindings[binding]):
            return False # If the values for the same bindings are different
    return environments_equal(env1.parent, env2.parent)

def catch_syntax_error(fn):
    def caught_syntax(*args):
        try:
            return fn(*args) # 先调用这个函数
        except SyntaxError: # 好吧，其实就是一个和装饰器差不多的玩意
            return 'Error'
    return caught_syntax

def scheme_eval(snippet): # snippet 是小片段的意思
    """Convert snippet into a single expression and scheme_eval it."""
    # TODO: figure out how to do this more cleanly
    # buffer_lines函数返回的是一个Buffer对象
    buf = scheme.buffer_lines(snippet.split('\n'), show_prompt=True)
    # scheme是一个模块
    exprs = [] # expressions 表达式是吧！哈哈！
    try:
        while True:
            exprs.append(scheme.scheme_read(buf))
    except EOFError:
        pass
    env = scheme.create_global_frame() # 构建一个全局的env，是吧！
    try:
        for expr in exprs[:-1]:
            scheme.scheme_eval(expr, env)
        return scheme.scheme_eval(exprs[-1], env)
    except scheme.SchemeError as err:
        return 'SchemeError'
    except BaseException as err:
        return type(err).__name__ + ' ' + str(err)

utils = """
(define (square x) (* x x))

(define (abs x)
  (cond ((> x 0) x)
        ((= x 0) 0)
        ((< x 0) (- x))))

(define (len s)
  (if (eq? s '())
    0
    (+ 1 (len (cdr s)))))

(define (equal? x y)
  (cond ((pair? x) (and (pair? y)
                        (equal? (car x) (car y))
                        (equal? (cdr x) (cdr y))))
        ((null? x) (null? y))
        (else (eq? x y))))

(define (map proc items)
  (if (null? items)
      nil
      (cons (proc (car items))
            (map proc (cdr items)))))

(define (filter predicate sequence)
  (cond ((null? sequence) nil)
        ((predicate (car sequence))
         (cons (car sequence)
               (filter predicate (cdr sequence))))
        (else (filter predicate (cdr sequence)))))

(define (accumulate op initial sequence)
  (if (null? sequence)
      initial
      (op (car sequence)
          (accumulate op initial (cdr sequence)))))

(define (combine f)
  (lambda (x y)
    (if (null? x) nil
      (f (list (car x) (car y))
         ((combine f) (cdr x) (cdr y))))))

(define (memq item x)
  (cond ((null? x) false)
        ((eq? item (car x)) x)
        (else (memq item (cdr x)))))

(define compose (lambda (f g) (lambda (x) (f (g x)))))

"""

def make_check_scheme(file='questions.scm'):
    """Check a Scheme question."""
    with open(file, 'r') as f:
        contents = utils + f.read()
    def check_scheme(snippet, preamble=''):
        stuff = contents + preamble + snippet
        return scheme_eval(stuff)
    return check_scheme

check_scheme = make_check_scheme()

##########################
# COMMAND LINE INTERFACE #
##########################

project_info = {
    'name': 'Project 4: Scheme',
    'remote_index': 'http://inst.eecs.berkeley.edu/~cs61a/fa13/proj/scheme/',
    'autograder_files': [
        'scheme_grader.py',
        'scheme_test.py',
        'autograder.py',
    ],
    'version': __version__,
}

@main
def run(*args):
    run_tests(**project_info)
