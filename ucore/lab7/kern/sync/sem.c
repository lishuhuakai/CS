#include <defs.h>
#include <wait.h>
#include <atomic.h>
#include <kmalloc.h>
#include <sem.h>
#include <proc.h>
#include <sync.h>
#include <assert.h>

void
sem_init(semaphore_t *sem, int value) { /* 信号量的初始化 */
    sem->value = value;
    wait_queue_init(&(sem->wait_queue));
}

static __noinline void __up(semaphore_t *sem, uint32_t wait_state) { /* 这应该被看做是一个原子操作 */
    bool intr_flag;
    local_intr_save(intr_flag); /* 关闭中断，这其实就相当于是一个原子操作了 */
    {
        wait_t *wait;
        if ((wait = wait_queue_first(&(sem->wait_queue))) == NULL) { /* 不过为什么要获得第一个元素的值,如果为NULL，说明没有进程等待 */
            sem->value ++; /* 信号量的值加1 */
        }
        else {
            assert(wait->proc->wait_state == wait_state);
            wakeup_wait(&(sem->wait_queue), wait, wait_state, 1); /* 直接唤醒一个进程即可 */
        }
    }
    local_intr_restore(intr_flag); /* 开启中断 */
}

static __noinline uint32_t __down(semaphore_t *sem, uint32_t wait_state) {
    bool intr_flag;
    local_intr_save(intr_flag); /* 关闭中断 */
    if (sem->value > 0) { /* 信号值大于0，其数值表示资源数目 */
        sem->value --; /* 信号量减1 */
        local_intr_restore(intr_flag);
        return 0;
    }
	/**
	 * 如果sem->value <= 0的话,sem->value的绝对值就是等待的进程的数目
	 */
    wait_t __wait, *wait = &__wait;
    wait_current_set(&(sem->wait_queue), wait, wait_state); /* __wait只是一个栈变量而已,其实这个函数只是要实现的只是将当前这个
															正在运行的进程放入wait_queue的等待队列里面而已*/
    local_intr_restore(intr_flag); /* 开启中断 */

    schedule(); /* 然后重新分配cpu资源 */

	/* 到这里的话，说明被唤醒了 */
    local_intr_save(intr_flag);
    wait_current_del(&(sem->wait_queue), wait); /* 将wait从sem的等待队列中删除 */
    local_intr_restore(intr_flag);

    if (wait->wakeup_flags != wait_state) { /* 等待前后的标志发生了改变，所以要返回发生了改变的标志？ */
        return wait->wakeup_flags;
    }
    return 0;
}

void
up(semaphore_t *sem) {
    __up(sem, WT_KSEM);
}

void
down(semaphore_t *sem) {
    uint32_t flags = __down(sem, WT_KSEM);
    assert(flags == 0);
}

bool
try_down(semaphore_t *sem) { /*有意思，try_down是干什么的呢？*/
    bool intr_flag, ret = 0;
    local_intr_save(intr_flag); /*关闭中断*/
    if (sem->value > 0) { 
        sem->value --, ret = 1;
    }
    local_intr_restore(intr_flag); /*开启中断*/
    return ret;
}

