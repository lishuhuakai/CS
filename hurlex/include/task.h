#ifndef IINCLUDE_TASK_H_
#define IINCLUDE_TASK_H_

#include "types.h"
#include "pmm.h"
#include "vmm.h"

// 进程状态描述
typedef
enum task_state {
	TASK_UNINIT = 0,	// 未初始化
	TASK_SLEEPING = 1, 	// 睡眠中
	TASK_RUNNABLE = 2,	// 可运行也许正在运行
	TASK_ZOMBLE = 3,	// 僵死状态
} task_state;

// 内核线程的上下文切换保存的信息
struct context {
	uint32_t esp;
	uint32_t ebp;
	uint32_t ebx;
	uint32_t esi;
	uint32_t edi;
	uint32_t eflags;
};


// 进程内存地址结构
struct mm_struct {
	pgd_t *pgd_dir;		// 进程页表
};

// 进程控制块pcb
struct task_struct {
	volatile task_state state;	// 用于记录进程的当前状态
	pid_t	pid;			// 进程标识符
	void *stack;			// 进程的内核栈地址
	struct mm_struct *mm;		// 当前进程的内存地址
	struct context context;		// 进程切换需要的上下文信息
	struct task_struct *next;	// 链表指针
};

// 全局pid值
extern pid_t now_pid;

// 内核线程创建
int32_t kernel_thread(int (*fn)(void *), void *arg);

// 线程退出函数
void kthread_exit();

#endif
