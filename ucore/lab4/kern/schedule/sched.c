#include <list.h>
#include <sync.h>
#include <proc.h>
#include <sched.h>
#include <assert.h>

// 我们来看一下这个函数吧！  
void
wakeup_proc(struct proc_struct *proc) {
    assert(proc->state != PROC_ZOMBIE && proc->state != PROC_RUNNABLE);
    proc->state = PROC_RUNNABLE; // 表示这个proc是可以运行的
}

// schedule究竟是用来干什么的？
// 是用来安排进程们的运行的
void
schedule(void) {
    bool intr_flag;
    list_entry_t *le, *last;
    struct proc_struct *next = NULL;
    local_intr_save(intr_flag); // 关闭中断
    {
        current->need_resched = 0;
        last = (current == idleproc) ? &proc_list : &(current->list_link);
        le = last;
        do {
            if ((le = list_next(le)) != &proc_list) {
                next = le2proc(le, list_link);
                if (next->state == PROC_RUNNABLE) { // 一直到找到一个可以运行的进程为止
                    break;
                }
            }
        } while (le != last);

		// next == NULL表示proc_list为空
		// next->state != PROC_RUNNABLE表示已经找到了最后一个了，但是仍然没有找到可以运行的进程
        if (next == NULL || next->state != PROC_RUNNABLE) {
            next = idleproc;
        }
		next->runs ++; // runs指的是进程运行的次数
        if (next != current) {
            proc_run(next);
        }
    }
    local_intr_restore(intr_flag); // 开启中断
}

