"""Unit testing framework for the Scheme interpreter.

Usage: python3 scheme_test.py FILE

Interprets FILE as interactive Scheme source code, and compares each line
of printed output from the read-eval-print loop and from any output functions
to an expected output described in a comment.  For example,

(display (+ 2 3))
; expect 5

Differences between printed and expected outputs are printed with line numbers.
"""

import io
import sys
from buffer import Buffer
from scheme import read_eval_print_loop, create_global_frame
from scheme_tokens import tokenize_lines
from ucb import main

def summarize(output, expected_output):
    """Summarize results of running tests."""
    num_failed, num_expected = 0, len(expected_output)

    def failed(expected, actual, line): # 如果失败了，输出行号
        nonlocal num_failed # 失败的次数
        num_failed += 1
        print('test failed at line', line)
        print('  expected', expected)
        print('   printed', actual)

    for (actual, (expected, line_number)) in zip(output, expected_output):
        if expected.startswith("Error"):
            if not actual.startswith("Error"):
                failed('an error indication', actual, line_number)
        elif actual != expected:
            failed(expected, actual, line_number)
    print('{0} tested; {1} failed.'.format(num_expected, num_failed))

EXPECT_STRING = '; expect'

class TestReader:
    """A TestReader is an iterable that collects test case expected results."""
    def __init__(self, lines, stdout):
        self.lines = lines
        self.stdout = stdout
        self.last_out_len = 0
        self.output = []
        self.expected_output = []
        self.line_number = 0

    def __iter__(self):
        for line in self.lines:
            line = line.rstrip('\n') # rstrip用于去除末尾的换行符
            self.line_number += 1 # 行号加1
            if line.lstrip().startswith(EXPECT_STRING): # 首先去除行首的空格，然后判断是不是以', expect'开始
                expected = line.split(EXPECT_STRING, 1)[1][1:].split(' ; ') # 这玩意又开始切分了
                print("TestReader>> ", expected)
                for exp in expected:
                    print(exp)
                    self.expected_output.append((exp, self.line_number)) # 加入了exp，以及行号
                out_lines = self.stdout.getvalue().split('\n')
                if len(out_lines) > self.last_out_len:
                    self.output.extend(out_lines[-1-len(expected):-1])
                else:
                    self.output.extend([''] * len(expected))
                self.last_out_len = len(out_lines)
            yield line
        raise EOFError

@main
def run_tests(src_file='tests.scm'):
    """Run a read-eval loop that reads from src_file and collects outputs."""
    sys.stderr = sys.stdout = io.StringIO() # Collect output to stdout and stderr
    reader = None
    try:
        reader = TestReader(open(src_file).readlines(), sys.stdout)
        src = Buffer(tokenize_lines(reader))
        def next_line():
            src.current()
            return src
        read_eval_print_loop(next_line, create_global_frame())
    except BaseException as exc:
        sys.stderr = sys.__stderr__
        if reader:
            print("Tests terminated due to unhandled exception "
                  "after line {0}:\n>>>".format(reader.line_number),
                  file=sys.stderr)
        raise
    finally:
        sys.stdout = sys.__stdout__  # Revert stdout
        sys.stderr = sys.__stderr__  # Revert stderr
    summarize(reader.output, reader.expected_output)
