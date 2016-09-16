#ifndef INCLUDE_HEAP_H_
#define INCLUDE_HEAP_H_

#include "types.h"
// 堆起始地址
#define HEAP_START 0xE0000000 // 这里指的应该是实际的物理地址吧

// 内存块管理结构
typedef
struct header {
	struct header *prev;  // 前后内存块管理结构指针
	struct header *next;
	uint32_t allocated : 1; // 该内存块是否已经被申请
	uint32_t length : 31; // 当前内存块的长度
} header_t;

// 初始化堆
void init_heap();

// 内存申请
void *kmalloc(uint32_t len);

// 内存释放
void kfree(void *p);

// 测试内核堆申请释放
void test_heap();

#endif
