#include <proc.h>
#include <kmalloc.h>
#include <string.h>
#include <sync.h>
#include <pmm.h>
#include <error.h>
#include <sched.h>
#include <elf.h>
#include <vmm.h>
#include <trap.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* ------------- process/thread mechanism design&implementation -------------
(an simplified Linux process/thread mechanism )
introduction:
  ucore implements a simple process/thread mechanism. process contains the independent memory sapce, at least one threads
for execution, the kernel data(for management), processor state (for context switch), files(in lab6), etc. ucore needs to
manage all these details efficiently. In ucore, a thread is just a special kind of process(share process's memory).
------------------------------
process state       :     meaning               -- reason
    PROC_UNINIT     :   uninitialized           -- alloc_proc
    PROC_SLEEPING   :   sleeping                -- try_free_pages, do_wait, do_sleep
    PROC_RUNNABLE   :   runnable(maybe running) -- proc_init, wakeup_proc, 
    PROC_ZOMBIE     :   almost dead             -- do_exit

-----------------------------
process state changing:
                                            
  alloc_proc                                 RUNNING
      +                                   +--<----<--+
      +                                   + proc_run +
      V                                   +-->---->--+ 
PROC_UNINIT -- proc_init/wakeup_proc --> PROC_RUNNABLE -- try_free_pages/do_wait/do_sleep --> PROC_SLEEPING --
                                           A      +                                                           +
                                           |      +--- do_exit --> PROC_ZOMBIE                                +
                                           +                                                                  + 
                                           -----------------------wakeup_proc----------------------------------
-----------------------------
process relations
parent:           proc->parent  (proc is children)
children:         proc->cptr    (proc is parent)
older sibling:    proc->optr    (proc is younger sibling)
younger sibling:  proc->yptr    (proc is older sibling)
-----------------------------
related syscall for process:
SYS_exit        : process exit,                           -->do_exit
SYS_fork        : create child process, dup mm            -->do_fork-->wakeup_proc
SYS_wait        : wait process                            -->do_wait
SYS_exec        : after fork, process execute a program   -->load a program and refresh the mm
SYS_clone       : create child thread                     -->do_fork-->wakeup_proc
SYS_yield       : process flag itself need resecheduling, -- proc->need_sched=1, then scheduler will rescheule this process
SYS_sleep       : process sleep                           -->do_sleep 
SYS_kill        : kill process                            -->do_kill-->proc->flags |= PF_EXITING
                                                                 -->wakeup_proc-->do_wait-->do_exit   
SYS_getpid      : get the process's pid

*/

// the process set's list
list_entry_t proc_list; // 进程的链表

#define HASH_SHIFT          10
#define HASH_LIST_SIZE      (1 << HASH_SHIFT)
#define pid_hashfn(x)       (hash32(x, HASH_SHIFT))

// has list for process set based on pid
static list_entry_t hash_list[HASH_LIST_SIZE];

// idle proc # 空闲的进程列表
struct proc_struct *idleproc = NULL;
// init proc
struct proc_struct *initproc = NULL;
// current proc
struct proc_struct *current = NULL;

static int nr_process = 0; // 进程的数目

void kernel_thread_entry(void);
void forkrets(struct trapframe *tf);
void switch_to(struct context *from, struct context *to);

// alloc_proc - alloc a proc_struct and init all fields of proc_struct
static struct proc_struct *
alloc_proc(void) {
    struct proc_struct *proc = kmalloc(sizeof(struct proc_struct));
    if (proc != NULL) {
    //LAB4:EXERCISE1 YOUR CODE
    /*
     * below fields in proc_struct need to be initialized
     *       enum proc_state state;                      // Process state # 进程状态
     *       int pid;                                    // Process ID # pid
     *       int runs;                                   // the running times of Proces
     *       uintptr_t kstack;                           // Process kernel stack
     *       volatile bool need_resched;                 // bool value: need to be rescheduled to release CPU?
     *       struct proc_struct *parent;                 // the parent process
     *       struct mm_struct *mm;                       // Process's memory management field
     *       struct context context;                     // Switch here to run process
     *       struct trapframe *tf;                       // Trap frame for current interrupt
     *       uintptr_t cr3;                              // CR3 register: the base addr of Page Directroy Table(PDT)
     *       uint32_t flags;                             // Process flag
     *       char name[PROC_NAME_LEN + 1];               // Process name
     */
		proc->state = PROC_UNINIT; // 表示还没有初始化,设置进程为“初始”态
		proc->pid = -1; // 设置进程pid的未初始化值
		proc->runs = 0; // 一次都没有执行
		proc->kstack = 0;
		proc->need_resched = 0;
		proc->parent = NULL;
		proc->mm = NULL;
		memset(&(proc->context), 0, sizeof(struct context));
		proc->tf = NULL;
		proc->cr3 = boot_cr3; // 使用内核页目录表的基址，正如我们所看得到的，这里一共有两个目录页
		proc->flags = 0;
		memset(proc->name, 0, PROC_NAME_LEN);
    }
    return proc;
}

// set_proc_name - set the name of proc
char *
set_proc_name(struct proc_struct *proc, const char *name) {
    memset(proc->name, 0, sizeof(proc->name));
    return memcpy(proc->name, name, PROC_NAME_LEN);
}

// get_proc_name - get the name of proc
char *
get_proc_name(struct proc_struct *proc) {
    static char name[PROC_NAME_LEN + 1];
    memset(name, 0, sizeof(name));
    return memcpy(name, proc->name, PROC_NAME_LEN);
}

// get_pid - alloc a unique pid for process
// 关于这个函数，不用想太多，只需要知道它会返回一个唯一的pid即可
static int
get_pid(void) {
    static_assert(MAX_PID > MAX_PROCESS);
    struct proc_struct *proc;
    list_entry_t *list = &proc_list, *le;
    static int next_safe = MAX_PID, last_pid = MAX_PID;
    if (++ last_pid >= MAX_PID) {
        last_pid = 1;
        goto inside;
    }
    if (last_pid >= next_safe) {
    inside:
        next_safe = MAX_PID;
    repeat:
        le = list;
        while ((le = list_next(le)) != list) {
            proc = le2proc(le, list_link);
            if (proc->pid == last_pid) {
                if (++ last_pid >= next_safe) {
                    if (last_pid >= MAX_PID) {
                        last_pid = 1;
                    }
                    next_safe = MAX_PID;
                    goto repeat;
                }
            }
            else if (proc->pid > last_pid && next_safe > proc->pid) {
                next_safe = proc->pid;
            }
        }
    }
    return last_pid;
}

// proc_run - make process "proc" running on cpu
// NOTE: before call switch_to, should load  base addr of "proc"'s new PDT

void
proc_run(struct proc_struct *proc) {
    if (proc != current) { // 要运行的进程不是当前的进程
        bool intr_flag;
        struct proc_struct *prev = current, *next = proc;
        local_intr_save(intr_flag); // 关闭中断
        {
            current = proc;
            load_esp0(next->kstack + KSTACKSIZE);
            lcr3(next->cr3); // 加载cr3寄存器中的值，实际上是完成了页表的切换
			// switch_to要干的事情我们想都想得到，那就是寄存器值的保存和切换，是吧！
            switch_to(&(prev->context), &(next->context));
        }
        local_intr_restore(intr_flag); // 开启中断
    }
}

// forkret -- the first kernel entry point of a new thread/process
// NOTE: the addr of forkret is setted in copy_thread function
//       after switch_to, the current proc will execute here.
static void
forkret(void) {
    forkrets(current->tf);
}

// hash_proc - add proc into proc hash_list
// 为什么要将进程加入proc hash_list?
static void
hash_proc(struct proc_struct *proc) {
    list_add(hash_list + pid_hashfn(proc->pid), &(proc->hash_link)); // 也就是加入表头
}

// find_proc - find proc frome proc hash_list according to pid
// 这个函数就是单纯地从proc hash_list中寻找pid对应的proc_struct
// 我从一个方面试图解释前面的问题，从这里来看，使用了hash之后，找到对应的进程变得比较快速了
struct proc_struct *
find_proc(int pid) {
    if (0 < pid && pid < MAX_PID) {
        list_entry_t *list = hash_list + pid_hashfn(pid), *le = list;
        while ((le = list_next(le)) != list) {
            struct proc_struct *proc = le2proc(le, hash_link);
            if (proc->pid == pid) {
                return proc;
            }
        }
    }
    return NULL;
}

// kernel_thread - create a kernel thread using "fn" function
// NOTE: the contents of temp trapframe tf will be copied to 
//       proc->tf in do_fork-->copy_thread function
// 使用fn函数构建一个内核线程
int
kernel_thread(int (*fn)(void *), void *arg, uint32_t clone_flags) {
    struct trapframe tf; // 首先要构建一个帧(trapframe)
    memset(&tf, 0, sizeof(struct trapframe));  // 先清空
    tf.tf_cs = KERNEL_CS; // 内核代码段，段选择子
    tf.tf_ds = tf.tf_es = tf.tf_ss = KERNEL_DS;
    tf.tf_regs.reg_ebx = (uint32_t)fn; // 函数地址放在ebx中
    tf.tf_regs.reg_edx = (uint32_t)arg; // 参数地址放在arg中
	// 需要说明一下的是，一旦线程开始运行，我们设置的这些存储器的值就真的在存储器里面了
    tf.tf_eip = (uint32_t)kernel_thread_entry; // 总之在运行进程之前，都要先到kernel_thread_entry走一遭，是吧！
	// 我想稍微说一下，这里的eip里面的地址对应的指令，并不是switch_to转到对应线程/进程之后立即执行的指令
	// 实际上，进程/线程在运行前，会将上下文中的保存的值恢复到寄存器中，获取cpu之后，会执行context.eip中的指令
	// 基本上返回之后才会用这里的tf_eip填充eip，开始执行相应的指令
    return do_fork(clone_flags | CLONE_VM, 0, &tf);
}

// setup_kstack - alloc pages with size KSTACKPAGE as process kernel stack
// 也需要分配栈，是吧！
static int
setup_kstack(struct proc_struct *proc) {
    struct Page *page = alloc_pages(KSTACKPAGE); // 栈的大小只有8KB
    if (page != NULL) {
        proc->kstack = (uintptr_t)page2kva(page); // kernel stack
        return 0;
    }
    return -E_NO_MEM; // 表示内存耗尽了
}

// put_kstack - free the memory space of process kernel stack
// 释放内核的内存空间
static void
put_kstack(struct proc_struct *proc) {
    free_pages(kva2page((void *)(proc->kstack)), KSTACKPAGE);
}

// copy_mm - process "proc" duplicate OR share process "current"'s mm according clone_flags
//         - if clone_flags & CLONE_VM, then "share" ; else "duplicate"
static int
copy_mm(uint32_t clone_flags, struct proc_struct *proc) {
    assert(current->mm == NULL);
    /* do nothing in this project */
    return 0;
}

// copy_thread - setup the trapframe on the  process's kernel stack top and
//             - setup the kernel entry point and stack of process
// 在内核线程的栈上建立trapframe,并且设定好kenel entry point 
static void
copy_thread(struct proc_struct *proc, uintptr_t esp, struct trapframe *tf) {
	// 在内核堆栈的顶部设置中断帧大小的一块栈空间
    proc->tf = (struct trapframe *)(proc->kstack + KSTACKSIZE) - 1;
    *(proc->tf) = *tf; // 拷贝在kernel_thread函数建立的临时中断帧的初始值
    proc->tf->tf_regs.reg_eax = 0; // 设置子进程/线程执行完do_fork后的返回值
	
    proc->tf->tf_esp = esp; // 设置中断帧中的栈指针栈
    proc->tf->tf_eflags |= FL_IF; // 使能中断
	// 下面用于设置进程的上下文，也称执行现场，只有设置好执行现场后，一旦ucore调度器选择了该进程/线程执行
	// 就需要根据context中保存的执行现场来恢复进程/线程的执行
	// 这里主要设置了两个信息:上次停止执行是的下一条指令地址context.eip
	// 上次停止执行是的堆栈地址context.esp
	// 但是其实这个进程/线程还没有执行过，所以这其实就是进程/线程实际执行的第一条指令地址和堆栈指针
	// 可以看出，由于线程/进程的中断帧占用了实际给进程/线程分配的栈空间的顶部，所以进程/线程就只能
	// 把栈顶指针context.esp设置在进程/线程的中断帧的起始位置
	// 根据context.eip的赋值，可以知道进程/线程实际开始执行的地方在forkret函数
	// 它主要完成do_fork函数的返回值的处理工作
    proc->context.eip = (uintptr_t)forkret; // 这里应该是线程运行完成后返回的地址
    proc->context.esp = (uintptr_t)(proc->tf);
}

/* do_fork -     parent process for a new child process
 * @clone_flags: used to guide how to clone the child process
 * @stack:       the parent's user stack pointer. if stack==0, It means to fork a kernel thread.
 * @tf:          the trapframe info, which will be copied to child process's proc->tf
 */
// 我们来看一下do_fork函数是如何来运行的吧！
int
do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf) {
    int ret = -E_NO_FREE_PROC;
    struct proc_struct *proc;
    if (nr_process >= MAX_PROCESS) { // 线程数目已经到达了极限
        goto fork_out;
    }
    ret = -E_NO_MEM;
    //LAB4:EXERCISE2 YOUR CODE
    /*
     * Some Useful MACROs, Functions and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   alloc_proc:   create a proc struct and init fields (lab4:exercise1)
     *   setup_kstack: alloc pages with size KSTACKPAGE as process kernel stack
     *   copy_mm:      process "proc" duplicate OR share process "current"'s mm according clone_flags
     *                 if clone_flags & CLONE_VM, then "share" ; else "duplicate"
     *   copy_thread:  setup the trapframe on the  process's kernel stack top and
     *                 setup the kernel entry point and stack of process
     *   hash_proc:    add proc into proc hash_list
     *   get_pid:      alloc a unique pid for process
     *   wakeup_proc:  set proc->state = PROC_RUNNABLE
     * VARIABLES:
     *   proc_list:    the process set's list
     *   nr_process:   the number of process set
     */

    //    1. call alloc_proc to allocate a proc_struct
    //    2. call setup_kstack to allocate a kernel stack for child process
    //    3. call copy_mm to dup OR share mm according clone_flag
    //    4. call copy_thread to setup tf & context in proc_struct
    //    5. insert proc_struct into hash_list && proc_list
    //    6. call wakeup_proc to make the new child process RUNNABLE
    //    7. set ret vaule using child proc's pid

	// 这里构建的貌似都是内核线程，因为alloc_proc()函数的初始值使得段寄存器都指向了内核
	if ((proc = alloc_proc()) == NULL) { // alloc_proc()就是返回一个proc_struct结构体
		goto fork_out;
	}

	proc->parent = current; // 父进程

	if (setup_kstack(proc) != 0) { // 就是分配栈空间啦
		goto bad_fork_cleanup_kstack;
	}

	if (copy_mm(clone_flags, proc) != 0) { // 在这个project里面是什么也不干的
		goto bad_fork_cleanup_kstack;
	}
	// 注意一下这个stack,如果是从kernel_thread函数调用的话，stack=0
	copy_thread(proc, stack, tf);

	bool intr_flag;
	local_intr_save(intr_flag); // 关闭中断
	{
		proc->pid = get_pid(); // 得到一个独一无二的pid
		hash_proc(proc); // 将这个proc挂到hash数组里面去
		list_add(&proc_list, &(proc->list_link)); // proc_list是干什么用的?这里我要说一声，因为线程/进程调度的时候，一般会到proc_list中去找对应的进程/线程
		nr_process++;
	}
	local_intr_restore(intr_flag); // 开启中断

	wakeup_proc(proc); // 主要是将这个进程标记为可以运行，至于分配cpu时间，要到schedule函数才行
	ret = proc->pid; 

fork_out:
    return ret;

bad_fork_cleanup_kstack:
    put_kstack(proc);
bad_fork_cleanup_proc:
    kfree(proc);
    goto fork_out;
}

// do_exit - called by sys_exit
//   1. call exit_mmap & put_pgdir & mm_destroy to free the almost all memory space of process
//   2. set process' state as PROC_ZOMBIE, then call wakeup_proc(parent) to ask parent reclaim itself.
//   3. call scheduler to switch to other process
int
do_exit(int error_code) {
    panic("process exit!!.\n");
}

// init_main - the second kernel thread used to create user_main kernel threads
static int
init_main(void *arg) {
    cprintf("this initproc, pid = %d, name = \"%s\"\n", current->pid, get_proc_name(current));
    cprintf("To U: \"%s\".\n", (const char *)arg);
    cprintf("To U: \"en.., Bye, Bye. :)\"\n");
    return 0;
}

// proc_init - set up the first kernel thread idleproc "idle" by itself and 
//           - create the second kernel thread init_main
// 用于初始化线程
void
proc_init(void) {
    int i;

    list_init(&proc_list); // proc_list是用来干什么的？
    for (i = 0; i < HASH_LIST_SIZE; i ++) {
        list_init(hash_list + i); // hash_list就是一个链表数组
    }

    if ((idleproc = alloc_proc()) == NULL) { // alloc_proc()函数主要是返回一个动态分配的proc_struct
        panic("cannot alloc idleproc.\n");
    }

	// 第0号线程是idleproc
    idleproc->pid = 0; // 有意思，很少看见为0的pid啦
    idleproc->state = PROC_RUNNABLE; // 可以运行的进程
    idleproc->kstack = (uintptr_t)bootstack;  // 设置了内核栈的起始位置
    idleproc->need_resched = 1; // 表示需要被换出，让别的线程/进程执行
    set_proc_name(idleproc, "idle");
    nr_process ++; // 线程计数器加1

    current = idleproc; // 当前运行的线程

    int pid = kernel_thread(init_main, "Hello world!!", 0); // 这是第1号线程，居然去执行init_main函数了
	// 之所以叫线程，因为这些内核线程共享代码，数据等一系列的东西，只有堆栈不同而已
	// 进程的话，数据段，代码段什么的都不一样
    if (pid <= 0) {
        panic("create init_main failed.\n");
    }

    initproc = find_proc(pid);
    set_proc_name(initproc, "init");

    assert(idleproc != NULL && idleproc->pid == 0);
    assert(initproc != NULL && initproc->pid == 1);
}

// cpu_idle - at the end of kern_init, the first kernel thread idleproc will do below works
void
cpu_idle(void) {
    while (1) {
        if (current->need_resched) { 
            schedule();
        }
    }
}

