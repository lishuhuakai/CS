#include <defs.h>
#include <list.h>
#include <proc.h>
#include <assert.h>
#include <default_sched.h>

// 我们来看一下所谓的round-robin算法
// rr非常类似于先进先出
static void
RR_init(struct run_queue *rq) {
    list_init(&(rq->run_list));
    rq->proc_num = 0;
}

static void
RR_enqueue(struct run_queue *rq, struct proc_struct *proc) { // 将一个进程放入队列之中
    assert(list_empty(&(proc->run_link)));
	// rq是就绪队列 ready queue
    list_add_before(&(rq->run_list), &(proc->run_link)); // 将进程控制块的指针放在rq队列的末尾
    if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) { 
        proc->time_slice = rq->max_time_slice; // 时间片要重新设置
    }
    proc->rq = rq;
    rq->proc_num ++;
}

static void
RR_dequeue(struct run_queue *rq, struct proc_struct *proc) { // 现在应该是出队列
    assert(!list_empty(&(proc->run_link)) && proc->rq == rq); // 
    list_del_init(&(proc->run_link)); // proc->run_link指向什么东西?run_link points to the entry linked in run queue
    rq->proc_num --; 
}

static struct proc_struct *
RR_pick_next(struct run_queue *rq) { // 获取下一个进程的proc_struct的地址
    list_entry_t *le = list_next(&(rq->run_list)); // 取就绪队列rq队头队列元素，并把队列元素转换成进程控制块的指针
    if (le != &(rq->run_list)) {
        return le2proc(le, run_link);
    }
    return NULL;
}

// 每次timer到时后，trap函数会间接调用此函数来吧当前执行进程的时间片time_slice减1
// 如果time_slice降到零，则设置次进程成员变量need_resched标识为1，这样咋爱下一次中断来后指向trap函数时
// 会由于当前进程成员变量need_resched标识为1而执行schedule函数，从而把当前执行进程放回就绪队列末尾，从而
// 就绪队列头取出在就绪队列上等待时间最久的那个就绪进程执行
static void
RR_proc_tick(struct run_queue *rq, struct proc_struct *proc) { // 时间片控制
    if (proc->time_slice > 0) {
        proc->time_slice --;
    }
    if (proc->time_slice == 0) { // 如果时间片为0，代表要重新分配
        proc->need_resched = 1; // need_resched代表需要重新分配
    }
}

struct sched_class default_sched_class_e = {
    .name = "RR_scheduler",
    .init = RR_init,
    .enqueue = RR_enqueue,
    .dequeue = RR_dequeue,
    .pick_next = RR_pick_next,
    .proc_tick = RR_proc_tick,
};

