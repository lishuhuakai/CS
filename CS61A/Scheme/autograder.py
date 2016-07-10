"""Common utility functions for automatic grading."""

import sys, os, traceback
from doctest import DocTestFinder, DocTestRunner
from collections import namedtuple, defaultdict, OrderedDict
import urllib.request, urllib.error
import re
import argparse

Test = namedtuple('Test', ['name', 'fn']) # 这里定义了一种结构一样的东西
# 具体来说，就是名字是Test，然后有两个成员变量
TESTS = OrderedDict()
# OrderedDict是一个有顺序的字典

# set path for autograder to test current working directory
sys.path[0:0] = [ os.getcwd() ]
# 上面这一句的意思是，将当前的路径加入到sys.path下

def test(name):
    """Decorator to register a test. A test returns a true value on failure."""
    def new_fn(fn):
        TESTS[name] = Test(fn.__name__, fn) # 将name加入有名元组
        return fn
    return new_fn

def test_all(project_name, tests=TESTS):
    """Run all TESTS. Exits with a useful code: 0 for ok, 1 for problems."""
    for name in tests:
        test = tests[name]
        underline('Test {0}'.format(test.name))
        try:
            failure = test.fn(None)
        except Exception as inst:
            traceback.print_exc()
            failure = True
        if failure:
            sys.exit(1)
        print('All released tests passed')
        print()
    sys.exit(0)

class TimeoutError(Exception): # 果然是自己定义的错误
    _message = 'Evaluation timed out!'
    pass

TIMEOUT = 20
def test_eval(func, inputs, timeout=TIMEOUT, **kwargs):
    if type(inputs) is not tuple:
        inputs = (inputs,)
    result = timed(func, timeout, inputs, kwargs)
    return result

def timed(func, timeout, args=(), kwargs={}):
    """Calls FUNC with arguments ARGS and keyword arguments KWARGS. If it takes
    longer than TIMEOUT seconds to finish executing, a TimeoutError will be
    raised."""
    from threading import Thread
    class ReturningThread(Thread): # 妈蛋，这居然是一个类
        """Creates a daemon Thread with a result variable."""
        def __init__(self): # 构造函数
            Thread.__init__(self)
            self.daemon = True # 守护程序
            self.result = None
            self.error = None
        def run(self): # 子类覆写了父类的run方法！
            try:
                self.result = func(*args, **kwargs) # 调用函数func获得返回值
            except Exception as e: # 当然，如果抛出了异常，那么，要
                e._message = traceback.format_exc(limit=2)
                self.error = e # 记录下error
    submission = ReturningThread() # 构造一个类
    submission.start() # 这可不是什么开启线程，这是每个thread及其子类都必须要使用到一次的函数
    submission.join(timeout) # 相当于等待吧！
    if submission.is_alive(): # 已经超时了，所以超时错误
        raise TimeoutError
    if submission.error is not None: # func发生了错误，所以要raise这个错误
        raise submission.error
    return submission.result # 否则的话，返回结果

def check_func(func, tests,
               comp = lambda x, y: x == y,
               in_print = repr, out_print = repr):
    """Test FUNC according to sequence TESTS.  Each item in TESTS consists of
    (I, V, D=None), where I is a tuple of inputs to FUNC (if not a tuple,
    (I,) is substituted) and V is the proper output according to comparison
    COMP.  Prints erroneous cases.  In case of error, uses D as the test
    description, or constructs a description from I and V otherwise.
    Returns 0 for all correct, or the number of tests failed."""
    code = 0
    for input, output, *desc in tests:
        try:
            val = test_eval(func, input) # 用于测试，需要说明的是，test_eval相当于是另外起了一个线程
        except Exception as e: # test_eval可能会抛异常啊，这里要抓取异常
            fail_msg = "Function {0} failed".format(func.__name__)
            if desc:
                print(fail_msg, desc[0])
            else:
                print(fail_msg, "with input", in_print(input))
            print(e._message)
            code += 1
            continue
        if not comp(val, output): # 如果结果和测试的值不一致的话
            wrong_msg = "Wrong result from {0}:".format(func.__name__)
            if desc:
                print(wrong_msg, desc[0])
            else:
                print(wrong_msg, "input", in_print(input))
                print("   returned", out_print(val), "not", out_print(output))
            code += 1
    return code

def check_doctest(func_name, module, run=True): # func_name是函数的名字，module是模块的名字
    """Check that MODULE.FUNC_NAME doctest passes."""
    func = getattr(module, func_name) # 好吧，终于看到了这个玩意了。Java的反射机制是吧！
    tests = DocTestFinder().find(func)
    if not tests:
        print("No doctests found for " + func_name)
        return True
    fn = lambda: DocTestRunner().run(tests[0]) # 一个匿名函数
    result = test_eval(fn, tuple()) #
    if result.failed != 0:
        print("A doctest example failed for " + func_name + ".")
        return True
    return False

def underline(s):
    """Print string S, double underlined in ASCII."""
    print(s)
    print('='*len(s))

def check_for_updates(index, filenames, version):
    print('You are running version', version, 'of the autograder')
    try:
        remotes = {}
        for filename in filenames:
            path = os.path.join(index, filename)
            data = timed(urllib.request.urlopen, 1, args=(path,))
            remotes[filename] = data.read().decode('utf-8')
    except (urllib.error.URLError, urllib.error.HTTPError):
        print("Couldn't check remote autograder")
        return
    except TimeoutError:
        print("Checking for updates timed out.")
        return
    remote_version = re.search("__version__ = '(.*)'",
                               remotes[filenames[0]])
    if remote_version and remote_version.group(1) != version:
        print('Version', remote_version.group(1),
              'is available with new tests.')
        prompt = input('Do you want to automatically download these files? [y/n]: ')
        if 'y' in prompt.lower():
            for file in filenames:
                with open(file, 'w') as new:
                    new.write(remotes[file])
                    print('\t', file, 'updated')
            exit(0)
        else:
            print('You can download the new autograder from the following links:')
            for file in filenames:
                print('\t' + os.path.join(index, file))
            print()

def run_tests(name, remote_index, autograder_files, version, **kwargs):
    parser = argparse.ArgumentParser(description='Autograder for CS 61A.')
    parser.add_argument('-q', '--question', type=str,
                        help='Run tests for the specified question')
    parser.add_argument('-v', '--version', action='store_true',
                        help='Prints autograder version and exits')
    args = parser.parse_args()
    if args.question:
        args.question = args.question.upper()

    check_for_updates(remote_index, autograder_files, version)
    if args.version:
        exit(0)
    elif args.question in TESTS:
        tests = {args.question: TESTS[args.question]}
    else:
        tests = TESTS
    test_all(name, tests)
