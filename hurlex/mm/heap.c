#include "debug.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

// 用于申请内存块
static void alloc_chunk(uint32_t start, uint32_t len);

// 释放内存块
static void free_chunk(header_t *chunk);

// 切分内存块
static void split_chunk(header_t *chunk, uint32_t len);

// 合并内存块
static void glue_chunk(header_t *chunk);

// 内存块管理头指针
static header_t *heap_first;

static uint32_t heap_max = HEAP_START;

void init_heap()
{
	heap_first = 0;
}
/*
void *kmalloc(uint32_t len)
{
	// 所有申请的内存长度加上管理头的长度
	// 因为在内存申请和释放的时候要通过该结构去管理
	len += sizeof(header_t);

	header_t *cur_header = heap_first;
	header_t *prev_header = 0;

	while (cur_header) { // 如果cur_header不为空的话
		// 如果当前内存块没有被申请过且长度大于待申请的块
		if (cur_header->allocated == 0 && cur_header->length >= len) {
			// 就按照当前的长度切割内存
			split_chunk(cur_header, len); // 好吧，进行切分
			cur_header->allocated = 1;
			// 返回的时候必须将指针挪到管理结构之后
			return (void *)((uint32_t)cur_header + sizeof(header_t)); // 返回实际可用的地址
		}
		// 逐次推移指针
		prev_header = cur_header;
		cur_header = cur_header->next;
	}

	uint32_t chunk_start;

	// 第一次执行该函数则初始化内存块起始位置
	// 之后根据当前指针加上申请的长度即可
	if (prev_header) {
		chunk_start = (uint32_t)prev_header + prev_header->length; // 从哪里开始的，是吧！
	} else { // 好吧，这些都是虚拟地址
		// 如果prev_header为空的话，也就是第一次开始分配内存
		chunk_start = HEAP_START; // 从HEAP_START开始分配
		heap_first = (header_t *)chunk_start;
	}

	// 检查是否需要申请内存页
	alloc_chunk(chunk_start, len); 

	
	cur_header = (header_t *)chunk_start;
	cur_header->prev = prev_header;
	cur_header->next = 0;
	cur_header->allocated = 1;
	cur_header->length = len;

	if (prev_header) {
		// 到这里为止，貌似只是为了将一切连贯起来
		prev_header->next = cur_header;
	}
	return (void*)(chunk_start + sizeof(header_t)); // 返回实际可用的地址
}
*/
void *kmalloc(uint32_t len)
{
	// 所有申请的内存长度加上管理头的长度
	// 因为在内存申请和释放的时候要通过该结构去管理
	len += sizeof(header_t);

	header_t *cur_header = heap_first;
	header_t *prev_header = 0;

	while (cur_header) {
		// 如果当前内存块没有被申请过而且长度大于待申请的块
		if (cur_header->allocated == 0 && cur_header->length >= len) {
			// 按照当前长度切割内存
			split_chunk(cur_header, len);
			cur_header->allocated = 1;
			// 返回的时候必须将指针挪到管理结构之后
			return (void *)((uint32_t)cur_header + sizeof(header_t));
		}
		// 逐次推移指针
		prev_header = cur_header;
		cur_header = cur_header->next;
	}

	uint32_t chunk_start;

	// 第一次执行该函数则初始化内存块起始位置
	// 之后根据当前指针加上申请的长度即可
	if (prev_header) {
		chunk_start = (uint32_t)prev_header + prev_header->length;
	} else {
		chunk_start = HEAP_START;
		heap_first = (header_t *)chunk_start;
	}

	// 检查是否需要申请内存页
	alloc_chunk(chunk_start, len);
	cur_header = (header_t *)chunk_start;
	cur_header->prev = prev_header;
	cur_header->next = 0;
	cur_header->allocated = 1;
	cur_header->length = len;
	
	if (prev_header) {
		prev_header->next = cur_header;
	}

	return (void*)(chunk_start + sizeof(header_t));
}

void kfree(void *p)
{
	// 指针回退到管理结构，并将已使用标记置0
	header_t *header = (header_t *)((uint32_t)p - sizeof(header_t));
	header->allocated = 0;
	// 粘合内存块
	glue_chunk(header);
}

void alloc_chunk(uint32_t start, uint32_t len)
{
	// 如果当前堆的位置已经到达界限，则申请内存页
	// 必须循环申请内存页直到有到足够的可用内存
	while (start + len > heap_max) {
		uint32_t page = pmm_alloc_page(); // 好吧，得到一个4kb的页
		map(pgd_kern, heap_max, page, PAGE_PRESENT | PAGE_WRITE);
		heap_max += PAGE_SIZE; // page_size就是4kb啦
	}

}

void free_chunk(header_t *chunk)
{
	if (chunk->prev == 0) { // 第一次申请，第一次释放
		heap_first = 0;
	} else {
		chunk->prev->next = 0;
	}

	// 我记得堆的话，是从底向上生长
	
	// 空闲的内存超过1页的话就释放掉
	// heap_max这个玩意究竟是什么？
	while ((heap_max - PAGE_SIZE) >= (uint32_t)chunk) {
		heap_max -= PAGE_SIZE;
		uint32_t page;
		get_mapping(pgd_kern, heap_max, &page); // 得到实际的地址
		unmap(pgd_kern, heap_max);
		pmm_free_page(page);
	}

}

void split_chunk(header_t *chunk, uint32_t len)
{
	// 切分内存块之前得保证之后的剩余内存至少容纳一个内存管理块的大小
	if (chunk->length - len > sizeof(header_t)) {
		header_t *newchunk = (header_t *)((uint32_t)chunk + chunk->length);
		newchunk->prev = chunk;
		newchunk->next = chunk->next;
		newchunk->allocated = 0;
		newchunk->length = chunk->length - len;

		chunk->next = newchunk;
		chunk->length = len;
	}

}

void glue_chunk(header_t *chunk)
{
	// 如果该内存块前面有链内存块且为被使用则拼合
	if (chunk->next && chunk->next->allocated == 0) {
		chunk->length = chunk->length + chunk->next->length;
		if (chunk->next->next) {
			chunk->next->next->prev = chunk;
		}
		chunk->next = chunk->next->next;
	}

	if (chunk->prev && chunk->prev->allocated == 0) {
		chunk->prev->length = chunk->prev->length + chunk->length;
		if (chunk->next) {
			chunk->next->prev = chunk->prev;
		}
		chunk = chunk->prev;
	}
	// 假如内存后面没有链表内存块了，则直接释放掉
	if (chunk->next == 0) {
		free_chunk(chunk);
	}
}

void test_heap()
{
	printk_color(rc_black, rc_magenta, "Test kmalloc() && kfree() now ...\n\n");
	void *addr1 = kmalloc(50);
	printk("kmalloc 50 byte in 0x%X\n", addr1);
	void *addr2 = kmalloc(500);
	printk("kmalloc 500 byte in ox%X\n", addr2);
	void *addr3 = kmalloc(5000);
	printk("kmalloc 5000 byte in 0x%X\n", addr3);
	void *addr4 = kmalloc(50000);
	printk("kmalloc 50000 byte in 0x%X\n", addr4);

	printk("free mem in 0x%X\n", addr1);
	kfree(addr1);

	printk("free mem in 0x%X\n", addr2);
	kfree(addr2);
	printk("free mem in 0x%X\n", addr3);
	kfree(addr3);
	printk("free mem in 0x%X\n", addr4);
	kfree(addr4);
}


