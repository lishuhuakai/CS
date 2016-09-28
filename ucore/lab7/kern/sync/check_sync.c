#include <stdio.h>
#include <proc.h>
#include <sem.h>
#include <monitor.h>
#include <assert.h>

#define N 5 /* 哲学家数目 */
#define LEFT (i-1+N)%N /* i的左邻号码 */
#define RIGHT (i+1)%N /* i的右邻号码 */
#define THINKING 0 /* 哲学家正在思考 */
#define HUNGRY 1 /* 哲学家想取得叉子 */
#define EATING 2 /* 哲学家正在吃面 */
#define TIMES  4 /* 吃4次饭 */
#define SLEEP_TIME 10

//-----------------philosopher problem using monitor ------------
/*PSEUDO CODE :philosopher problem using semaphore
system DINING_PHILOSOPHERS

VAR
me:    semaphore, initially 1;                    # for mutual exclusion 
s[5]:  semaphore s[5], initially 0;               # for synchronization 
pflag[5]: {THINK, HUNGRY, EAT}, initially THINK;  # philosopher flag 

# As before, each philosopher is an endless cycle of thinking and eating.

procedure philosopher(i)
  {
    while TRUE do
     {
       THINKING;
       take_chopsticks(i);
       EATING;
       drop_chopsticks(i);
     }
  }

# The take_chopsticks procedure involves checking the status of neighboring 
# philosophers and then declaring one's own intention to eat. This is a two-phase 
# protocol; first declaring the status HUNGRY, then going on to EAT.

procedure take_chopsticks(i)
  {
    DOWN(me);               # critical section 
    pflag[i] := HUNGRY;
    test[i];
    UP(me);                 # end critical section 
    DOWN(s[i])              # Eat if enabled 
   }

void test(i)                # Let phil[i] eat, if waiting 
  {
    if ( pflag[i] == HUNGRY
      && pflag[i-1] != EAT
      && pflag[i+1] != EAT)
       then
        {
          pflag[i] := EAT;
          UP(s[i])
         }
    }


# Once a philosopher finishes eating, all that remains is to relinquish the 
# resources---its two chopsticks---and thereby release waiting neighbors.

void drop_chopsticks(int i)
  {
    DOWN(me);                # critical section 
    test(i-1);               # Let phil. on left eat if possible 
    test(i+1);               # Let phil. on rght eat if possible 
    UP(me);                  # up critical section 
   }

*/
//---------- philosophers problem using semaphore ----------------------
int state_sema[N]; /* 记录每个人状态的数组 */
/* 信号量是一个特殊的整型变量 */
semaphore_t mutex; /* 临界区互斥 */
semaphore_t s[N]; /* 每个哲学家一个信号量 */

struct proc_struct *philosopher_proc_sema[N];

/// 关于哲学家问题的信号量解法，我想说一下，虽然下面的代码可以解，但是每次只有一个哲学家
/// 进餐或者放下筷子，效率非常低下
/// 

void phi_test_sema(i) /* i：哲学家号码从0到N-1 */
{ 
    if(state_sema[i] == HUNGRY && state_sema[LEFT]!= EATING
            &&state_sema[RIGHT] != EATING)
    {
        state_sema[i]=EATING;
        up(&s[i]); // 吃完之后记得释放资源,关键是我没有看见down(&s[i])
    }
}

void phi_take_forks_sema(int i) /* i：哲学家号码从0到N-1 */
{ 
        down(&mutex); /* 进入临界区 */
        state_sema[i] = HUNGRY; /* 记录下哲学家i饥饿的事实 */
        phi_test_sema(i); /* 试图得到两只叉子 */

		/*	 if (state_sema[i] == HUNGRY && state_sema[LEFT] != EATING
		 *		&&state_sema[RIGHT] != EATING)
		 *	 {
		 *		state_sema[i] = EATING;
		 *		up(&s[i]); // 吃完之后记得释放资源
		 *	 }
		 */

        up(&mutex); /* 离开临界区 */
        down(&s[i]); /* 如果得不到叉子就阻塞,什么时候能够解除阻塞呢？只有当哲学家i获得了叉子才行
					  * 注意下面的phi_put_forks_sema函数，它会检查左邻居和右邻居是否可以进餐，
					  * 如果可以的话，它会唤醒阻塞在这里的哲学家i
					  */
		// 如果得到了叉子就会返回
}

void phi_put_forks_sema(int i) /* i：哲学家号码从0到N-1 */
{ 
		/* 取和放叉子使用的是同一个mutex */
        down(&mutex); /* 进入临界区 */
        state_sema[i]=THINKING; /* 哲学家进餐结束 */
        phi_test_sema(LEFT); /* 看一下左邻居现在是否能进餐 */
        phi_test_sema(RIGHT); /* 看一下右邻居现在是否能进餐 */
        up(&mutex); /* 离开临界区 */
}

int philosopher_using_semaphore(void * arg) /* i：哲学家号码，从0到N-1 */
{
    int i, iter=0;
    i=(int)arg;
    cprintf("I am No.%d philosopher_sema\n",i);
    while(iter++ < TIMES)
    { /* 无限循环 */
        cprintf("Iter %d, No.%d philosopher_sema is thinking\n",iter,i); /* 哲学家正在思考 */
        do_sleep(SLEEP_TIME);
        phi_take_forks_sema(i); /* 醒来之后试图取叉子 */
        /* 需要两只叉子，或者阻塞 */
        cprintf("Iter %d, No.%d philosopher_sema is eating\n",iter,i); /* 进餐 */
        do_sleep(SLEEP_TIME); /* 模拟耗时操作 */
        phi_put_forks_sema(i); 
        /* 把两把叉子同时放回桌子 */
    }
    cprintf("No.%d philosopher_sema quit\n",i);
    return 0;    
}

//-----------------philosopher problem using monitor ------------
/*PSEUDO CODE :philosopher problem using monitor
 * monitor dp
 * {
 *  enum {thinking, hungry, eating} state[5];
 *  condition self[5];
 *
 *  void pickup(int i) {
 *      state[i] = hungry;
 *      if ((state[(i+4)%5] != eating) && (state[(i+1)%5] != eating)) {
 *        state[i] = eating;
 *      else
 *         self[i].wait();
 *   }
 *
 *   void putdown(int i) {
 *      state[i] = thinking;
 *      if ((state[(i+4)%5] == hungry) && (state[(i+3)%5] != eating)) {
 *          state[(i+4)%5] = eating;
 *          self[(i+4)%5].signal();
 *      }
 *      if ((state[(i+1)%5] == hungry) && (state[(i+2)%5] != eating)) {
 *          state[(i+1)%5] = eating;
 *          self[(i+1)%5].signal();
 *      }
 *   }
 *
 *   void init() {
 *      for (int i = 0; i < 5; i++)
 *         state[i] = thinking;
 *   }
 * }
 */

struct proc_struct *philosopher_proc_condvar[N]; // N philosopher
int state_condvar[N];                            // the philosopher's state: EATING, HUNGARY, THINKING  
monitor_t mt, *mtp=&mt;                          // monitor

void phi_test_condvar (i) { 
    if(state_condvar[i] == HUNGRY && state_condvar[LEFT] != EATING
            && state_condvar[RIGHT] != EATING) { // 左边没有在吃饭，右边也没有在吃饭
        cprintf("phi_test_condvar: state_condvar[%d] will eating\n",i);
        state_condvar[i] = EATING ; // 那么这个哲学家就可以吃饭了
        cprintf("phi_test_condvar: signal self_cv[%d] \n",i);
        cond_signal(&mtp->cv[i]) ; // 吃完饭后释放资源

		/*  我们来仔细挖一挖cond_signal(&mtp->cv[i])究竟干了一些什么事情.我们将mtp->cv[i]记作cvp
		 *  if (cvp->count > 0) // count大于0代表当前有执行cond_wait而睡眠的进程A,自然，进程B要唤醒进程A，因为进程B要释放资源
		 *	{
		 *		mtp->next_count++;		// mtp是当前的管程 mtp->next_count代表等待在管程中执行的进程的数目
		 *		up(&(cvp->sem));		// 唤醒一个等待在cpv上的进程,如果没有等待的话，就让计数加1
		 *		down(&(mtp->next));		// 让当前进程睡在信号量monitor.next上,初始化的时候monitor.next值为0
		 *		// 到这里的话，说明被别的进程唤醒了
		 *		mtp->next_count--;		// 管程的等待唤醒的进程的数目减1
		 *	}
		 *
		 */

    }
}


void phi_take_forks_condvar(int i) { /* 哲学家号码从0到N-1 */
     down(&(mtp->mutex)); // 任何时候，管程只有一个进程在运行
//--------into routine in monitor--------------
     // LAB7 EXERCISE1: YOUR CODE
     // I am hungry
	 state_condvar[i] = HUNGRY;
     // try to get fork
	 phi_test_condvar(i);
	 if (state_condvar[i] != EATING) { // 表示这个哲学家并没有获取到餐具
		 cprintf("phi_take_forks_condvar: %d didn't get fork and will wait\n", i);
		 cond_wait(&mtp->cv[i]); // 在cond_wait函数中，可能会唤醒别的阻塞在monitor.next上的进程，也可能释放mtp->mutex
		 /* 假定&mtp->cv[i]为cvp，则执行的结果如下:
		  * cvp->count++;
		  * if (mtp->next_count > 0) // 表示有大于等于1个进程因为执行cond_signal而睡着了
		  *		up(&(mtp->next)); // 唤醒一个继续运行
		  *	else // 表示当前没有进程执行cond_signal函数且睡着了
		  *		up(&(mtp->mutex)); // 唤醒由于互斥条件限制而无法进入管程的线程 
		  * down(&(cvp->sem)); // 等待这个条件变为真,down函数会重新调度程序
		  * // 运行到这里的话，表明被唤醒，什么时候会被唤醒呢？我们看一下就知道了，在phi_put_forks_condvar中
		  * // 有这样的指令 phi_test_condvar(LEFT);或者phi_test_condvar(RIGHT);
		  * // 用于测试左右的哲学家是否可以就餐，而在phi_test_condvar函数中，有这样的语句
		  * // cond_signal(&mtp->cv[i]) ;
		  * // 很明显，这里就是被这样的语句唤醒的
		  * cvp->count--; // 表示等待此条件的进程数目减1 
		  */
	 }
//--------leave routine in monitor--------------
	 // 到这里的话，是被唤醒了,说明已经获取到了叉子
	 // 同时也获得了mtp->mutex
	 // 既然离开了管程，那么就要唤醒别的进程
      if(mtp->next_count > 0) // 如果已经有进程进入了管程，但是被阻塞到了monitor.next之上
         up(&(mtp->next)); // 唤醒一个
      else
         up(&(mtp->mutex)); // 允许别的进程进入管程
}

void phi_put_forks_condvar(int i) {
     down(&(mtp->mutex)); // 进入互斥区一定要先获得monitor的mutex
//--------into routine in monitor--------------
     // LAB7 EXERCISE1: YOUR CODE
     // I ate over
	 state_condvar[i] = THINKING;
     // test left and right neighbors
	 phi_test_condvar(LEFT);  // 这里可能会唤醒别的阻塞进程
	 phi_test_condvar(RIGHT);
//--------leave routine in monitor--------------
	 // 离开之后要唤醒别的进程
     if(mtp->next_count > 0)
        up(&(mtp->next));
     else
        up(&(mtp->mutex));
}

//---------- philosophers using monitor (condition variable) ----------------------
int philosopher_using_condvar(void * arg) { /* arg is the No. of philosopher 0~N-1*/
  // 我们来看一下，使用了条件变量的线程是如何来完成哲学家问题的吧！
    int i, iter=0;
    i=(int)arg;
    cprintf("I am No.%d philosopher_condvar\n",i);
    while(iter++ < TIMES)
    { /* iterate*/
        cprintf("Iter %d, No.%d philosopher_condvar is thinking\n",iter,i); /* thinking*/
        do_sleep(SLEEP_TIME); /* 模拟耗时 */
        phi_take_forks_condvar(i); /* 取叉子，或者被阻塞 */
        /* need two forks, maybe blocked */
        cprintf("Iter %d, No.%d philosopher_condvar is eating\n",iter,i); /* eating*/
        do_sleep(SLEEP_TIME);
        phi_put_forks_condvar(i); /* 放叉子 */
        /* return two forks back*/
    }
    cprintf("No.%d philosopher_condvar quit\n",i);
    return 0;    
}

void check_sync(void){

    int i;

    //check semaphore
    sem_init(&mutex, 1); // 资源的数目只有1
    //for(i=0;i<N;i++){
    //    sem_init(&s[i], 0); // 一共初始化5个信号量
    //    int pid = kernel_thread(philosopher_using_semaphore, (void *)i, 0); // 构建5个线程
    //    if (pid <= 0) {
    //        panic("create No.%d philosopher_using_semaphore failed.\n");
    //    }
    //    philosopher_proc_sema[i] = find_proc(pid); // 将子进程的proc信息记录到数组里面
    //    set_proc_name(philosopher_proc_sema[i], "philosopher_sema_proc");
    //}

    //check condition variable
    monitor_init(&mt, N); // 好吧，终于看见调用管程了
    for(i=0;i<N;i++){
        state_condvar[i]=THINKING;
        int pid = kernel_thread(philosopher_using_condvar, (void *)i, 0); // 使用了条件变量
        if (pid <= 0) {
            panic("create No.%d philosopher_using_condvar failed.\n");
        }
        philosopher_proc_condvar[i] = find_proc(pid);
        set_proc_name(philosopher_proc_condvar[i], "philosopher_condvar_proc");
    }
}
