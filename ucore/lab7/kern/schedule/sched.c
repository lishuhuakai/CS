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
    rq->max_time_slice = 5;
    sched_class->init(rq); // 初始化调度类

    cprintf("sched class: %s\n", sched_class->name);
}

void
wakeup_proc(struct proc_struct *proc) { // 用于唤醒进程
	//cprintf("\n==> wakeup_proc\n");
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
	//cprintf("\n<== wakeup_proc\n");
}

void
schedule(void) { // 这个用来分配进程的运行
	//cprintf("\n==> schedule\n");
    bool intr_flag;
    struct proc_struct *next;
    local_intr_save(intr_flag); // 关闭中断
    {
        current->need_resched = 0;
        if (current->state == PROC_RUNNABLE) { // 标记为可以运行
            sched_class_enqueue(current); // 将当前进程放入就绪队列
        }
        if ((next = sched_class_pick_next()) != NULL) { // 选出优先级最高的一个进程,当然，这要看具体的调度算法
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

// add timer to timer_list
/*
 *	这个我要稍微讲解一下原理：
 * timer_list是按照timer的过期时间从小到大排列各个timer的。
 * 举一个例子，下面这张图是一个timer_list.为了方便，去掉了表头，省略掉了timer的其它部分，只取timer的expires部分
 *  1 --> 2 --> 1 --> 4 --> 2
 * 这说明，在现在这个时刻，第一个timer还有1个时间单位到期
 * 第二个timer还有3个时间单位到期
 * 第三个timer还有4个时间单位到期
 * 依次类推
 */
void
add_timer(timer_t *timer) {
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        assert(timer->expires > 0 && timer->proc != NULL);
        assert(list_empty(&(timer->timer_link))); // 一个timer只能被加入到一个timer_list中
        list_entry_t *le = list_next(&timer_list); // 得到第一个元素
        while (le != &timer_list) { // 开始遍历
            timer_t *next = le2timer(le, timer_link);
            if (timer->expires < next->expires) { // timer_list是按照过期时间从小到大排序的
                next->expires -= timer->expires; // 为什么都要减去timer->expires
                break;
            }
            timer->expires -= next->expires; // 参照前面的原理自己来推
            le = list_next(le);
        }
        list_add_before(le, &(timer->timer_link));
    }
    local_intr_restore(intr_flag);
}

// del timer from timer_list
void
del_timer(timer_t *timer) {
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        if (!list_empty(&(timer->timer_link))) {
            if (timer->expires != 0) {
                list_entry_t *le = list_next(&(timer->timer_link));
                if (le != &timer_list) {
                    timer_t *next = le2timer(le, timer_link);
                    next->expires += timer->expires; // 也就是说，过期时间不变
                }
            }
            list_del_init(&(timer->timer_link));
        }
    }
    local_intr_restore(intr_flag);
}

// call scheduler to update tick related info, and check the timer is expired? If expired, then wakup proc
// 我们来看一下这些个函数究竟都干了一些什么事情吧！
void
run_timer_list(void) {
    bool intr_flag;
    local_intr_save(intr_flag); /* 关闭中断 */
    {
        list_entry_t *le = list_next(&timer_list); /* timer_list应该是一个全局的变量 */
        if (le != &timer_list) {
            timer_t *timer = le2timer(le, timer_link);
            assert(timer->expires != 0);
            timer->expires --; /* 距离过期又近了一步 */
            while (timer->expires == 0) { /* 这个进程的时间片到了 */
                le = list_next(le);
                struct proc_struct *proc = timer->proc; /* 获得下一个进程 */
                if (proc->wait_state != 0) {
                    assert(proc->wait_state & WT_INTERRUPTED);
                }
                else {
                    warn("process %d's wait_state == 0.\n", proc->pid);
                }
                wakeup_proc(proc); /* 唤醒下一个进程 */
                del_timer(timer);
                if (le == &timer_list) {
                    break;
                }
                timer = le2timer(le, timer_link);
            }
        }
        sched_class_proc_tick(current);
    }
    local_intr_restore(intr_flag); /* 开启中断 */
}
