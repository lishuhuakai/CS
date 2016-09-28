#include <defs.h>
#include <list.h>
#include <sync.h>
#include <wait.h>
#include <proc.h>

void
wait_init(wait_t *wait, struct proc_struct *proc) {
    wait->proc = proc; /* 等待的进程 */
    wait->wakeup_flags = WT_INTERRUPTED;
    list_init(&(wait->wait_link));
}

void
wait_queue_init(wait_queue_t *queue) { /* 等待队列 */
    list_init(&(queue->wait_head));
}

void
wait_queue_add(wait_queue_t *queue, wait_t *wait) { /* queue是表头，wait是元素 */
    assert(list_empty(&(wait->wait_link)) && wait->proc != NULL);
    wait->wait_queue = queue; /* 这种结构很怪异，因为链表中每个元素都有指向表头的指针 */
    list_add_before(&(queue->wait_head), &(wait->wait_link)); /* 将这个wait添加到表尾 */
}

void
wait_queue_del(wait_queue_t *queue, wait_t *wait) {
    assert(!list_empty(&(wait->wait_link)) && wait->wait_queue == queue);
    list_del_init(&(wait->wait_link));
}

wait_t *
wait_queue_next(wait_queue_t *queue, wait_t *wait) { /* 查找下一个元素 */
    assert(!list_empty(&(wait->wait_link)) && wait->wait_queue == queue);
    list_entry_t *le = list_next(&(wait->wait_link)); /* 找到wait的第一个元素 */
    if (le != &(queue->wait_head)) {
        return le2wait(le, wait_link);
    }
    return NULL;
}

/* 我有一个疑问，那就是既然叫做queue，那么哪里体现了呢？ */
wait_t *
wait_queue_prev(wait_queue_t *queue, wait_t *wait) { /* 估计是要还找到wait在queue中的前一个元素 */
    assert(!list_empty(&(wait->wait_link)) && wait->wait_queue == queue);
    list_entry_t *le = list_prev(&(wait->wait_link));
    if (le != &(queue->wait_head)) {
        return le2wait(le, wait_link);
    }
    return NULL;
}

wait_t *
wait_queue_first(wait_queue_t *queue) { /* 返回第一个元素 */
    list_entry_t *le = list_next(&(queue->wait_head));
    if (le != &(queue->wait_head)) {
        return le2wait(le, wait_link);
    }
    return NULL;
}

wait_t *
wait_queue_last(wait_queue_t *queue) { /* 返回最后一个元素 */
    list_entry_t *le = list_prev(&(queue->wait_head));
    if (le != &(queue->wait_head)) {
        return le2wait(le, wait_link);
    }
    return NULL;
}

bool
wait_queue_empty(wait_queue_t *queue) { /* 判断是否为空 */
    return list_empty(&(queue->wait_head));
}

bool
wait_in_queue(wait_t *wait) { /* 判断这个wait是否在queue之中 */
    return !list_empty(&(wait->wait_link));
}

void
wakeup_wait(wait_queue_t *queue, wait_t *wait, uint32_t wakeup_flags, bool del) {
    if (del) { /* 是否要将其从queue中删除 */
        wait_queue_del(queue, wait); /* 从queue中删除这个元素 */
    }
    wait->wakeup_flags = wakeup_flags;
    wakeup_proc(wait->proc); /* 所谓的wakeup_proc不过是将进程放入就绪队列而已 */
}

void
wakeup_first(wait_queue_t *queue, uint32_t wakeup_flags, bool del) {
    wait_t *wait;
    if ((wait = wait_queue_first(queue)) != NULL) {
        wakeup_wait(queue, wait, wakeup_flags, del);
    }
}

void
wakeup_queue(wait_queue_t *queue, uint32_t wakeup_flags, bool del) {
    wait_t *wait;
    if ((wait = wait_queue_first(queue)) != NULL) {
        if (del) {
            do {
                wakeup_wait(queue, wait, wakeup_flags, 1);
            } while ((wait = wait_queue_first(queue)) != NULL);
        }
        else {
            do {
                wakeup_wait(queue, wait, wakeup_flags, 0);
            } while ((wait = wait_queue_next(queue, wait)) != NULL);
        }
    }
}

void
wait_current_set(wait_queue_t *queue, wait_t *wait, uint32_t wait_state) {
	/* queue是队列的头部 */
    assert(current != NULL);
    wait_init(wait, current);
    current->state = PROC_SLEEPING; /* 表示在沉睡中 */
    current->wait_state = wait_state;
    wait_queue_add(queue, wait); /* 将wait加入queue的尾部 */
}

