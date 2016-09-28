#include <defs.h>
#include <x86.h>
#include <stdio.h>
#include <string.h>
#include <mmu.h>
#include <memlayout.h>
#include <pmm.h>
#include <default_pmm.h>
#include <sync.h>
#include <error.h>
#include <swap.h>
#include <vmm.h>

/* *
 * Task State Segment:
 *
 * The TSS may reside anywhere in memory. A special segment register called
 * the Task Register (TR) holds a segment selector that points a valid TSS
 * segment descriptor which resides in the GDT. Therefore, to use a TSS
 * the following must be done in function gdt_init:
 *   - create a TSS descriptor entry in GDT
 *   - add enough information to the TSS in memory as needed
 *   - load the TR register with a segment selector for that segment
 *
 * There are several fileds in TSS for specifying the new stack pointer when a
 * privilege level change happens. But only the fields SS0 and ESP0 are useful
 * in our os kernel.
 *
 * The field SS0 contains the stack segment selector for CPL = 0, and the ESP0
 * contains the new ESP value for CPL = 0. When an interrupt happens in protected
 * mode, the x86 CPU will look in the TSS for SS0 and ESP0 and load their value
 * into SS and ESP respectively.
 * */
static struct taskstate ts = {0}; // 用于记录任务的状态吗？

// virtual address of physicall page array
struct Page *pages; // 虚拟地址
// amount of physical memory (in pages)
size_t npage = 0; // 物理内存被分成了多少个页

// virtual address of boot-time page directory
pde_t *boot_pgdir = NULL; // page directory entry 用于指向页表的一个指针
// physical address of boot-time page directory
uintptr_t boot_cr3;

// physical memory management
const struct pmm_manager *pmm_manager;

/* *
 * The page directory entry corresponding to the virtual address range
 * [VPT, VPT + PTSIZE) points to the page directory itself. Thus, the page
 * directory is treated as a page table as well as a page directory.
 *
 * One result of treating the page directory as a page table is that all PTEs
 * can be accessed though a "virtual page table" at virtual address VPT. And the
 * PTE for number n is stored in vpt[n].
 *
 * A second consequence is that the contents of the current page directory will
 * always available at virtual address PGADDR(PDX(VPT), PDX(VPT), 0), to which
 * vpd is set bellow.
 * */
 
 // 其实很有意思的是这个vpt，定义在memlayout.h这个文件里面
 // #define VPT  0xFAC00000
 // 是一个虚拟地址va
pte_t * const vpt = (pte_t *)VPT; 
// PDX用于获取page table index也就是高10位，
pde_t * const vpd = (pde_t *)PGADDR(PDX(VPT), PDX(VPT), 0); // PGADDR用于构建一个虚拟地址

/* *
 * Global Descriptor Table:
 *
 * The kernel and user segments are identical (except for the DPL). To load
 * the %ss register, the CPL must equal the DPL. Thus, we must duplicate the
 * segments for the user and the kernel. Defined as follows:
 *   - 0x0 :  unused (always faults -- for trapping NULL far pointers)
 *   - 0x8 :  kernel code segment
 *   - 0x10:  kernel data segment
 *   - 0x18:  user code segment
 *   - 0x20:  user data segment
 *   - 0x28:  defined for tss, initialized in gdt_init
 * */
static struct segdesc gdt[] = {
    SEG_NULL,
    [SEG_KTEXT] = SEG(STA_X | STA_R, 0x0, 0xFFFFFFFF, DPL_KERNEL), // kernel 的代码段基址从0x0000开始，这是虚拟地址吧？
    [SEG_KDATA] = SEG(STA_W, 0x0, 0xFFFFFFFF, DPL_KERNEL),
    [SEG_UTEXT] = SEG(STA_X | STA_R, 0x0, 0xFFFFFFFF, DPL_USER), // 好吧，感觉都差不多，是一对一的映射
    [SEG_UDATA] = SEG(STA_W, 0x0, 0xFFFFFFFF, DPL_USER),
    [SEG_TSS]   = SEG_NULL,
};

static struct pseudodesc gdt_pd = {
    sizeof(gdt) - 1, (uintptr_t)gdt
};

static void check_alloc_page(void);
static void check_pgdir(void);
static void check_boot_pgdir(void);

/* *
 * lgdt - load the global descriptor table register and reset the
 * data/code segement registers for kernel.
 * */
static inline void
lgdt(struct pseudodesc *pd) {
    asm volatile ("lgdt (%0)" :: "r" (pd));
    asm volatile ("movw %%ax, %%gs" :: "a" (USER_DS));
    asm volatile ("movw %%ax, %%fs" :: "a" (USER_DS));
    asm volatile ("movw %%ax, %%es" :: "a" (KERNEL_DS));
    asm volatile ("movw %%ax, %%ds" :: "a" (KERNEL_DS));
    asm volatile ("movw %%ax, %%ss" :: "a" (KERNEL_DS));
    // reload cs
    asm volatile ("ljmp %0, $1f\n 1:\n" :: "i" (KERNEL_CS));
}

/* *
 * load_esp0 - change the ESP0 in default task state segment,
 * so that we can use different kernel stack when we trap frame
 * user to kernel.
 * */
 
// ts是一个taskstate的结构
void
load_esp0(uintptr_t esp0) {
    ts.ts_esp0 = esp0; // 一个task的esp0
}

/* gdt_init - initialize the default GDT and TSS */
static void
gdt_init(void) {
    // set boot kernel stack and default SS0
	// 我们来看一下全局的描述符吧！
    load_esp0((uintptr_t)bootstacktop); // bootstacktop估计是内核的栈顶吧！
    ts.ts_ss0 = KERNEL_DS; // 段选择子

    // initialize the TSS filed of the gdt
    gdt[SEG_TSS] = SEGTSS(STS_T32A, (uintptr_t)&ts, sizeof(ts), DPL_KERNEL);

    // reload all segment registers
    lgdt(&gdt_pd); // 然后开始加载gdt_pd

    // load the TSS
    ltr(GD_TSS);
}

//init_pmm_manager - initialize a pmm_manager instance
static void
init_pmm_manager(void) {
	
    pmm_manager = &default_pmm_manager;
    cprintf("memory management: %s\n", pmm_manager->name);
    pmm_manager->init(); // 关键是看这个玩意如何初始化的了。
}

//init_memmap - call pmm->init_memmap to build Page struct for free memory  
// 好吧，我们来看一下吧，也就是说，调用init_memmap来为空闲的内存来构建Page struct
static void
init_memmap(struct Page *base, size_t n) {
    pmm_manager->init_memmap(base, n);
}

//alloc_pages - call pmm->alloc_pages to allocate a continuous n*PAGESIZE memory 
// 分配页面,连续的内存
struct Page *
alloc_pages(size_t n) {
    struct Page *page = NULL;
    bool intr_flag;
    
    while (1)
    {
    	local_intr_save(intr_flag); // 这些指令都是啥意思?我查了一下，貌似是关闭中断的意思。
         {
              page = pmm_manager->alloc_pages(n);
         }
    	local_intr_restore(intr_flag); // 分配完成之后再开启中断

         if (page != NULL || n > 1 || swap_init_ok == 0) break; // 如果申请不了的话，就要换页了，一直到
         
         extern struct mm_struct *check_mm_struct;
         //cprintf("page %x, call swap_out in alloc_pages %d\n",page, n);
         swap_out(check_mm_struct, n, 0); // 前面申请不到页面了，所以要换出
    }
    //cprintf("n %d,get page %x, No %d in alloc_pages\n",n,page,(page-pages));
    return page;
}

//free_pages - call pmm->free_pages to free a continuous n*PAGESIZE memory 
void
free_pages(struct Page *base, size_t n) {
    bool intr_flag; 
    local_intr_save(intr_flag);
    {
        pmm_manager->free_pages(base, n);
    }
    local_intr_restore(intr_flag);
}

//nr_free_pages - call pmm->nr_free_pages to get the size (nr*PAGESIZE) 
//of current free memory
size_t
nr_free_pages(void) {
    size_t ret;
    bool intr_flag;
    local_intr_save(intr_flag);
    {
        ret = pmm_manager->nr_free_pages();
    }
    local_intr_restore(intr_flag);
    return ret; // ret是释放的？？？
}

/* pmm_init - initialize the physical memory management */
// 我当初好像看见过这个函数的调用
static void
page_init(void) {
	// 这个函数是用来初始化page的吧！
	// 运行到了这一步的话，已经进入了保护模式，然后加载了选择子
	// KERNBASE = 0xC0000000
	// 1100 0000 0000 0000 0000 0000 0000 0000 一共32位
	// 好吧，我想，我读懂了，我们现在给出的地址其实是虚拟地址，也就是前10位是段选择子
	// 什么时候多了一个0xc0000000
	// 我来解释一下吧，我们前面知道，这个结构的实际物理地址应该是0x8000,但是由于段选择器的存在，实际的虚拟地址是va=pa - 0xC0000000，因此，为了正确访问到0x8000的物理地址
	// 也就是说，pa=0x8000 = (0x8000+KERNBASE)-KERNBASE，所以就是这样喽
    struct e820map *memmap = (struct e820map *)(0x8000 + KERNBASE); 
	// 话说，e820真的是用来获取内存信息的。
    uint64_t maxpa = 0; // max page address?

    cprintf("e820map:\n");
    int i;
	// 我们来看一下定义吧！
	// #define KMEMSIZE            0x38000000   指的是kernel的最大的物理内存的数目
    for (i = 0; i < memmap->nr_map; i ++) {
        uint64_t begin = memmap->map[i].addr, end = begin + memmap->map[i].size;
        cprintf("  memory: %08llx, [%08llx, %08llx], type = %d.\n",
                memmap->map[i].size, begin, end - 1, memmap->map[i].type);
        if (memmap->map[i].type == E820_ARM) { // E820_ARM表示，这段内存是可以被OS使用的ram
            if (maxpa < end && begin < KMEMSIZE) {
                maxpa = end; // 这里的maxpa指的是物理地址
            }
        }
    }
    if (maxpa > KMEMSIZE) {
        maxpa = KMEMSIZE;
    }

    extern char end[];
	// end定义在kernel.ld文件里面

    npage = maxpa / PGSIZE; // 一共的页数
	// pages是这张页表的位置是位于end之后，应该是虚拟地址，是吧！
	// PGSIZE是4kb
	// pages也是物理地址
    pages = (struct Page *)ROUNDUP((void *)end, PGSIZE);

	// 令我感到好奇的是，这些page压根就没有连接在一起是吧，虽然他们是数组一样聚集在一起的。
    for (i = 0; i < npage; i ++) { // 好吧，我们来看一下，这个函数究竟干了一些什么事情
        SetPageReserved(pages + i); // 用于表示这些页面都已经被占用了
    }

    uintptr_t freemem = PADDR((uintptr_t)pages + sizeof(struct Page) * npage); // 得到空表后的实际的空闲的块的首地址

    for (i = 0; i < memmap->nr_map; i ++) { // nr_map是这样的结构的个数
        uint64_t begin = memmap->map[i].addr, end = begin + memmap->map[i].size;
		// begin指的是开始的地方，end指的是这段内存结束的地方
        if (memmap->map[i].type == E820_ARM) { // 这段内存是可以被OS使用的ram
            if (begin < freemem) { 
                begin = freemem;
            }
            if (end > KMEMSIZE) {
                end = KMEMSIZE; // 这里实际上是要找一段区间，在freemem和KMEMSIZE之间
            }
            if (begin < end) {
                begin = ROUNDUP(begin, PGSIZE);
                end = ROUNDDOWN(end, PGSIZE);
                if (begin < end) {
					//  pa2page(begin)返回一个begin这个物理地址对应的Page结构
                    init_memmap(pa2page(begin), (end - begin) / PGSIZE); // 大概的意思是，将这些页全部设置为可用
                }
            }
        }
    }
}

// 好吧，其实就是开启分页
static void
enable_paging(void) {
	// boot_cr3记录的是页目录表的起始地址，是物理地址
    lcr3(boot_cr3); // 将页目录表的起始地址写入cr3寄存器

    // turn on paging
    uint32_t cr0 = rcr0(); // 得到cr0的值是吧。
    cr0 |= CR0_PE | CR0_PG | CR0_AM | CR0_WP | CR0_NE | CR0_TS | CR0_EM | CR0_MP;
    cr0 &= ~(CR0_TS | CR0_EM);
    lcr0(cr0); // 从此开启了分页
}

//boot_map_segment - setup&enable the paging mechanism
// parameters
//  la:   linear address of this memory need to map (after x86 segment map)
//  size: memory size
//  pa:   physical address of this memory
//  perm: permission of this memory  

// 好吧，这个函数其实就是用来映射的
static void
boot_map_segment(pde_t *pgdir, uintptr_t la, size_t size, uintptr_t pa, uint32_t perm) {
	// 我其实就是想看一看这个函数究竟是如何来实现映射的,下面是调用的一个过程
	// boot_map_segment(boot_pgdir, KERNBASE, KMEMSIZE, 0, PTE_W);
	// PGOFF应该是Page offset的缩写，表示页内的偏移
    assert(PGOFF(la) == PGOFF(pa));
    size_t n = ROUNDUP(size + PGOFF(la), PGSIZE) / PGSIZE;
    la = ROUNDDOWN(la, PGSIZE); // la是虚拟地址的起始
	// la确实容易混淆，因为一会儿是logic address，一会儿又是linear address
    pa = ROUNDDOWN(pa, PGSIZE); // pa是线性地址吧！
    for (; n > 0; n --, la += PGSIZE, pa += PGSIZE) {
        pte_t *ptep = get_pte(pgdir, la, 1); // page table entry
        assert(ptep != NULL);
        *ptep = pa | PTE_P | perm; // pa应该是physical address吧！
    }
	// 基本上算是懂了，这个玩意起始就是要实现一个映射而已。
}

//boot_alloc_page - allocate one page using pmm->alloc_pages(1) 
// return value: the kernel virtual address of this allocated page
//note: this function is used to get the memory for PDT(Page Directory Table)&PT(Page Table)

// 用来分配页
static void *
boot_alloc_page(void) {
    struct Page *p = alloc_page();
    if (p == NULL) {
        panic("boot_alloc_page failed.\n");
    }
	// 我们来看一下page2Kva函数吧！
    return page2kva(p); // 应该是返回虚拟的地址吧！
}

//pmm_init - setup a pmm to manage physical memory, build PDT&PT to setup paging mechanism 
//         - check the correctness of pmm & paging mechanism, print PDT&PT
void
pmm_init(void) {
    //We need to alloc/free the physical memory (granularity is 4KB or other size). 
    //So a framework of physical memory manager (struct pmm_manager)is defined in pmm.h
    //First we should init a physical memory manager(pmm) based on the framework.
    //Then pmm can alloc/free the physical memory. 
    //Now the first_fit/best_fit/worst_fit/buddy_system pmm are available.
    init_pmm_manager(); // 好吧，这个玩意大概干了这么一些事情：
	// 将freelist置空

    // detect physical memory space, reserve already used memory,
    // then use pmm->init_memmap to create free page list
    page_init();

    //use pmm->check to verify the correctness of the alloc/free function in a pmm
    check_alloc_page();

    // create boot_pgdir, an initial page directory(Page Directory Table, PDT)
    boot_pgdir = boot_alloc_page(); // 得到了分配的虚拟地址,好吧，根据变量的名称，我们恐怕是会将这个页作为page directory
    memset(boot_pgdir, 0, PGSIZE); // 全部清零
    boot_cr3 = PADDR(boot_pgdir); // boot_cr3是boot_pgdir的实际的物理地址
	cprintf("%s\n", "hello, I am yihulee!");
    check_pgdir();

    static_assert(KERNBASE % PTSIZE == 0 && KERNTOP % PTSIZE == 0);

    // recursively insert boot_pgdir in itself
    // to form a virtual page table at virtual address VPT
	// PDX取得高地址
	// PADDR取得虚拟地址对应的物理地址
	// 下面的语句非常有意思，用一个page table entry指向自己
    boot_pgdir[PDX(VPT)] = PADDR(boot_pgdir) | PTE_P | PTE_W;

    // map all physical memory to linear memory with base linear addr KERNBASE
    //linear_addr KERNBASE~KERNBASE+KMEMSIZE = phy_addr 0~KMEMSIZE
    //But shouldn't use this map until enable_paging() & gdt_init() finished.
	// 下面这个函数实现了非常漂亮的映射
    boot_map_segment(boot_pgdir, KERNBASE, KMEMSIZE, 0, PTE_W); 

    //temporary map: 
    //virtual_addr 3G~3G+4M = linear_addr 0~4M = linear_addr 3G~3G+4M = phy_addr 0~4M    
	// 什么叫做暂时的映射
	// 物理地址0~4M被映射到了线性地址0~4M,同时也被映射到了线性地址3G~3G+4M
	// 之所以这么干，是因为下面的函数一旦打开了分页机制之后，cpu立即按照分页机制来寻址，请记住，我们的代码还是按照原来的虚拟地址来编码的
	// 
    boot_pgdir[0] = boot_pgdir[PDX(KERNBASE)];

    enable_paging(); // 现在要开启分页了。
	// 虽然说开启了分页模式，但是到了这里的话，我想阐述一下现在的状况
	// 首先在执行kern_entry的时候，kern_entry函数设置了临时的新的段映射机制
	// 从而导致了线性地址和虚拟地址的不相等，而刚才建立的页映射关系，是建立在简单的段对等映射，即虚拟地址=线性地址的假设之上的
	// 所以我们需要进一步调整段映射关系，即重新设置新的GDT，建立对等段映射

	// 进入分页模式到重新设置新GDT的过程是一个过渡的过程。在这个过渡过程之中，已经建立了页表机制，所以通过现在的段机制和
	// 页机制实现的地址映射如下
	// Virtual Address = Linear Address + 0xC0000000 = Physical Address + 0xC0000000 + 0xC0000000



    //reload gdt(third time,the last time) to map all physical memory
    //virtual_addr 0~4G=liear_addr 0~4G
    //then set kernel stack(ss:esp) in TSS, setup TSS in gdt, load TSS
    gdt_init();

    //disable the map of virtual_addr 0~4M
    boot_pgdir[0] = 0;

    //now the basic virtual memory map(see memalyout.h) is established.
    //check the correctness of the basic virtual memory map.
    check_boot_pgdir();

    print_pgdir();

}

//get_pte - get pte and return the kernel virtual address of this pte for la
//        - if the PT contians this pte didn't exist, alloc a page for PT
// parameter:
//  pgdir:  the kernel virtual base address of PDT
//  la:     the linear address need to map
//  create: a logical value to decide if alloc a page for PT
// return vaule: the kernel virtual address of this pte
// pgdir是这个page table entry的虚拟地址
// la是需要被映射的虚拟地址

pte_t *
get_pte(pde_t *pgdir, uintptr_t la, bool create) {
    /* LAB2 EXERCISE 2: YOUR CODE
     *
     * If you need to visit a physical address, please use KADDR()
     * please read pmm.h for useful macros
     *
     * Maybe you want help comment, BELOW comments can help you finish the code
     *
     * Some Useful MACROs and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   PDX(la) = the index of page directory entry of VIRTUAL ADDRESS la.
     *   KADDR(pa) : takes a physical address and returns the corresponding kernel virtual address.
     *   set_page_ref(page,1) : means the page be referenced by one time
     *   page2pa(page): get the physical address of memory which this (struct Page *) page  manages
     *   struct Page * alloc_page() : allocation a page
     *   memset(void *s, char c, size_t n) : sets the first n bytes of the memory area pointed by s
     *                                       to the specified value c.
     * DEFINEs:
     *   PTE_P           0x001                   // page table/directory entry flags bit : Present
     *   PTE_W           0x002                   // page table/directory entry flags bit : Writeable
     *   PTE_U           0x004                   // page table/directory entry flags bit : User can access
     */
#if 0
    pde_t *pdep = NULL;   // (1) find page directory entry
    if (0) {              // (2) check if entry is not present
                          // (3) check if creating is needed, then alloc page for page table
                          // CAUTION: this page is used for page table, not for common data page
                          // (4) set page reference
        uintptr_t pa = 0; // (5) get linear address of page
                          // (6) clear page content using memset
                          // (7) set page directory entry's permission
    }
    return NULL;          // (8) return page table entry
#endif
	// 我们来具体分析一下这个函数究竟干了一些什么东西吧！
	// PDX是page directory index的意思，也就是这个线性地址对应的page directory index
	pde_t *pdep = &pgdir[PDX(la)]; // 首先是找到对应page directory entry
	if (!(*pdep & PTE_P)) { // PTE_P是存在，也即测试这个page directory entry对应的page table entry是否存在
		struct Page *page; // 如果不存在的话，那就需要重新分配一个页面了

		if (!create || (page = alloc_page()) == NULL) {
			return NULL;
		}
		set_page_ref(page, 1); // 创建了一个页，然后设置这个页已经被使用了
		uintptr_t pa = page2pa(page); // 得到对应的物理地址
        memset(KADDR(pa), 0, PGSIZE);
		*pdep = pa | PTE_U | PTE_W | PTE_P; // 看到，这里标记已经存在了这样一个页面啦，并且将新申请的页面的物理地址写入*pdep
	}
	// 首先，我要说一句的是，这个函数要求返回一个虚拟地址，以供放置page table entry
	// 它给了我们一个线性地址，我们知道，线性地址的高10位是page directory index
	// 中间的10位是page table index,最后的12位是页内的偏移
	// PTX是返回这个线性对应的page table index
	// 然后我还要科普一下page directory entry的结构:
	// 高20位是页表物理基地址，然后低12位是一些属性信息
	// 而PDE_ADDR(*pdep)很明显，是从*pdep这个pde中取出页表的基地址，然后PTX(la)指出这个la在页表中的偏移
	// 然后返回这个地址，很漂亮
	// KADDR(pa)返回pa这个物理地址对应的虚拟地址
	return &((pte_t *)KADDR(PDE_ADDR(*pdep)))[PTX(la)];
}

//get_page - get related Page struct for linear address la using PDT pgdir
struct Page *
get_page(pde_t *pgdir, uintptr_t la, pte_t **ptep_store) {
    pte_t *ptep = get_pte(pgdir, la, 0); // page_table_entry
    if (ptep_store != NULL) {
        *ptep_store = ptep; // 好的，写入到ptep_store指定的地址上去
    }
    if (ptep != NULL && *ptep & PTE_P) { // 如果这个page table entry已经存在
        return pte2page(*ptep); // 然后返回这个entry对应的Page的地址
    }
    return NULL;
}

//page_remove_pte - free an Page sturct which is related linear address la
//                - and clean(invalidate) pte which is related linear address la
//note: PT is changed, so the TLB need to be invalidate 
// pde_t这个结构指的是page directory entry,pgdir应该指向的是page directory table的首址
// la没有任何疑问，这是一个线性地址
// ptep指向一个普通的page table entry，也就是页表项
static inline void
page_remove_pte(pde_t *pgdir, uintptr_t la, pte_t *ptep) {
    /* LAB2 EXERCISE 3: YOUR CODE
     *
     * Please check if ptep is valid, and tlb must be manually updated if mapping is updated
     *
     * Maybe you want help comment, BELOW comments can help you finish the code
     *
     * Some Useful MACROs and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   struct Page *page pte2page(*ptep): get the according page from the value of a ptep
     *   free_page : free a page
     *   page_ref_dec(page) : decrease page->ref. NOTICE: ff page->ref == 0 , then this page should be free.
     *   tlb_invalidate(pde_t *pgdir, uintptr_t la) : Invalidate a TLB entry, but only if the page tables being
     *                        edited are the ones currently in use by the processor.
     * DEFINEs:
     *   PTE_P           0x001                   // page table/directory entry flags bit : Present
     */
#if 0
    if (0) {                      //(1) check if this page table entry is present
        struct Page *page = NULL; //(2) find corresponding page to pte
                                  //(3) decrease page reference
                                  //(4) and free this page when page reference reachs 0
                                  //(5) clear second page table entry
                                  //(6) flush tlb
    }
#endif

	if (*ptep & PTE_P) { // page table entry是存在的
		struct Page *page = pte2page(*ptep); // 得到这个*ptep所记录的页基址对应的Page的地址(每个page table entry都记录了页的基址)
		if (page_ref_dec(page) == 0) { // 减少对这个页面的引用
			free_page(page); // 释放这个页面
		}
		*ptep = 0; // 清空该表项
		tlb_invalidate(pgdir, la); // 使更改生效
	}
}

//page_remove - free an Page which is related linear address la and has an validated pte
void
page_remove(pde_t *pgdir, uintptr_t la) {
    pte_t *ptep = get_pte(pgdir, la, 0); // 注意这个0，其意思是，如果不存在，就不用创建了
    if (ptep != NULL) { // 如果这个页表项存在的话
		// 清除这个页表项
        page_remove_pte(pgdir, la, ptep);  
    }
}

//page_insert - build the map of phy addr of an Page with the linear addr la
// paramemters:
//  pgdir: the kernel virtual base address of PDT
//  page:  the Page which need to map
//  la:    the linear address need to map
//  perm:  the permission of this Page which is setted in related pte
// return value: always 0
//note: PT is changed, so the TLB need to be invalidate

// pgdir是page directory table的基地址
// page 是需要被映射的页
// la是需要被映射的线性地址
// perm权限
// page_insert这个函数主要实现了将la映射到由*page指向的物理地址，解除了原来的映射
int
page_insert(pde_t *pgdir, struct Page *page, uintptr_t la, uint32_t perm) {
    pte_t *ptep = get_pte(pgdir, la, 1); // 得到线性地址la对应的页表(page table entry)的地址，如果不存在，就创建一个
    if (ptep == NULL) {
        return -E_NO_MEM; // 到了这里，无非是内存已经耗尽了
    }
	// 这个函数主要用于将la映射到给定Page结构page所对应的页
    page_ref_inc(page); // 也就是这个页面的引用计数加1
    if (*ptep & PTE_P) { // 如果这个页面存在的话
        struct Page *p = pte2page(*ptep); // 由*ptep这个页表项记录的页的基地址，得到这个页对应的Page结构的地址
        if (p == page) { // 两个指针其实指向同一个地址,也就是说，早就映射完成了
            page_ref_dec(page); 
        }
        else { // 如果不是指向同一个地址,那么就要将原来的映射解除
            page_remove_pte(pgdir, la, ptep); // 这个函数具体是将*ptep清空了，然后使更改生效，也就是解除了映射
        }
    }
	// page2pa(page)是得到page指向的地址的首址，下面实现了映射
    *ptep = page2pa(page) | PTE_P | perm;
    tlb_invalidate(pgdir, la); // 使更改生效
    return 0;
}

// invalidate a TLB entry, but only if the page tables being
// edited are the ones currently in use by the processor.
// 使得tlb entry无效
void
tlb_invalidate(pde_t *pgdir, uintptr_t la) {
    if (rcr3() == PADDR(pgdir)) {
        invlpg((void *)la);
    }
}

// pgdir_alloc_page - call alloc_page & page_insert functions to 
//                  - allocate a page size memory & setup an addr map
//                  - pa<->la with linear address la and the PDT pgdir
// @pgdir 是page direcotry table的首地址
// @la 是线性地址 
// @perm 是权限

struct Page *
pgdir_alloc_page(pde_t *pgdir, uintptr_t la, uint32_t perm) { 
    struct Page *page = alloc_page(); // 用于分配一个页面
    if (page != NULL) {
        if (page_insert(pgdir, page, la, perm) != 0) { // 将la映射到了page对应的这个页
            free_page(page); // 这里无非就是出错了，要释放内存
            return NULL;
        }
        if (swap_init_ok){
			// 这个就很有意思啦，
            swap_map_swappable(check_mm_struct, la, page, 0); // 好吧，分配的时候将这个页面标记为可以交换
            page->pra_vaddr=la; // 记录下page对应的线性地址la吗
            assert(page_ref(page) == 1);
            //cprintf("get No. %d  page: pra_vaddr %x, pra_link.prev %x, pra_link_next %x in pgdir_alloc_page\n", (page-pages), page->pra_vaddr,page->pra_page_link.prev, page->pra_page_link.next);
        }

    }

    return page;
}

static void
check_alloc_page(void) {
    pmm_manager->check();
    cprintf("check_alloc_page() succeeded!\n");
}

static void
check_pgdir(void) {
    assert(npage <= KMEMSIZE / PGSIZE);
    assert(boot_pgdir != NULL && (uint32_t)PGOFF(boot_pgdir) == 0);
    assert(get_page(boot_pgdir, 0x0, NULL) == NULL);

    struct Page *p1, *p2;
    p1 = alloc_page();
    assert(page_insert(boot_pgdir, p1, 0x0, 0) == 0);

    pte_t *ptep;
    assert((ptep = get_pte(boot_pgdir, 0x0, 0)) != NULL);
    assert(pte2page(*ptep) == p1);
    assert(page_ref(p1) == 1);

    ptep = &((pte_t *)KADDR(PDE_ADDR(boot_pgdir[0])))[1];
    assert(get_pte(boot_pgdir, PGSIZE, 0) == ptep);

    p2 = alloc_page();
    assert(page_insert(boot_pgdir, p2, PGSIZE, PTE_U | PTE_W) == 0);
    assert((ptep = get_pte(boot_pgdir, PGSIZE, 0)) != NULL);
    assert(*ptep & PTE_U);
    assert(*ptep & PTE_W);
    assert(boot_pgdir[0] & PTE_U);
    assert(page_ref(p2) == 1);

    assert(page_insert(boot_pgdir, p1, PGSIZE, 0) == 0);
    assert(page_ref(p1) == 2);
    assert(page_ref(p2) == 0);
    assert((ptep = get_pte(boot_pgdir, PGSIZE, 0)) != NULL);
    assert(pte2page(*ptep) == p1);
    assert((*ptep & PTE_U) == 0);

    page_remove(boot_pgdir, 0x0);
    assert(page_ref(p1) == 1);
    assert(page_ref(p2) == 0);

    page_remove(boot_pgdir, PGSIZE);
    assert(page_ref(p1) == 0);
    assert(page_ref(p2) == 0);

    assert(page_ref(pde2page(boot_pgdir[0])) == 1);
    free_page(pde2page(boot_pgdir[0]));
    boot_pgdir[0] = 0;

    cprintf("check_pgdir() succeeded!\n");
}

static void
check_boot_pgdir(void) {
    pte_t *ptep;
    int i;
    for (i = 0; i < npage; i += PGSIZE) {
        assert((ptep = get_pte(boot_pgdir, (uintptr_t)KADDR(i), 0)) != NULL);
        assert(PTE_ADDR(*ptep) == i);
    }

    assert(PDE_ADDR(boot_pgdir[PDX(VPT)]) == PADDR(boot_pgdir));

    assert(boot_pgdir[0] == 0);

    struct Page *p;
    p = alloc_page(); // 分配了一个页面
    assert(page_insert(boot_pgdir, p, 0x100, PTE_W) == 0);
    assert(page_ref(p) == 1);
    assert(page_insert(boot_pgdir, p, 0x100 + PGSIZE, PTE_W) == 0);
    assert(page_ref(p) == 2);

    const char *str = "ucore: Hello world!!";
    strcpy((void *)0x100, str);
    assert(strcmp((void *)0x100, (void *)(0x100 + PGSIZE)) == 0);

    *(char *)(page2kva(p) + 0x100) = '\0';
    assert(strlen((const char *)0x100) == 0);

    free_page(p);
    free_page(pde2page(boot_pgdir[0]));
    boot_pgdir[0] = 0;

    cprintf("check_boot_pgdir() succeeded!\n");
}

//perm2str - use string 'u,r,w,-' to present the permission
static const char *
perm2str(int perm) {
    static char str[4];
    str[0] = (perm & PTE_U) ? 'u' : '-';
    str[1] = 'r';
    str[2] = (perm & PTE_W) ? 'w' : '-';
    str[3] = '\0';
    return str;
}

//get_pgtable_items - In [left, right] range of PDT or PT, find a continuous linear addr space
//                  - (left_store*X_SIZE~right_store*X_SIZE) for PDT or PT
//                  - X_SIZE=PTSIZE=4M, if PDT; X_SIZE=PGSIZE=4K, if PT
// paramemters:
//  left:        no use ???
//  right:       the high side of table's range
//  start:       the low side of table's range
//  table:       the beginning addr of table
//  left_store:  the pointer of the high side of table's next range
//  right_store: the pointer of the low side of table's next range
// return value: 0 - not a invalid item range, perm - a valid item range with perm permission 
static int
get_pgtable_items(size_t left, size_t right, size_t start, uintptr_t *table, size_t *left_store, size_t *right_store) {
    if (start >= right) {
        return 0;
    }
    while (start < right && !(table[start] & PTE_P)) {
        start ++;
    }
    if (start < right) {
        if (left_store != NULL) {
            *left_store = start;
        }
        int perm = (table[start ++] & PTE_USER);
        while (start < right && (table[start] & PTE_USER) == perm) {
            start ++;
        }
        if (right_store != NULL) {
            *right_store = start;
        }
        return perm;
    }
    return 0;
}

//print_pgdir - print the PDT&PT
void
print_pgdir(void) {
    cprintf("-------------------- BEGIN --------------------\n");
    size_t left, right = 0, perm;
    while ((perm = get_pgtable_items(0, NPDEENTRY, right, vpd, &left, &right)) != 0) {
        cprintf("PDE(%03x) %08x-%08x %08x %s\n", right - left,
                left * PTSIZE, right * PTSIZE, (right - left) * PTSIZE, perm2str(perm));
        size_t l, r = left * NPTEENTRY;
        while ((perm = get_pgtable_items(left * NPTEENTRY, right * NPTEENTRY, r, vpt, &l, &r)) != 0) {
            cprintf("  |-- PTE(%05x) %08x-%08x %08x %s\n", r - l,
                    l * PGSIZE, r * PGSIZE, (r - l) * PGSIZE, perm2str(perm));
        }
    }
    cprintf("--------------------- END ---------------------\n");
}

void *
kmalloc(size_t n) {
    void * ptr=NULL;
    struct Page *base=NULL;
    assert(n > 0 && n < 1024*0124);
    int num_pages=(n+PGSIZE-1)/PGSIZE;
    base = alloc_pages(num_pages);
    assert(base != NULL);
    ptr=page2kva(base);
    return ptr;
}

void 
kfree(void *ptr, size_t n) {
    assert(n > 0 && n < 1024*0124);
    assert(ptr != NULL);
    struct Page *base=NULL;
    int num_pages=(n+PGSIZE-1)/PGSIZE;
    base = kva2page(ptr);
    free_pages(base, num_pages);
}
