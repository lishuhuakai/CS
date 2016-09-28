#ifndef __KERN_PROCESS_PROC_H__
#define __KERN_PROCESS_PROC_H__

#include <defs.h>
#include <list.h>
#include <trap.h>
#include <memlayout.h>


// process's state in his life cycle
enum proc_state {
    PROC_UNINIT = 0,  // uninitialized
    PROC_SLEEPING,    // sleeping
    PROC_RUNNABLE,    // runnable(maybe running)
    PROC_ZOMBIE,      // almost dead, and wait parent proc to reclaim his resource
};

// Saved registers for kernel context switches.
// Don't need to save all the %fs etc. segment registers,
// because they are constant across kernel contexts.
// Save all the regular registers so we don't need to care
// which are caller save, but not the return register %eax.
// (Not saving %eax just simplifies the switching code.)
// The layout of context must match code in switch.S.
struct context {
    uint32_t eip;
    uint32_t esp;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
};

#define PROC_NAME_LEN               15 // 名字的长度
#define MAX_PROCESS                 4096 // 最大线程的个数
#define MAX_PID                     (MAX_PROCESS * 2)

extern list_entry_t proc_list;

struct proc_struct {
    enum proc_state state;                      // Process state # 进程的状态
    int pid;                                    // Process ID # 进程的id
    int runs;                                   // the running times of Proces # 进程运行的次数
    uintptr_t kstack;                           // Process kernel stack # 内核栈的地址
    volatile bool need_resched;                 // bool value: need to be rescheduled to release CPU? # 为什么感觉是需要重新分配cpu或者释放cpu
    struct proc_struct *parent;                 // the parent process # 父进程
    struct mm_struct *mm;                       // Process's memory management field # 好吧，我就知道每个线程都会有这么一茬
    struct context context;                     // Switch here to run process
    struct trapframe *tf;                       // Trap frame for current interrupt # 中断的frame
    uintptr_t cr3;                              // CR3 register: the base addr of Page Directroy Table(PDT) # cr3都可以随时切换，有意思
    uint32_t flags;                             // Process flag
    char name[PROC_NAME_LEN + 1];               // Process name # 进程的名字
    list_entry_t list_link;                     // Process link list # 为什么会有两种link
    list_entry_t hash_link;                     // Process hash list
};

#define le2proc(le, member)         \
    to_struct((le), struct proc_struct, member)

extern struct proc_struct *idleproc, *initproc, *current;

void proc_init(void);
void proc_run(struct proc_struct *proc);
int kernel_thread(int (*fn)(void *), void *arg, uint32_t clone_flags);

char *set_proc_name(struct proc_struct *proc, const char *name);
char *get_proc_name(struct proc_struct *proc);
void cpu_idle(void) __attribute__((noreturn));

struct proc_struct *find_proc(int pid);
int do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf);
int do_exit(int error_code);

#endif /* !__KERN_PROCESS_PROC_H__ */

