.text
.globl kernel_thread_entry
# 来看一下全局的kernel_thread_entry函数吧！
kernel_thread_entry:        # void kernel_thread(void)

    pushl %edx              # push arg # edx中存放了参数的地址
    call *%ebx              # call fn # ebx中存放了函数的地址

    pushl %eax              # save the return value of fn(arg)
    call do_exit            # call do_exit to terminate current thread

