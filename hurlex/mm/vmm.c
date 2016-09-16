#include "idt.h"
#include "string.h"
#include "debug.h"
#include "vmm.h"
#include "pmm.h"

// 内核页目录区域
pgd_t pgd_kern[PGD_SIZE] __attribute__ ((aligned(PAGE_SIZE)));

// 内核页表区域
static pte_t pte_kern[PTE_COUNT][PTE_SIZE] __attribute__ ((aligned(PAGE_SIZE)));

void init_vmm()
{
	// 0xc0000000 这个地址在页目录的索引
	uint32_t kern_pte_first_idx = PGD_INDEX(PAGE_OFFSET);

	uint32_t i, j;
	for (i = kern_pte_first_idx, j = 0; i < PTE_COUNT + kern_pte_first_idx; i++, j++)
	{
		// 此处是内核虚拟地址, MMU需要物理地址，所以减去偏移，下同
		pgd_kern[i] = ((uint32_t)pte_kern[j] - PAGE_OFFSET) | PAGE_PRESENT | PAGE_WRITE;

	}

	uint32_t *pte = (uint32_t *)pte_kern;
	// 不映射第0页，便于跟踪NULL指针
	// 下面是设置页表项 i << 12代表页基址，从这里看的话，一个表项4kb
	
	for (i = 1; i < PTE_COUNT * PTE_SIZE; i++) {
		pte[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
	}
	
	// 表示的是物理地址吗？ PAGE_OFFSET我记得貌似是0xc0000000
	// pgd_kern指的是页目录的地址(虚拟地址)
	// 下面得到物理地址，有什么区别吗？
	uint32_t pgd_kern_phy_addr = (uint32_t)pgd_kern - PAGE_OFFSET;

	// 注册页错误中断的处理函数(14是页故障的中断号)
	register_interrupt_handler(14, &page_fault);
	switch_pgd(pgd_kern_phy_addr);
}

void switch_pgd(uint32_t pd)
{
	// 用于更换页目录表项
	asm volatile ("mov %0, %%cr3" : : "r" (pd));
}

void map(pgd_t *pgd_now, uint32_t va, uint32_t pa, uint32_t flags)
{
	// va表示的是虚拟地址
	// PGD_INDEX用于获取一个地址的目录项
	uint32_t pgd_idx = PGD_INDEX(va);
	// PTE_INDEX用于获取一个地址的页表项
	uint32_t pte_idx = PTE_INDEX(va);

	// pte是page table entry的一个简称，是吧！
	// #define PAGE_MASK	0xFFFFF000
	pte_t *pte = (pte_t *)(pgd_now[pgd_idx] & PAGE_MASK); // 这个用于从页目录表中获取该虚拟地址对应的页表的地址
	if (!pte) { // 也就是还没有这个表项
		pte = (pte_t *)pmm_alloc_page(); // 随机分配一个页面
		// 好吧，在这里我们就看到了很有意思的东西了，这里就是所谓的映射，可以随便映射到那一块，是吧！
		pgd_now[pgd_idx] = (uint32_t)pte | PAGE_PRESENT | PAGE_WRITE;

		// 转换到内核线性地址并清0
		pte = (pte_t *) ((uint32_t)pte + PAGE_OFFSET);
		bzero(pte, PAGE_SIZE); // 好一个映射啊！
	} else {
		// 转换到内核线性地址
		pte = (pte_t *) ((uint32_t)pte + PAGE_OFFSET);
	}

	// #define PAGE_MASK	0xFFFFF000
	// pte是页表项，pa & PAGE_MASK得到基址,也就是--低12位全为0,低12位是页内的偏移吗？
	pte[pte_idx] = (pa & PAGE_MASK) | flags; // 这个是要干嘛？
	// 通知cpu更新页表缓存
	asm volatile ("invlpg (%0)" : : "a"(va));
}

void unmap(pgd_t *pgd_now, uint32_t va)
{
	// 首先是得到虚拟地址va在对应的页目录的索引
	uint32_t pgd_idx = PGD_INDEX(va);
	// 接下来是得到虚拟地址va对应的在页表中的索引
	uint32_t pte_idx = PTE_INDEX(va);

	// 得到这个页表的基址
	pte_t *pte = (pte_t *)(pgd_now[pgd_idx] & PAGE_MASK);

	if (!pte) { // 都没有这个页表项
		return; 
	}

	// 切换到内核线性地址
	pte = (pte_t *) ((uint32_t)pte + PAGE_OFFSET);
	
	// 将对应的页面清空
	pte[pte_idx] = 0;

	// 通知cpu更新页表缓存
	asm volatile ("invlpg (%0)" : : "a" (va));
}

// 我想，我大概懂了分页机制的是怎样来实现的了，以及它所带来的好处了。
uint32_t get_mapping(pgd_t *pgd_now, uint32_t va, uint32_t *pa)
{
	// 得到虚拟地址va在对应的页目录中的索引
	uint32_t pgd_idx = PGD_INDEX(va);
	// 得到虚拟地址va在对应的页表中的索引
	uint32_t pte_idx = PTE_INDEX(va);

	// 首先是得到虚拟地址va在对应的页表的基址
	pte_t *pte = (pte_t *)(pgd_now[pgd_idx] & PAGE_MASK);
	if (!pte) { // 如果这个页表并没有指向一个实际的地址
		return 0;
	}

	// 切换到内核线性地址
	pte = (pte_t *)((uint32_t)pte + PAGE_OFFSET);

	// 如果地址有效而且指针不为0，则返回地址NULL
	if (pte[pte_idx] != 0 && pa) {
		//#define PAGE_MASK	0xFFFFF000,也就是取得对应虚拟地址对应的实际地址的基址
		*pa = pte[pte_idx] & PAGE_MASK;
		return 1; 
	}
	return 0;
}
