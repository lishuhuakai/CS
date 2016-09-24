#include "coroutine.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#if __APPLE__ && __MACH__
#include <sys/ucontext.h>
#else 
#include <ucontext.h>
#endif 

#define STACK_SIZE (1024*1024) // 表示栈的大小
#define DEFAULT_COROUTINE 16

struct coroutine;

struct schedule {
	char stack[STACK_SIZE]; // 原来schedule里面就已经存有了stack
	ucontext_t main; // ucontext_t你可以看做是记录上下文信息的一个结构
	int nco; // 协程的数目
	int cap; // 容量
	int running; // 正在运行的coroutine的id
	struct coroutine **co; // 这里是一个二维的指针
};

struct coroutine {
	coroutine_func func; // 运行的函数
	void *ud; // 参数
	ucontext_t ctx; // 用于记录上下文信息的一个结构
	struct schedule * sch; // 指向schedule
	ptrdiff_t cap; // 堆栈的容量
	ptrdiff_t size; // 用于表示堆栈的大小
	int status;
	char *stack; // 指向栈地址么？
};

struct coroutine *
	_co_new(struct schedule *S, coroutine_func func, void *ud) { // 用于构建一个coroutine吗？
	struct coroutine * co = malloc(sizeof(*co)); // 新构建一个coroutine结构
	co->func = func; // 记录要运行的函数
	co->ud = ud; // 参数
	co->sch = S; // schedule究竟是用来干什么的？一个指针用于指向S
	co->cap = 0;
	co->size = 0;
	co->status = COROUTINE_READY;
	co->stack = NULL; // 要看到，这里指向的是NULL
	return co;
}

void
_co_delete(struct coroutine *co) {
	free(co->stack);
	free(co);
}

struct schedule *
	coroutine_open(void) { // 用于打开一个coroutine是吧！
	struct schedule *S = malloc(sizeof(*S)); // 居然构建一个schedule
	S->nco = 0; // 表示协程的数目为0
	S->cap = DEFAULT_COROUTINE; // DEFAULT_COROUTINE貌似是16啊！ cap表示容量
	S->running = -1; // -1表示还没有开始运行
	S->co = malloc(sizeof(struct coroutine *) * S->cap); 
	memset(S->co, 0, sizeof(struct coroutine *) * S->cap);
	return S;
}

void
coroutine_close(struct schedule *S) {
	int i;
	for (i = 0; i < S->cap; i++) {
		struct coroutine * co = S->co[i];
		if (co) { // 如果不为空
			_co_delete(co);
		}
	}
	free(S->co);
	S->co = NULL;
	free(S);
}

int
coroutine_new(struct schedule *S, coroutine_func func, void *ud) { // 用于新建一个一个coroutine
	struct coroutine *co = _co_new(S, func, ud); // ud你可以认为是参数
	if (S->nco >= S->cap) { // nco表示协程的数目大于容量了。
		int id = S->cap;
		S->co = realloc(S->co, S->cap * 2 * sizeof(struct coroutine *)); // 用于重新分配
		memset(S->co + S->cap, 0, sizeof(struct coroutine *) * S->cap);
		S->co[S->cap] = co; // 好吧，终于开始装入coroutine了！
		S->cap *= 2; // 因为容量翻倍了嘛
		++S->nco;
		return id;
	}
	else {
		int i;
		for (i = 0; i < S->cap; i++) { // 总之就是不断寻找，找到一个空的位置为止
			int id = (i + S->nco) % S->cap;
			if (S->co[id] == NULL) {
				S->co[id] = co;
				++S->nco;
				return id;
			}
		}
	}
	assert(0);
	return -1;
}

static void
mainfunc(uint32_t low32, uint32_t hi32) {
	uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
	struct schedule *S = (struct schedule *)ptr;
	int id = S->running;
	struct coroutine *C = S->co[id]; // 得到对应的coroutine
	C->func(S, C->ud); // 运行对应的函数, ud是用户传入的参数
	_co_delete(C); // 如果运行完成了，就删除这个coroutine
	S->co[id] = NULL;
	--S->nco;
	S->running = -1;
}

void
coroutine_resume(struct schedule * S, int id) { // 用于恢复coroutine的运行
	assert(S->running == -1);
	assert(id >= 0 && id < S->cap);
	struct coroutine *C = S->co[id]; // 得到对应的协程
	if (C == NULL)
		return;
	int status = C->status;
	switch (status) {
	case COROUTINE_READY: // 如果是才准备好开始执行
		getcontext(&C->ctx); // 初始化一个ctx
		C->ctx.uc_stack.ss_sp = S->stack; // 好吧，设置schedule的stack为coroutine的堆栈，因为足够大
		C->ctx.uc_stack.ss_size = STACK_SIZE;
		C->ctx.uc_link = &S->main; // coroutine运行完成之后默认回到schedule指向的main context继续执行
		S->running = id; // running表示现在正在运行的coroutine的id
		C->status = COROUTINE_RUNNING;
		uintptr_t ptr = (uintptr_t)S; 
		// mainfunc是这个文件里的一个函数
		makecontext(&C->ctx, (void(*)(void)) mainfunc, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
		swapcontext(&S->main, &C->ctx); // 用于切换上下文
		break;
	case COROUTINE_SUSPEND: // 如果从suspend中恢复
		memcpy(S->stack + STACK_SIZE - C->size, C->stack, C->size); // 先恢复堆栈信息
		S->running = id;
		C->status = COROUTINE_RUNNING;
		swapcontext(&S->main, &C->ctx); // 切换上下文
		break;
	default:
		assert(0);
	}
}

static void
_save_stack(struct coroutine *C, char *top) { // 这个函数用于保存堆栈的信息
	// 需要注意的是，栈是从高位向低位生长
	// 所以，top表示的是其实是堆栈的底部，虽然它的地址最高
	char dummy = 0; // 这个真的很有意思，dummy的位置一定是栈栈的顶部，因为现在我们仍然在coroutine的堆栈里
	assert(top - &dummy <= STACK_SIZE); // top - &dummy恰好是栈的大小
	if (C->cap < top - &dummy) { // 第一次保存的时候,C->cap为0,C->stack为NULL
		// 注意区分C->stack和C->ctk->uc_stack.ss_sp，这两者不是同一个东西，前面的C->ctk->uc_stack.ss_sp设置成了schedule的stack
		free(C->stack); // 先清空栈，如果c->stack为NULL，那么free什么也不干，这是规定
		C->cap = top - &dummy;
		C->stack = malloc(C->cap); // 这里出现了malloc
	}
	C->size = top - &dummy;
	// 也就是说&dummy在低位
	memcpy(C->stack, &dummy, C->size); // 其实就是清空C->stack的意思是吧！
}

void
coroutine_yield(struct schedule * S) {
	int id = S->running;
	assert(id >= 0);
	struct coroutine * C = S->co[id];
	assert((char *)&C > S->stack);
	_save_stack(C, S->stack + STACK_SIZE); // 用于保存堆栈的信息
	C->status = COROUTINE_SUSPEND; // 状态变成了挂起
	S->running = -1;
	swapcontext(&C->ctx, &S->main);
}

int
coroutine_status(struct schedule * S, int id) {
	assert(id >= 0 && id < S->cap);
	if (S->co[id] == NULL) {
		return COROUTINE_DEAD; // COROUTINE_DEAD返回0
	}
	return S->co[id]->status;
}

int
coroutine_running(struct schedule * S) {
	return S->running;
}

