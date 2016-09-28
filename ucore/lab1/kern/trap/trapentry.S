#include <memlayout.h>

# vectors.S sends all traps here.
.text
.globl __alltraps
__alltraps: # 也就是说，所有的中断处理函数都会运行到这个函数，是吧！
    # push registers to build a trap frame
    # therefore make the stack look like a struct trapframe
    pushl %ds     # 猜测pushl是压入一个双字到栈里,可以跟着调试一把
    pushl %es
    pushl %fs
    pushl %gs
    pushal # pushal是压入通用寄存器的值到栈里
	# 错误码0
	# 中断号num
	# 扩展的ds
	# ...es
	# ...fs
	# ...gs
	# ...通用寄存器的值

    # load GD_KDATA into %ds and %es to set up data segments for kernel
	# GD_KDATA是个啥东西？
    movl $GD_KDATA, %eax # l应该是代表8个字节
    movw %ax, %ds # w应该是代表4个字节
    movw %ax, %es # 也就是说，ds和es指向同一个段

    # push %esp to pass a pointer to the trapframe as an argument to trap()
    pushl %esp # 为什么要压入esp，好吧，据说是要把esp当做一个参数传递过去

    # call trap(tf), where tf=%esp
    call trap # 我好奇的是，有没有将eip，ebp什么的压栈，不过貌似没有
	# 应该有吧？

	# 好吧，现在调用trap函数返回了。
    # pop the pushed stack pointer
	# 首先是恢复esp，是吧！
    popl %esp

    # return falls through to trapret...
.globl __trapret
__trapret:
    # restore registers from stack
    popal # 首先是恢复那些通用的寄存器

    # restore %ds, %es, %fs and %gs
    popl %gs
    popl %fs
    popl %es
    popl %ds

    # get rid of the trap number and error code
	# 错误码0
	# 中断号num
	# 扩展的ds
	# ...es
	# ...fs
	# ...gs
	# ...通用寄存器的值
    addl $0x8, %esp # esp移动8个单位，相当于去掉中断号以及错误码
    iret # 调用完了中断之后直接返回了。

