#include "common.h"
#include "console.h"
#include "string.h"
#include "debug.h"
#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "task.h"
#include "sched.h"

// 内核初始化函数
void kern_init();

// 开启分页机制后的Multiboot数据指针
multiboot_t *glb_mboot_ptr;

// 开启分页机制之后的内核栈
char kern_stack[STACK_SIZE]  __attribute__ ((aligned(16)));

// 内核栈的栈顶
uint32_t kern_stack_top;

// 内核使用的临时页表和页地址
// 该地址必须是页对齐的地址，内存0 - 640Kb肯定是空闲的
__attribute__((section(".init.data"))) pgd_t *pgd_tmp  = (pgd_t *)0x1000;
__attribute__((section(".init.data"))) pgd_t *pte_low  = (pgd_t *)0x2000;
__attribute__((section(".init.data"))) pgd_t *pte_high = (pgd_t *)0x3000;

// 内核入口函数
__attribute__((section(".init.text"))) void kern_entry()
{
	// 好吧，临时的页目录表，一共有两个页目录
	pgd_tmp[0] = (uint32_t)pte_low | PAGE_PRESENT | PAGE_WRITE;
	// #define PAGE_OFFSET 0xc0000000
	// #define PGD_INDEX(x) (((x) >> 22) & 0x3FF)
	// 一共32位嘛，PGD_INDEX应该是取高10位，然后作为下面的索引
	pgd_tmp[PGD_INDEX(PAGE_OFFSET)] = (uint32_t)pte_high | PAGE_PRESENT | PAGE_WRITE;

	// 映射内核虚拟地址4MB到物理地址的前4MB
	int i;
	// 这一段，应该是将0x00000000 - 0x00400000 映射到了自己
	for (i = 0; i < 1024; i++) {
		// 从这里开始真正映射了
		pte_low[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
	}

	// 映射0x00000000-0x00400000的物理地址到虚拟地址0xC0000000-0xC0400000
	for (i = 0; i < 1024; i++) {
		pte_high[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
	}
	// 我总是感觉这里有重复的样子
	// 设置临时页表
	asm volatile ("mov %0, %%cr3" : : "r" (pgd_tmp));

	uint32_t cr0;

	// 启用分页，将cr0寄存器的分页位置为1就好
	asm volatile("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000;
	asm volatile ("mov %0, %%cr0" : : "r" (cr0));
	
	// 切换内核栈
	// 从现在开始，我们使用的就是虚拟地址了！
	// STACK_SIZE = 8192
	kern_stack_top = ((uint32_t)kern_stack + STACK_SIZE);
	asm volatile ("mov %0, %%esp\n\t"
			"xor %%ebp, %%ebp" : : "r" (kern_stack_top));

	// 更新全局multiboot_t指针
	glb_mboot_ptr = mboot_ptr_tmp + PAGE_OFFSET; // 指向虚拟地址

	// 调用内核初始化函数
	kern_init();
}

int flag = 0;
int thread(void *arg)
{
	while (1) {
		if (flag == 1) {
			printk_color(rc_black, rc_green, "B");
			flag = 0;
		}
	}

	return 0;
}

void kern_init()
{
	init_debug();
	init_gdt();
	init_idt();

	console_clear();
	printk_color(rc_black, rc_green, "Hello, OS kernel!\n\n");

	// 开启中断
	//asm volatile ("sti");
	init_timer(200);

	printk("kernel in memory start: 0x%08X\n", kern_start);
	printk("kernel in memory end:   0x%08X\n", kern_end);
	printk("kernel in memory used:   %d KB\n\n", (kern_end - kern_start) / 1024);
	
	// show_memory_map();
	init_pmm();
	init_vmm();
	init_heap();

	//printk_color(rc_black, rc_red, "\nThe Count of Physical Memory Page is: %u\n\n", phy_page_count);

	//uint32_t allc_addr = NULL;
	//printk_color(rc_black, rc_light_brown, "Test Physical Memory Alloc :\n");
	//allc_addr = pmm_alloc_page();
	//printk_color(rc_black, rc_light_brown, "Alloc Physical Addr: 0x%08X\n", allc_addr);
	//allc_addr = pmm_alloc_page();
	//printk_color(rc_black, rc_light_brown, "Alloc Physical Addr: 0x%08X\n", allc_addr);
	//allc_addr = pmm_alloc_page();
	//printk_color(rc_black, rc_light_brown, "Alloc Physical Addr: 0x%08X\n", allc_addr);
	//allc_addr = pmm_alloc_page();
	//printk_color(rc_black, rc_light_brown, "Alloc Physical Addr: 0x%08X\n", allc_addr);
	printk_color(rc_black, rc_red, "\n The Count of Physical Memory Page is: %u\n\n", phy_page_count);

	test_heap();

	init_sched();

	kernel_thread(thread, NULL);

	// 开启中断
	enable_intr();

	while (1) {
		if (flag == 0) {
			printk_color(rc_black, rc_red, "A");
			flag = 1;
		}
	}
	while (1) {
		asm volatile ("hlt");
	}
}




