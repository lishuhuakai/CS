#include <list.h>
#include <sync.h>
#include <proc.h>
#include <sched.h>
#include <stdio.h>
#include <assert.h>
#include <default_sched.h>

// the list of timer
static list_entry_t timer_list;

static struct sched_class *sched_class;

static struct run_queue *rq;

// 有人好奇，这个sched_class究竟是什么？其实在sched_init函数里有这么一句: sched_class = &default_sched_class;
// 课件这个玩意早就分配好了
static inline void
sched_class_enqueue(struct proc_struct *proc) {
    if (proc != idleproc) {
        sched_class->enqueue(rq, proc); // 总之就是入队列
    }
}

static inline void
sched_class_dequeue(struct proc_struct *proc) {
    sched_class->dequeue(rq, proc); // 出队列
}

static inline struct proc_struct *
sched_class_pick_next(void) {
    return sched_class->pick_next(rq);
}

static void
sched_class_proc_tick(struct proc_struct *proc) {
    if (proc != idleproc) {
        sched_class->proc_tick(rq, proc);
    }
    else {
        proc->need_resched = 1;
    }
}

static struct run_queue __rq;

// 现在貌似又改变了风向，进程的调度制度发生了改变
void
sched_init(void) {
    list_init(&timer_list);

    sched_class = &default_sched_class;

    rq = &__rq;
    rq->max_time_slice = MAX_TIME_SLICE; // 最大的时间片好像为5
    sched_class->init(rq); // 初始化调度类

    cprintf("sched class: %s\n", sched_class->name);
}

void
wakeup_proc(struct proc_struct *proc) { // 用于唤醒进程
	cprintf("\n==> wakeup_proc\n");
    assert(proc->state != PROC_ZOMBIE);
    bool intr_flag;
    local_intr_save(intr_flag); // 关闭中断
    {
        if (proc->state != PROC_RUNNABLE) { // 表示不可以运行
            proc->state = PROC_RUNNABLE; // 见进程标记为可以运行
            proc->wait_state = 0;
            if (proc != current) {
                sched_class_enqueue(proc); // 将其放入就绪队列，是吧
            }
        }
        else {
            warn("wakeup runnable process.\n");
        }
    }
    local_intr_restore(intr_flag); // 开启中断
	cprintf("\n<== wakeup_proc\n");
}

void
schedule(void) { // 这个用来分配进程的运行
	cprintf("\n==> schedule\n");
    bool intr_flag;
    struct proc_struct *next;
    local_intr_save(intr_flag);
    {
        current->need_resched = 0;
        if (current->state == PROC_RUNNABLE) { // 标记为可以运行
            sched_class_enqueue(current); // 将当前进程放入就绪队列
        }
        if ((next = sched_class_pick_next()) != NULL) { // 选出优先级最高的一个进程
            sched_class_dequeue(next);
        }
        if (next == NULL) {
            next = idleproc; // 如果没有找到就绪的进程，那么继续运行自己
        }
        next->runs ++;
        if (next != current) {
            proc_run(next); // 开始运行
        }
    }
    local_intr_restore(intr_flag);
}
