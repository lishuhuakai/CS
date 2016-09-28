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
#include <unistd.h>

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
     //LAB5 YOUR CODE : (update LAB4 steps)
    /*
     * below fields(add in LAB5) in proc_struct need to be initialized	
     *       uint32_t wait_state;                        // waiting state
     *       struct proc_struct *cptr, *yptr, *optr;     // relations between processes
	 */
     //LAB6 YOUR CODE : (update LAB5 steps)
    /*
     * below fields(add in LAB6) in proc_struct need to be initialized
     *     struct run_queue *rq;                       // running queue contains Process
     *     list_entry_t run_link;                      // the entry linked in run queue
     *     int time_slice;                             // time slice for occupying the CPU
     *     skew_heap_entry_t lab6_run_pool;            // FOR LAB6 ONLY: the entry in the run pool
     *     uint32_t lab6_stride;                       // FOR LAB6 ONLY: the current stride of the process
     *     uint32_t lab6_priority;                     // FOR LAB6 ONLY: the priority of process, set by lab6_set_priority(uint32_t)
     */
		proc->state = PROC_UNINIT; // 处于未初始化的状态
		proc->pid = -1; // 表示还未分配身份信息
		proc->runs = 0; // 已经运行的次数
		proc->kstack = 0; 
		proc->need_resched = 0;
		proc->parent = NULL;
		proc->mm = NULL;
		memset(&(proc->context), 0, sizeof(struct context));
		proc->tf = NULL;
		proc->cr3 = boot_cr3;
		proc->flags = 0;
		memset(proc->name, 0, PROC_NAME_LEN);
		proc->wait_state = 0;
		proc->cptr = proc->optr = proc->yptr = NULL;
		proc->rq = NULL; // rq是个什么玩意？
		list_init(&(proc->run_link));
		proc->time_slice = 0; // 时间片
		proc->lab6_run_pool.left = proc->lab6_run_pool.right = proc->lab6_run_pool.parent = NULL;
		proc->lab6_stride = 0;
		proc->lab6_priority = 0; // 优先级
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

// set_links - set the relation links of process
static void
set_links(struct proc_struct *proc) {
    list_add(&proc_list, &(proc->list_link)); // 将这个proc加到proc_list的头部
    proc->yptr = NULL; // proc就是最年轻的
    if ((proc->optr = proc->parent->cptr) != NULL) { // proc的老兄弟现在变成了父节点子线程链表的头部
        proc->optr->yptr = proc;
    }
    proc->parent->cptr = proc; // 并且要将proc插入父进程子线程链表的头部
    nr_process ++; // 进程计数加1
}

// remove_links - clean the relation links of process
static void
remove_links(struct proc_struct *proc) {
    list_del(&(proc->list_link));
    if (proc->optr != NULL) { // old 的兄弟
        proc->optr->yptr = proc->yptr; // younger 的兄弟 
    }
    if (proc->yptr != NULL) {
        proc->yptr->optr = proc->optr;
    }
    else { // proc是最年轻的兄弟
       proc->parent->cptr = proc->optr; // cptr指的是child构成的链表
    }
    nr_process --;
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
	//cprintf("\n==> proc_run\n");
    if (proc != current) { // 要运行的进程不是当前的进程
		// 我发现一个很有意思的问题，那就是时间片好像并没有重新设置
        bool intr_flag;
        struct proc_struct *prev = current, *next = proc;
        local_intr_save(intr_flag); // 关闭中断
        {
            current = proc;
            load_esp0(next->kstack + KSTACKSIZE);
            lcr3(next->cr3); // 加载cr3寄存器中的值，实际上是完成了页表的切换
			// switch_to要干的事情我们想都想得到，那就是寄存器值的保存和切换，是吧！
            switch_to(&(prev->context), &(next->context));
			//cprintf("\n<== switch_to\n");
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

// unhash_proc - delete proc from proc hash_list
static void
unhash_proc(struct proc_struct *proc) {
    list_del(&(proc->hash_link)); // 也就是将这个proc从hansh_list中移除
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
	//cprintf("\n==> kernel_thread\n");
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

	// CLONE_VM表示只是线程而已，也就是和父进程共享memory manage结构
    return do_fork(clone_flags | CLONE_VM, 0, &tf);
}

// setup_kstack - alloc pages with size KSTACKPAGE as process kernel stack
// 也需要分配栈，是吧！
static int
setup_kstack(struct proc_struct *proc) {
	//cprintf("\n==> setup_kstack\n");
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

// setup_pgdir - alloc one page as PDT
// 每个进程都有一个以及页表
static int
setup_pgdir(struct mm_struct *mm) {
	//cprintf("\n==> setup_pgdir\n");
    struct Page *page;
    if ((page = alloc_page()) == NULL) { // 先分配一个页
        return -E_NO_MEM;
    }
    pde_t *pgdir = page2kva(page); // 根据这个页得到虚拟地址
    memcpy(pgdir, boot_pgdir, PGSIZE); // 复制的是内核的PDT
    pgdir[PDX(VPT)] = PADDR(pgdir) | PTE_P | PTE_W; // 为什么一级页表会有一项指向自己？
    mm->pgdir = pgdir; // 记录下这张一级页表
	//cprintf("\n<== setup_pgdir\n");
    return 0;
}

// put_pgdir - free the memory space of PDT
static void
put_pgdir(struct mm_struct *mm) {
	//cprintf("\n-----put_pgdir-----\n");
    free_page(kva2page(mm->pgdir)); // 连以及页表所在的页都要释放
}

// copy_mm - process "proc" duplicate OR share process "current"'s mm according clone_flags
//         - if clone_flags & CLONE_VM, then "share" ; else "duplicate"
// 我们来看一下copy_mm函数究竟干了一些什么吧！
static int
copy_mm(uint32_t clone_flags, struct proc_struct *proc) {
	//cprintf("\n==> copy_mm\n");
    struct mm_struct *mm, *oldmm = current->mm;

    /* current is a kernel thread */
    if (oldmm == NULL) { // oldmm为空的话，表示是一个内核线程
        return 0;
    }
    if (clone_flags & CLONE_VM) { // 表示直接复制，也就是共享吧！
        mm = oldmm;
        goto good_mm;
    }

    int ret = -E_NO_MEM;
    if ((mm = mm_create()) == NULL) {
        goto bad_mm;
    }
    if (setup_pgdir(mm) != 0) { // setup_pgdir主要是从内核拷贝一张一级页表
        goto bad_pgdir_cleanup_mm;
    }

    lock_mm(oldmm);
    {
        ret = dup_mmap(mm, oldmm); // 到这里的话，说明oldmm不为空,这里不单单是简单地地复制了一下oldmm的vma链表到mm
		// 同时也复制了数据
    }
    unlock_mm(oldmm);

    if (ret != 0) {
        goto bad_dup_cleanup_mmap;
    }

good_mm:
    mm_count_inc(mm); // 对这个memeory manage的引用加1
    proc->mm = mm;
    proc->cr3 = PADDR(mm->pgdir);
    return 0;
bad_dup_cleanup_mmap:
    exit_mmap(mm);
    put_pgdir(mm);
bad_pgdir_cleanup_mm:
    mm_destroy(mm);
bad_mm:
    return ret;
}

// copy_thread - setup the trapframe on the  process's kernel stack top and
//             - setup the kernel entry point and stack of process
// 在内核线程的栈上建立trapframe,并且设定好kenel entry point 
static void
copy_thread(struct proc_struct *proc, uintptr_t esp, struct trapframe *tf) {
	//cprintf("\n==> copy_thread\n");
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
    proc->context.eip = (uintptr_t)forkret;
    proc->context.esp = (uintptr_t)(proc->tf);
	//cprintf("\n<== copy_thread\n");
}

/* do_fork -     parent process for a new child process
 * @clone_flags: used to guide how to clone the child process
 * @stack:       the parent's user stack pointer. if stack==0, It means to fork a kernel thread.
 * @tf:          the trapframe info, which will be copied to child process's proc->tf
 */
// 我们来看一下do_fork函数是如何来运行的吧！
int
do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf) {
	//cprintf("\n==> do_fork\n");
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

	//LAB5 YOUR CODE : (update LAB4 steps)
   /* Some Functions
    *    set_links:  set the relation links of process.  ALSO SEE: remove_links:  lean the relation links of process 
    *    -------------------
	*    update step 1: set child proc's parent to current process, make sure current process's wait_state is 0
	*    update step 5: insert proc_struct into hash_list && proc_list, set the relation links of process
    */
	if ((proc = alloc_proc()) == NULL) { // alloc_proc()仅仅是分配了一个空间而已
		goto fork_out;
	}
	proc->parent = current; // 父进程
	assert(current->wait_state == 0);

	if (setup_kstack(proc) != 0) { // 就是分配栈空间,好吧，我们可以看到，所有的线程都有自己的栈空间
		goto bad_fork_cleanup_proc;
	}
	//cprintf("\n<== setup_kstack\n");

	if (copy_mm(clone_flags, proc) != 0) { // 拷贝一下进程的数据，如果是内核线程的话，什么也不干
		// 如果clone_flags 中包含 CLONE_VM的话，就将原来缓存的oldmm放入proc的mm域
		// 否则的话，所有的数据要从原来的oldmm复制，包括oldmm的vma
		goto bad_fork_cleanup_kstack;
	}
	//cprintf("\n<== copy_mm\n");

	copy_thread(proc, stack, tf);

	bool intr_flag;
	local_intr_save(intr_flag); // 关闭中断
	{
		proc->pid = get_pid(); // 得到一个独一无二的pid
		hash_proc(proc); // 将这个proc挂到hash数组里面去
		set_links(proc);
	}
	local_intr_restore(intr_flag); // 开启中断
	
	wakeup_proc(proc); // 主要是将这个进程标记为可以运行，至于分配cpu时间，要到schedule函数才行
	ret = proc->pid;
	//cprintf("\n<== do_fork\n");
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
	//cprintf("\n==> do_exit\n");
    if (current == idleproc) {
        panic("idleproc exit.\n");
    }
	assert(current != initproc);
    if (current == initproc) { // 为什么会这样？
        panic("initproc exit.\n");
    }
    
    struct mm_struct *mm = current->mm;
    if (mm != NULL) {
        lcr3(boot_cr3); // 切换到内核态的页表上
        if (mm_count_dec(mm) == 0) { // 引用这个mm结构的线程数目减1
            exit_mmap(mm); // 注销这些空间
            put_pgdir(mm);
            mm_destroy(mm); // 这个mm管理的资源全部都释放干净了
        }
        current->mm = NULL;
    }
    current->state = PROC_ZOMBIE; // 僵尸进程
    current->exit_code = error_code; // 记录下退出码，以便父进程查看
    
    bool intr_flag;
    struct proc_struct *proc;
    local_intr_save(intr_flag); // 关闭中断
    {
        proc = current->parent; // 得到父进程
        if (proc->wait_state == WT_CHILD) 
		{ // 等待子进程退出
            wakeup_proc(proc); // 主要是将父进程标记为可以运行,你也可以认为是唤醒父进程吧。
        }
        while (current->cptr != NULL) 
		{ // 如果该线程/进程还有子线程
			// 就不断循环，将这些子线程的父线程改为initproc
            proc = current->cptr; // proc指向子线程
            current->cptr = proc->optr; // 选取一个作为父节点
    
            proc->yptr = NULL;
            if ((proc->optr = initproc->cptr) != NULL)  // 下面代码要干的事情就是将proc加入initproc的子线程列表
			{   
				initproc->cptr->yptr = proc;
            }
            proc->parent = initproc; // 将proc的父进程标记为initproc
            initproc->cptr = proc;
            if (proc->state == PROC_ZOMBIE) 
			{
                if (initproc->wait_state == WT_CHILD) 
				{
                    wakeup_proc(initproc);
                }
            }
        }
    }
    local_intr_restore(intr_flag);
    
    schedule();
    panic("do_exit will not return!! %d.\n", current->pid);
}

/* load_icode - load the content of binary program(ELF format) as the new content of current process
 * @binary:  the memory addr of the content of binary program
 * @size:  the size of the content of binary program
 */
// 加载elf格式二进制程序最为当前线程的内容
static int
load_icode(unsigned char *binary, size_t size) {
	//cprintf("\n==> load_icode\n");
    if (current->mm != NULL) { // 也就是说只有内核线程才能够加载，用户进程别多事，因为内核线程的mm都为NULL
        panic("load_icode: current->mm must be empty.\n");
    }

    int ret = -E_NO_MEM;
    struct mm_struct *mm;
    //(1) create a new mm for current process
	// 首先为当前的进程构建一个新的memory manage
    if ((mm = mm_create()) == NULL) {
        goto bad_mm;
    }
    //(2) create a new PDT, and mm->pgdir= kernel virtual addr of PDT
	// 居然还要构建一个新的page directory table
    if (setup_pgdir(mm) != 0) { // 貌似setup_pgdir只是拷贝了一下内核的一级页表,因为内核部分的数据是共享的
        goto bad_pgdir_cleanup_mm;
    }
    //(3) copy TEXT/DATA section, build BSS parts in binary to memory space of process
	// 拷贝代码段，数据段，构建BSS...
    struct Page *page;
    //(3.1) get the file header of the bianry program (ELF format)
	// 首先是读取elf头部的信息
    struct elfhdr *elf = (struct elfhdr *)binary; 
    //(3.2) get the entry of the program section headers of the bianry program (ELF format)
	// 好吧，又要扯到e_phoff了，这个域和链接和执行有关
	// start of program headers
    struct proghdr *ph = (struct proghdr *)(binary + elf->e_phoff);
    //(3.3) This program is valid?
	// 判断程序是否有效
    if (elf->e_magic != ELF_MAGIC) {
        ret = -E_INVAL_ELF;
        goto bad_elf_cleanup_pgdir;
    }

    uint32_t vm_flags, perm;
    struct proghdr *ph_end = ph + elf->e_phnum;
	// e_phnum - number of program headers
    for (; ph < ph_end; ph ++) {
		//(3.4) find every program section headers
        if (ph->p_type != ELF_PT_LOAD) { // 只需要找到能够加载的segment
            continue ;
        }
        if (ph->p_filesz > ph->p_memsz) { // 纯粹地判断一下，防止异常的情况
            ret = -E_INVAL_ELF;
            goto bad_cleanup_mmap;
        }
        if (ph->p_filesz == 0) {
            continue ;
        }
		//(3.5) call mm_map fun to setup the new vma ( ph->p_va, ph->p_memsz)
		// 又要开始映射啦！
        vm_flags = 0, perm = PTE_U;
        if (ph->p_flags & ELF_PF_X) vm_flags |= VM_EXEC;
        if (ph->p_flags & ELF_PF_W) vm_flags |= VM_WRITE;
        if (ph->p_flags & ELF_PF_R) vm_flags |= VM_READ;
        if (vm_flags & VM_WRITE) perm |= PTE_W;
		// 我们去看一下mm_mmap函数到底干了什么事情吧！
		// mm_map确实没有干什么事情,只是将vma添加进了mm结构中而已
		// p_va是进程虚拟地址空间的起始位置
		// p_memsz是segment在虚拟地址空间中所占用的长度
        if ((ret = mm_map(mm, ph->p_va, ph->p_memsz, vm_flags, NULL)) != 0) {
            goto bad_cleanup_mmap;
        }
        unsigned char *from = binary + ph->p_offset; // 得到段在文件中的偏移
        size_t off, size;
		uintptr_t start = ph->p_va; // start是虚拟地址的起始地址
		uintptr_t end;
		uintptr_t la = ROUNDDOWN(start, PGSIZE); // la一般比start要小，因为la是start向下取整得到的

        ret = -E_NO_MEM;

		//(3.6) alloc memory, and  copy the contents of every program section (from, from+end) to process's memory (la, la+end)
        end = ph->p_va + ph->p_filesz; // 进程的虚拟地址的尾部地址
		//(3.6.1) copy TEXT/DATA section of bianry program
		// 拷贝数据段以及代码段
        while (start < end) { // 要开始拷贝了
            if ((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL) { // 首先为将la映射到新分配的页中
                goto bad_cleanup_mmap;
            }
			// 还有，从指导书上我们可以知道，代码段开始的地方是0x800020，其实并不是4KB对齐的
			// 而分配的页绝对是4KB对齐的，所以两者出现一个差值off
			// start是代码开始的地方，而la是页开始的地方.假设start = 0x800020,而la = 0x800000，所以off = 0x20
			off = start - la; 
			size = PGSIZE - off; // size表示要复制的大小,比如上面的，第一次要复制的大小为4096-0x20，如果出现有那么大的话
			la += PGSIZE; // 往内存中写自然是一页一页地写
            if (end < la) { // 如果剩余的字节不足一页
                size -= la - end; //获得实际要拷贝的大小
            }
			// 为了将代码拷贝到正确的位置，所以要添加上这个off,继续上面的page2kva(page)返回0x800000,肯定要加上0x20才行
            memcpy(page2kva(page) + off, from, size); // 从from处拷贝size个字节到对应的虚拟地址去
            start += size, from += size; // 不断后移
        }

		//(3.6.2) build BSS section of binary program
		// 构建bss section
        end = ph->p_va + ph->p_memsz; // memsz是进程在虚拟地址空间中所占用的长度
        if (start < la) {
            /* ph->p_memsz == ph->p_filesz */
            if (start == end) { // 也就是没有bss section，自然返回
                continue ;
            }
            off = start + PGSIZE - la, size = PGSIZE - off;
            if (end < la) {
                size -= la - end;
            }
            memset(page2kva(page) + off, 0, size); // 清空
            start += size;
            assert((end < la && start == end) || (end >= la && start == la));
        }
        while (start < end) {
            if ((page = pgdir_alloc_page(mm->pgdir, la, perm)) == NULL) { // 重新分配一个页
                goto bad_cleanup_mmap;
            }
            off = start - la, size = PGSIZE - off, la += PGSIZE;
            if (end < la) {
                size -= la - end;
            }
            memset(page2kva(page) + off, 0, size); // 一样地拷贝
            start += size;
        }
    }
    //(4) build user stack memory
	// 构建用户栈
    vm_flags = VM_READ | VM_WRITE | VM_STACK;
    if ((ret = mm_map(mm, USTACKTOP - USTACKSIZE, USTACKSIZE, vm_flags, NULL)) != 0) { // 构建了一个栈段
        goto bad_cleanup_mmap;
    }
	// 分配4个页
	// 通过pgdir_alloc_page分配的页一般都被标记为可以换出
	// 也就是先分配了，防止缺页中断
    assert(pgdir_alloc_page(mm->pgdir, USTACKTOP-PGSIZE , PTE_USER) != NULL);
    assert(pgdir_alloc_page(mm->pgdir, USTACKTOP-2*PGSIZE , PTE_USER) != NULL);
    assert(pgdir_alloc_page(mm->pgdir, USTACKTOP-3*PGSIZE , PTE_USER) != NULL);
    assert(pgdir_alloc_page(mm->pgdir, USTACKTOP-4*PGSIZE , PTE_USER) != NULL);
    
    //(5) set current process's mm, sr3, and set CR3 reg = physical addr of Page Directory
    mm_count_inc(mm);
    current->mm = mm; // memory manage
    current->cr3 = PADDR(mm->pgdir); // 好吧，将来这个玩意是要写入cr3寄存器的
    lcr3(PADDR(mm->pgdir)); // 好吧，直接就加载了,更新了用户进程的虚拟内存空间

    //(6) setup trapframe for user environment
    struct trapframe *tf = current->tf; // 大部分trapframe都和内核的一致，所以直接复制
    memset(tf, 0, sizeof(struct trapframe));
    /* LAB5:EXERCISE1 YOUR CODE
     * should set tf_cs,tf_ds,tf_es,tf_ss,tf_esp,tf_eip,tf_eflags
     * NOTICE: If we set trapframe correctly, then the user level process can return to USER MODE from kernel. So
     *          tf_cs should be USER_CS segment (see memlayout.h)
     *          tf_ds=tf_es=tf_ss should be USER_DS segment
     *          tf_esp should be the top addr of user stack (USTACKTOP)
     *          tf_eip should be the entry point of this binary program (elf->e_entry)
     *          tf_eflags should be set to enable computer to produce Interrupt
     */
	tf->tf_cs = USER_CS; // 用户代码段选择子
	tf->tf_ds = tf->tf_es = tf->tf_ss = USER_DS;
	tf->tf_esp = USTACKTOP;
	tf->tf_eip = elf->e_entry; // 程序的入口地址
	tf->tf_eflags = FL_IF; // 开启中断
	//cprintf("\n<== load_icode\n");
    ret = 0;
out:
    return ret;
bad_cleanup_mmap:
    exit_mmap(mm);
bad_elf_cleanup_pgdir:
    put_pgdir(mm);
bad_pgdir_cleanup_mm:
    mm_destroy(mm);
bad_mm:
    goto out;
}

// do_execve - call exit_mmap(mm)&put_pgdir(mm) to reclaim memory space of current process
//           - call load_icode to setup new memory space accroding binary prog.
// 这里应该是整个实验中最关键的一个部分
int
do_execve(const char *name, size_t len, unsigned char *binary, size_t size) {
	//cprintf("\n==> do_execve\n");
    struct mm_struct *mm = current->mm; // 我记得execv是直接使用当前进程的一些资源
    if (!user_mem_check(mm, (uintptr_t)name, len, 0)) {
        return -E_INVAL;
    }
    if (len > PROC_NAME_LEN) {
        len = PROC_NAME_LEN;
    }

    char local_name[PROC_NAME_LEN + 1];
    memset(local_name, 0, sizeof(local_name));
    memcpy(local_name, name, len);

    if (mm != NULL) {
        lcr3(boot_cr3);
		// 释放调用函数的资源
        if (mm_count_dec(mm) == 0) {
            exit_mmap(mm);
            put_pgdir(mm);
            mm_destroy(mm);
        }
        current->mm = NULL; // 要重新分配资源
    }
    int ret;
    if ((ret = load_icode(binary, size)) != 0) { // load_icode是用于加载用户的程序
        goto execve_exit;
    }
	//cprintf("\n==> do_execve ==> set_proc_name\n");
    set_proc_name(current, local_name);
	//cprintf("\n<== do_execve\n");
    return 0;

execve_exit:
    do_exit(ret);
    panic("already exit: %e.\n", ret);
}

// do_yield - ask the scheduler to reschedule
int
do_yield(void) {
    current->need_resched = 1;
    return 0;
}

// do_wait - wait one OR any children with PROC_ZOMBIE state, and free memory space of kernel stack
//         - proc struct of this child.
// NOTE: only after do_wait function, all resources of the child proces are free.

// 等待子进程退出，并且释放子进程的资源
int
do_wait(int pid, int *code_store) {
	//cprintf("\n==> do_wait\n");
    struct mm_struct *mm = current->mm;
    if (code_store != NULL) {
        if (!user_mem_check(mm, (uintptr_t)code_store, sizeof(int), 1)) {
            return -E_INVAL;
        }
    }

    struct proc_struct *proc;
    bool intr_flag, haskid;
repeat:
    haskid = 0;
    if (pid != 0) {
        proc = find_proc(pid);
        if (proc != NULL && proc->parent == current) {
            haskid = 1; // 存在孩子
            if (proc->state == PROC_ZOMBIE) { // 处于僵尸状态
                goto found;
            }
        }
    }
    else { // pid = 0表示只要有一个孩子返回了就可以了
        proc = current->cptr;
        for (; proc != NULL; proc = proc->optr) { // optr是兄弟线程
            haskid = 1;
            if (proc->state == PROC_ZOMBIE) {
                goto found;
            }
        }
    }
    if (haskid) { // 运行到了这一步的话，说明没有子线程是僵尸状态
        current->state = PROC_SLEEPING; // 陷入沉睡状态
        current->wait_state = WT_CHILD; // 等待孩子退出
		//cprintf("\n+ all kids are not zombie, so should give up cpu.\n");
		//cprintf("\n==> do_fork ==> schedule\n");
        schedule(); 
        if (current->flags & PF_EXITING) {
            do_exit(-E_KILLED);
        }
        goto repeat;
    }
	// 到这里的话，说明已经没有子线程了
    return -E_BAD_PROC;

found:
    if (proc == idleproc || proc == initproc) {
        panic("wait idleproc or initproc.\n");
    }
    if (code_store != NULL) { // 记录下返回的值
        *code_store = proc->exit_code;
    }
    local_intr_save(intr_flag); // 关闭中断
    {
        unhash_proc(proc);
        remove_links(proc);
    }
    local_intr_restore(intr_flag); // 开启中断
    put_kstack(proc); // 释放内核栈所占用的空间
    kfree(proc); // 测试释放资源
    return 0;
}

// do_kill - kill process with pid by set this process's flags with PF_EXITING
// 向进程发送kill命令
int
do_kill(int pid) {
	//cprintf("\n==> do_kill\n");
    struct proc_struct *proc;
    if ((proc = find_proc(pid)) != NULL) { // 根据pid找到一个proc
        if (!(proc->flags & PF_EXITING)) {
            proc->flags |= PF_EXITING; // 添加上标记
            if (proc->wait_state & WT_INTERRUPTED) { // 可以被打断的中断状态
                wakeup_proc(proc); // 将这个proc标记为可以运行
            }
            return 0;
        }
        return -E_KILLED;
    }
    return -E_INVAL;
}

// kernel_execve - do SYS_exec syscall to exec a user program called by user_main kernel_thread
static int
kernel_execve(const char *name, unsigned char *binary, size_t size) {
	//cprintf("\n==> kernel_execve\n");
    int ret, len = strlen(name); // name是用户程序的名字,binary是程序的起始地址
    asm volatile (
        "int %1;"
        : "=a" (ret)
        : "i" (T_SYSCALL), "0" (SYS_exec), "d" (name), "c" (len), "b" (binary), "D" (size)
        : "memory");
	// 这里发送中断信号T_SYSCALL
	//cprintf("\n<== kernel_execve\n");
    return ret;
}

// binary指的是程序开始的地址，虚拟地址
#define __KERNEL_EXECVE(name, binary, size) ({                          \
            cprintf("kernel_execve: pid = %d, name = \"%s\".\n",        \
                    current->pid, name);                                \
            kernel_execve(name, binary, (size_t)(size));                \
        })

#define KERNEL_EXECVE(x) ({                                             \
            extern unsigned char _binary_obj___user_##x##_out_start[],  \
                _binary_obj___user_##x##_out_size[];                    \
            __KERNEL_EXECVE(#x, _binary_obj___user_##x##_out_start,     \
                            _binary_obj___user_##x##_out_size);         \
        })

#define __KERNEL_EXECVE2(x, xstart, xsize) ({                           \
            extern unsigned char xstart[], xsize[];                     \
            __KERNEL_EXECVE(#x, xstart, (size_t)xsize);                 \
        })

#define KERNEL_EXECVE2(x, xstart, xsize)        __KERNEL_EXECVE2(x, xstart, xsize)

// user_main - kernel thread used to exec a user program
// 我们来看一下user_main函数究竟干了一些什么事情吧！
static int
user_main(void *arg) {
	//cprintf("\n==> user_main\n");
#ifdef TEST
    KERNEL_EXECVE2(TEST, TESTSTART, TESTSIZE);
#else
    KERNEL_EXECVE(exit); // 调用exit函数
#endif
    panic("user_main execve failed.\n");
}

// init_main - the second kernel thread used to create user_main kernel threads
static int
init_main(void *arg) {
	//cprintf("\n==> init_main\n");
    size_t nr_free_pages_store = nr_free_pages(); // 空闲的页面数
    size_t kernel_allocated_store = kallocated(); // 已经分配的字节数

    int pid = kernel_thread(user_main, NULL, 0); // 居然又构建一个内核线程
	//cprintf("\n<== kernel_thread\n");
	// user_main，其实我并没有看出来这个thread和上一个有什么区别
    if (pid <= 0) {
        panic("create user_main failed.\n");
    }
	extern void check_sync(void);
    check_sync();                // check philosopher sync problem

    while (do_wait(0, NULL) == 0) { // 等待子进程退出
		//cprintf("\n<== do_wait\n");
        schedule();
    }
	assert(current == initproc); // 也就是现在正在运行的确实是initproc

    cprintf("all user-mode processes have quit.\n");
    assert(initproc->cptr == NULL && initproc->yptr == NULL && initproc->optr == NULL);
    assert(nr_process == 2); // 一共是两个线程
    assert(list_next(&proc_list) == &(initproc->list_link));
    assert(list_prev(&proc_list) == &(initproc->list_link));
    assert(nr_free_pages_store == nr_free_pages());
    assert(kernel_allocated_store == kallocated());
    cprintf("init check memory pass.\n");
	//cprintf("\n<== init_main\n");
    return 0;
}

// proc_init - set up the first kernel thread idleproc "idle" by itself and 
//           - create the second kernel thread init_main
// 用于初始化线程
void
proc_init(void) {
	//cprintf("\n==> proc_init\n");
    int i;

    list_init(&proc_list); // proc_list是进程/线程的链表
    for (i = 0; i < HASH_LIST_SIZE; i ++) {
        list_init(hash_list + i); // hash_list就是一个链表数组
    }

    if ((idleproc = alloc_proc()) == NULL) { // alloc_proc()函数主要是返回一个动态分配的proc_struct
        panic("cannot alloc idleproc.\n");
    }

	// 第0号线程是idleproc
    idleproc->pid = 0;
    idleproc->state = PROC_RUNNABLE; // 标识为可以运行的进程
    idleproc->kstack = (uintptr_t)bootstack;  // 设置了内核栈的起始位置
    idleproc->need_resched = 1; // 表示需要被换出，让别的线程/进程执行
    set_proc_name(idleproc, "idle");
    nr_process ++; // 线程计数器加1

    current = idleproc; 

    int pid = kernel_thread(init_main, NULL, 0); // 创建一个内核线程
	//cprintf("\n<== kernel_thread\n");
	// 之所以叫线程，因为这些内核线程共享代码，数据等一系列的东西，只有堆栈不同而已
	// 进程的话，数据段，代码段什么的都不一样
    if (pid <= 0) {
        panic("create init_main failed.\n");
    }

    initproc = find_proc(pid);
    set_proc_name(initproc, "init");

    assert(idleproc != NULL && idleproc->pid == 0);
    assert(initproc != NULL && initproc->pid == 1);
	//cprintf("\n<== proc_init\n");
}

// cpu_idle - at the end of kern_init, the first kernel thread idleproc will do below works
void
cpu_idle(void) {
	//cprintf("\n==> cpu_idle\n");
    while (1) {
        if (current->need_resched) { // 如果当前的进程需要被切换的话
            schedule();
        }
    }
}

//FOR LAB6, set the process's priority (bigger value will get more CPU time) 
void
lab6_set_priority(uint32_t priority) // 设定优先级
{
    if (priority == 0)
        current->lab6_priority = 1; // 优先级至少是1
    else current->lab6_priority = priority;
}

// do_sleep - set current process state to sleep and add timer with "time"
//          - then call scheduler. if process run again, delete timer first.
// 我们来看一下do_sleep函数究竟干了一些什么吧！
// 
int
do_sleep(unsigned int time) {
    if (time == 0) {
        return 0;
    }
    bool intr_flag;
    local_intr_save(intr_flag); // 关闭中断
    timer_t __timer, *timer = timer_init(&__timer, current, time);
    current->state = PROC_SLEEPING; // 当前进程要处于睡眠状态
    current->wait_state = WT_TIMER;
    add_timer(timer);
    local_intr_restore(intr_flag);

    schedule();
	// 到这里的话，是被唤醒了
    del_timer(timer);
    return 0;
}
