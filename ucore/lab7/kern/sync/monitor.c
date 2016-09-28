#include <stdio.h>
#include <monitor.h>
#include <kmalloc.h>
#include <assert.h>

/* 管程 */

// Initialize monitor.
void     
monitor_init (monitor_t * mtp, size_t num_cv) { /* 我们来看一下管程是如何初始化的吧！ */
    int i;
    assert(num_cv>0);
    mtp->next_count = 0; /* 管程是一个构件 */
    mtp->cv = NULL;
    sem_init(&(mtp->mutex), 1); // unlocked,确实是代表解锁,允许别的进程进入
    sem_init(&(mtp->next), 0);  // next就没有解锁
    mtp->cv =(condvar_t *) kmalloc(sizeof(condvar_t)*num_cv); /* 居然动态申请了一个条件变量数组 */
    assert(mtp->cv!=NULL);
    for(i=0; i<num_cv; i++){
        mtp->cv[i].count=0;
        sem_init(&(mtp->cv[i].sem),0);
        mtp->cv[i].owner=mtp; /* 管程 */ 
    }
}

// Unlock one of threads waiting on the condition variable. 
// 条件变量的学名叫做管程
void 
cond_signal (condvar_t *cvp) { /* 条件变量 */
	// 如果说，我要调用cond_signal的话，我是肯定要唤醒一个睡眠的进程的
   //LAB7 EXERCISE1: YOUR CODE
   cprintf("cond_signal begin: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);  
  /*
   *      cond_signal(cv) {
   *          if(cv.count>0) {
   *             mt.next_count ++;
   *             signal(cv.sem);
   *             wait(mt.next);
   *             mt.next_count--;
   *          }
   *       }
   */

   /* 首先进程B判断cv.count，如果不大于0，则表示当前没有执行cond_wait而睡眠的进程，因此就没有被唤
	* 醒的对象了，直接函数返回即可；如果大于0，这表示当前有执行cond_wait而睡眠的进程A，因此需要唤醒等待在cv.sem上睡眠的进程A。由于
	* 只允许一个进程在管程中执行，所以一旦进程B唤醒了别人（进程A），那么自己就需要睡眠。故让monitor.next_count加一，且让自己（进程B）
	* 睡在信号量monitor.next上。如果睡醒了，这让monitor.next_count减一。
	*/

   // 假设执行这段代码的是进程B
   if (cvp->count > 0) // count大于0代表当前有执行cond_wait而睡眠的进程A,自然，进程B要唤醒进程A，因为进程B要释放资源
   {
	   cvp->owner->next_count++; // owner代表这个进程的所属的管程 owner->next_count代表等待在管程中执行的进程的数目
	   up(&(cvp->sem)); // 信号量增加 cpv代表条件变量, cvp->sem代表信号量,一般而言这会唤醒一个等待进程(将这个进程放入就绪队列)
	   down(&(cvp->owner->next)); // 让当前进程睡在信号量monitor.next上
	   cvp->owner->next_count--; // 管程的等待唤醒的进程的数目减1
   }
   cprintf("cond_signal end: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}

// 条件变量是什么东西，一般来说，只要你拥有条件变量，

// Suspend calling thread on a condition variable waiting for condition Atomically unlocks 
// mutex and suspends calling thread on conditional variable after waking up locks mutex.
// Notice: mp is mutex semaphore for monitor's procedures
void
cond_wait (condvar_t *cvp) {
    //LAB7 EXERCISE1: YOUR CODE
    cprintf("cond_wait begin:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
   /*
    *         cv.count ++;
    *         if(mt.next_count>0)
    *            signal(mt.next)
    *         else
    *            signal(mt.mutex);
    *         wait(cv.sem);
    *         cv.count --;
    */
	cvp->count++; /* 等待这个条件的进程数加1 */

	/* 情况一：如果monitor.next_count如果大于0，表示有大于等于1个进程执行cond_signal函数且睡着了，就睡在了monitor.next信号量上。
	 * 假定这些进程形成S进程链表。因此需要唤醒S进程链表中的一个进程B。然后进程A睡在cv.sem上，如果睡醒了，则让cv.count减一，
	 * 表示等待此条件的睡眠进程个数少了一个，可继续执行了！这里隐含这一个现象，即某进程A在时间顺序上先执行了signal_cv，而另一个进程B
	 * 后执行了wait_cv，这会导致进程A没有起到唤醒进程B的作用。这里还隐藏这一个问题，在cond_wait有sem_signal(mutex)，但没有看到哪
	 * 里有sem_wait(mutex)，这好像没有成对出现，是否是错误的？其实在管程中的每一个函数的入口处会有wait(mutex)，这样二者就配好对了。

     * 情况二：如果monitor.next_count如果小于等于0，表示目前没有进程执行cond_signal函数且睡着了，那需要唤醒的是由于互斥条件限制而
	 * 无法进入管程的进程，所以要唤醒睡在monitor.mutex上的进程。然后进程A睡在cv.sem上，如果睡醒了，则让cv.count减一，表示等待此条
	 * 件的睡眠进程个数少了一个，可继续执行了！
	 */
	if (cvp->owner->next_count > 0) // 表示有大于等于1个进程因为执行cond_signal而睡着了
		up(&(cvp->owner->next)); // 唤醒一个继续运行
	else /* 表示当前没有进程执行cond_signal函数且睡着了 */
		up(&(cvp->owner->mutex)); /* 唤醒由于互斥条件限制而无法进入管程的线程 */
	down(&(cvp->sem)); /* 等待这个条件变为真 */
	/* 表示被唤醒 */
	cvp->count--; /* 表示等待此条件的进程数目减1 */
    cprintf("cond_wait end:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}
