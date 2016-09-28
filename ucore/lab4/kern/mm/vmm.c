#include <vmm.h>
#include <sync.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <error.h>
#include <pmm.h>
#include <x86.h>
#include <swap.h>
#include <kmalloc.h>

/*
  vmm design include two parts: mm_struct (mm) & vma_struct (vma)
  mm is the memory manager for the set of continuous virtual memory
  area which have the same PDT. vma is a continuous virtual memory area.
  There a linear link list for vma & a redblack link list for vma in mm.
---------------
  mm related functions:
   golbal functions
	 struct mm_struct * mm_create(void)
	 void mm_destroy(struct mm_struct *mm)
	 int do_pgfault(struct mm_struct *mm, uint32_t error_code, uintptr_t addr)
--------------
  vma related functions:
   global functions
	 struct vma_struct * vma_create (uintptr_t vm_start, uintptr_t vm_end,...)
	 void insert_vma_struct(struct mm_struct *mm, struct vma_struct *vma)
	 struct vma_struct * find_vma(struct mm_struct *mm, uintptr_t addr)
   local functions
	 inline void check_vma_overlap(struct vma_struct *prev, struct vma_struct *next)
---------------
   check correctness functions
	 void check_vmm(void);
	 void check_vma_struct(void);
	 void check_pgfault(void);
*/

static void check_vmm(void);
static void check_vma_struct(void);
static void check_pgfault(void);

// mm_create -  alloc a mm_struct & initialize it.
// 用于分配一个mm_struct结构，并且初始化
struct mm_struct *
mm_create(void) {
	struct mm_struct *mm = kmalloc(sizeof(struct mm_struct)); // 好吧，先分配

	if (mm != NULL) {
		list_init(&(mm->mmap_list)); // 大概是让头尾指针指向自己
		mm->mmap_cache = NULL;
		mm->pgdir = NULL;
		mm->map_count = 0;

		if (swap_init_ok) swap_init_mm(mm);
		else mm->sm_priv = NULL;
	}
	return mm;
}

// vma_create - alloc a vma_struct & initialize it. (addr range: vm_start~vm_end)
struct vma_struct *
vma_create(uintptr_t vm_start, uintptr_t vm_end, uint32_t vm_flags) {
	struct vma_struct *vma = kmalloc(sizeof(struct vma_struct)); // 好吧，类似于指针一类的东西，首先申请

	if (vma != NULL) {
		vma->vm_start = vm_start; // 开始的地方
		vma->vm_end = vm_end; // 结束的地方
		vma->vm_flags = vm_flags; // 区域的一些标志
	}
	return vma;
}


// find_vma - find a vma  (vma->vm_start <= addr <= vma_vm_end)
struct vma_struct *
find_vma(struct mm_struct *mm, uintptr_t addr) {
	// 我已经说过了，mm其实是个头部节点
	struct vma_struct *vma = NULL;
	if (mm != NULL) {
		vma = mm->mmap_cache; // mm 指的是 memory manage吗？根据某某定理，cache的命中率非常高
		if (!(vma != NULL && vma->vm_start <= addr && vma->vm_end > addr)) {
			bool found = 0; // 表示没有找到
			list_entry_t *list = &(mm->mmap_list), *le = list; // 然后再到list中寻找
			while ((le = list_next(le)) != list) { // 开始遍历了
				vma = le2vma(le, list_link); // 我很好奇，到底是怎么实现的？
				if (vma->vm_start <= addr && addr < vma->vm_end) {
					found = 1; // 表示找到了
					break;
				}
			}
			if (!found) { // 如果仍然没有找到，就返回null
				vma = NULL;
			}
		}
		if (vma != NULL) { // 要记得及时更新cache
			mm->mmap_cache = vma;
		}
	}
	return vma;
}


// check_vma_overlap - check if vma1 overlaps vma2 ?
// 这个具体是干什么的？ 大概是检查重叠的情况吧！
static inline void
check_vma_overlap(struct vma_struct *prev, struct vma_struct *next) {
	assert(prev->vm_start < prev->vm_end);
	assert(prev->vm_end <= next->vm_start);
	assert(next->vm_start < next->vm_end);
}


// insert_vma_struct -insert vma in mm's list link
// 插入新的vma_struct
// mm是链表的头部
void
insert_vma_struct(struct mm_struct *mm, struct vma_struct *vma) {
	assert(vma->vm_start < vma->vm_end);
	list_entry_t *list = &(mm->mmap_list);
	list_entry_t *le_prev = list, *le_next;

	list_entry_t *le = list;
	while ((le = list_next(le)) != list) { // 开始遍历
		// 首先要说一句，mm所指向的链表本来就是按照vma的大小排序的。
		struct vma_struct *mmap_prev = le2vma(le, list_link);
		if (mmap_prev->vm_start > vma->vm_start) {
			break;
		}
		le_prev = le;
	}

	le_next = list_next(le_prev); // le_next所在的vma_struct记为k，则k->vm_start > vma->vm_start

	/* check overlap */
	// 也就是前后都不能有重叠
	if (le_prev != list) {
		check_vma_overlap(le2vma(le_prev, list_link), vma);
	}
	if (le_next != list) {
		check_vma_overlap(vma, le2vma(le_next, list_link));
	}

	vma->vm_mm = mm;
	list_add_after(le_prev, &(vma->list_link));

	mm->map_count++; // map_count原来是用于统计节点的个数的
}

// mm_destroy - free mm and mm internal fields
// 好猛，用于毁掉整个链表
void
mm_destroy(struct mm_struct *mm) {

    list_entry_t *list = &(mm->mmap_list), *le;
    while ((le = list_next(list)) != list) {
        list_del(le);
        kfree(le2vma(le, list_link));  //kfree vma        
    }
    kfree(mm); //kfree mm
    mm=NULL;
}

// vmm_init - initialize virtual memory management
//          - now just call check_vmm to check correctness of vmm
// 构建链表中的一个节点
void
vmm_init(void) {
	check_vmm();
}

// check_vmm - check correctness of vmm
static void
check_vmm(void) {
	size_t nr_free_pages_store = nr_free_pages(); // 空闲的页的数目

	check_vma_struct();
	check_pgfault();

    cprintf("check_vmm() succeeded.\n");
}

// 下面的这个函数究竟是要干嘛？
static void
check_vma_struct(void) {
	size_t nr_free_pages_store = nr_free_pages(); // 空闲页的数目

	struct mm_struct *mm = mm_create(); // 构建一个头部，是吧！
	assert(mm != NULL);

    int step1 = 10, step2 = step1 * 10;

	int i;
	for (i = step1; i >= 1; i--) {
		struct vma_struct *vma = vma_create(i * 5, i * 5 + 2, 0);
		assert(vma != NULL);
		insert_vma_struct(mm, vma); // 不断地插入
	}

	for (i = step1 + 1; i <= step2; i++) {
		struct vma_struct *vma = vma_create(i * 5, i * 5 + 2, 0);
		assert(vma != NULL);
		insert_vma_struct(mm, vma);
	}

	list_entry_t *le = list_next(&(mm->mmap_list));

	for (i = 1; i <= step2; i++) { // 开始遍历了
		assert(le != &(mm->mmap_list));
		struct vma_struct *mmap = le2vma(le, list_link);
		assert(mmap->vm_start == i * 5 && mmap->vm_end == i * 5 + 2); // 这个主要是用于检查插入的时候的是否按照vma的地址进行排序的
		le = list_next(le);
	}

	for (i = 5; i <= 5 * step2; i += 5) {
		struct vma_struct *vma1 = find_vma(mm, i); // find_vma主要是根据地址i来寻找i所属的那个vma，并且返回那个vma，否则返回0
		assert(vma1 != NULL);
		struct vma_struct *vma2 = find_vma(mm, i + 1);
		assert(vma2 != NULL);
		struct vma_struct *vma3 = find_vma(mm, i + 2);
		assert(vma3 == NULL);
		struct vma_struct *vma4 = find_vma(mm, i + 3);
		assert(vma4 == NULL);
		struct vma_struct *vma5 = find_vma(mm, i + 4);
		assert(vma5 == NULL);

		assert(vma1->vm_start == i  && vma1->vm_end == i + 2);
		assert(vma2->vm_start == i  && vma2->vm_end == i + 2);
	}

	for (i = 4; i >= 0; i--) {
		struct vma_struct *vma_below_5 = find_vma(mm, i);
		if (vma_below_5 != NULL) {
			cprintf("vma_below_5: i %x, start %x, end %x\n", i, vma_below_5->vm_start, vma_below_5->vm_end);
		}
		assert(vma_below_5 == NULL);
	}

	mm_destroy(mm);

    cprintf("check_vma_struct() succeeded!\n");
}

struct mm_struct *check_mm_struct;

// check_pgfault - check correctness of pgfault handler
// 用于检查页错误，是吧!
static void
check_pgfault(void) {
	size_t nr_free_pages_store = nr_free_pages(); // 空闲页的数目

	check_mm_struct = mm_create(); // 为什么都要构建一个头部？
	assert(check_mm_struct != NULL);

	struct mm_struct *mm = check_mm_struct; // 好吧，这个恰好是前面构建的那个头部
	pde_t *pgdir = mm->pgdir = boot_pgdir; // pgdir指向page directory table
	assert(pgdir[0] == 0);

	// 这个新构建的vma，起始的地址是0，然后终止的位置是PTSIZE
	struct vma_struct *vma = vma_create(0, PTSIZE, VM_WRITE); // PTSIZE一般指的是page table的大小，是吧！
	assert(vma != NULL);

	insert_vma_struct(mm, vma);

	uintptr_t addr = 0x100;
	assert(find_vma(mm, addr) == vma);

	int i, sum = 0;
	for (i = 0; i < 100; i++) {
		*(char *)(addr + i) = i;
		sum += i;
	}
	for (i = 0; i < 100; i++) { // 这里主要是为了检测对于地址的访问没有出错是吧，不过，也应该不会出错的，不是吗？
		sum -= *(char *)(addr + i);
	}
	assert(sum == 0);

	page_remove(pgdir, ROUNDDOWN(addr, PGSIZE)); // 移除对这个页的引用
	free_page(pde2page(pgdir[0]));
	pgdir[0] = 0;

	mm->pgdir = NULL;
	mm_destroy(mm);
	check_mm_struct = NULL;

	assert(nr_free_pages_store == nr_free_pages());

	cprintf("check_pgfault() succeeded!\n");
}
//page fault number
//页错误号
volatile unsigned int pgfault_num = 0;

/* do_pgfault - interrupt handler to process the page fault execption
 * @mm         : the control struct for a set of vma using the same PDT # mm是使用同样的page direcotry table的vma所构成的链表的头部
 * 虽然说在现在看来，same PDT看起来貌似没有什么用，不过一旦扩展到多任务的时候，用处就非常大了
 * @error_code : the error code recorded in trapframe->tf_err which is setted by x86 hardware
 * @addr       : the addr which causes a memory access exception, (the contents of the CR2 register)
 *
 * CALL GRAPH: trap--> trap_dispatch-->pgfault_handler-->do_pgfault # 程序的调用顺序图
 * The processor provides ucore's do_pgfault function with two items of information to aid in diagnosing
 * the exception and recovering from it.
 *   (1) The contents of the CR2 register. The processor loads the CR2 register with the
 *       32-bit linear address that generated the exception. The do_pgfault fun can
 *       use this address to locate the corresponding page directory and page-table
 *       entries.
 *   (2) An error code on the kernel stack. The error code for a page fault has a format different from
 *       that for other exceptions. The error code tells the exception handler three things:
 *         -- The P flag   (bit 0) indicates whether the exception was due to a not-present page (0)
 *            or to either an access rights violation or the use of a reserved bit (1).
 *         -- The W/R flag (bit 1) indicates whether the memory access that caused the exception
 *            was a read (0) or write (1).
 *         -- The U/S flag (bit 2) indicates whether the processor was executing at user mode (1)
 *            or supervisor mode (0) at the time of the exception.
 */
int
do_pgfault(struct mm_struct *mm, uint32_t error_code, uintptr_t addr) {
	// addr来自cr2寄存器，也就是说，发生页错误的时候，cr2中会存放引发错误的地址，并且addr是线性地址
	// 我们来看一下，操作系统是如何来处理页错误的吧！
	int ret = -E_INVAL;
	//try to find a vma which include addr
	struct vma_struct *vma = find_vma(mm, addr); // 首先是找到addr对应的vma结构体

	pgfault_num++; // 页错误的计数器增加了1
	//If the addr is in the range of a mm's vma?
	if (vma == NULL || vma->vm_start > addr) { // 无效的地址
		cprintf("not valid addr %x, and  can not find it in vma\n", addr);
		goto failed;
	}
	//check the error_code
	switch (error_code & 3) { // 然后是检查错误码,下面的应该都是不可处理的错误，是吧！
	default:
		/* error code flag : default is 3 ( W/R=1, P=1): write, present */
	case 2: /* error code flag : (W/R=1, P=0): write, not present */
		if (!(vma->vm_flags & VM_WRITE)) { // 显示这段虚拟区域不可写
			cprintf("do_pgfault failed: error code flag = write AND not present, but the addr's vma cannot write\n");
			goto failed;
		}
		break;
	case 1: /* error code flag : (W/R=0, P=1): read, present */
		cprintf("do_pgfault failed: error code flag = read AND present\n");
		goto failed;
	case 0: /* error code flag : (W/R=0, P=0): read, not present */
		if (!(vma->vm_flags & (VM_READ | VM_EXEC))) {
			cprintf("do_pgfault failed: error code flag = read AND not present, but the addr's vma cannot read or exec\n");
			goto failed;
		}
	}
	/* IF (write an existed addr ) OR
	 *    (write an non_existed addr && addr is writable) OR
	 *    (read  an non_existed addr && addr is readable)
	 * THEN
	 *    continue process
	 */
	uint32_t perm = PTE_U; // perm一般表示权限，是吧。
	if (vma->vm_flags & VM_WRITE) {
		perm |= PTE_W;
	}
	addr = ROUNDDOWN(addr, PGSIZE); // 又要4KB对齐了

	ret = -E_NO_MEM;

	pte_t *ptep = NULL;
	/*LAB3 EXERCISE 1: YOUR CODE
	* Maybe you want help comment, BELOW comments can help you finish the code
	*
	* Some Useful MACROs and DEFINEs, you can use them in below implementation.
	* MACROs or Functions:
	*   get_pte : get an pte and return the kernel virtual address of this pte for la
	*             if the PT contians this pte didn't exist, alloc a page for PT (notice the 3th parameter '1')
	*   pgdir_alloc_page : call alloc_page & page_insert functions to allocate a page size memory & setup
	*             an addr map pa<--->la with linear address la and the PDT pgdir
	* DEFINES:
	*   VM_WRITE  : If vma->vm_flags & VM_WRITE == 1/0, then the vma is writable/non writable
	*   PTE_W           0x002                   // page table/directory entry flags bit : Writeable
	*   PTE_U           0x004                   // page table/directory entry flags bit : User can access
	* VARIABLES:
	*   mm->pgdir : the PDT of these vma
	*
	*/
#if 0
	/*LAB3 EXERCISE 1: YOUR CODE*/
	ptep = ? ? ?              //(1) try to find a pte, if pte's PT(Page Table) isn't existed, then create a PT.
		if (*ptep == 0) {
			//(2) if the phy addr isn't exist, then alloc a page & map the phy addr with logical addr

		}
		else {
			/*LAB3 EXERCISE 2: YOUR CODE
			* Now we think this pte is a  swap entry, we should load data from disk to a page with phy addr,
			* and map the phy addr with logical addr, trigger swap manager to record the access situation of this page.
			*
			*  Some Useful MACROs and DEFINEs, you can use them in below implementation.
			*  MACROs or Functions:
			*    swap_in(mm, addr, &page) : alloc a memory page, then according to the swap entry in PTE for addr,
			*                               find the addr of disk page, read the content of disk page into this memroy page
			*    page_insert ： build the map of phy addr of an Page with the linear addr la
			*    swap_map_swappable ： set the page swappable
			*/
			if (swap_init_ok) {
				struct Page *page = NULL;
				//(1）According to the mm AND addr, try to load the content of right disk page
				//    into the memory which page managed.
				//(2) According to the mm, addr AND page, setup the map of phy addr <---> logical addr
				//(3) make the page swappable.
			}
			else {
				cprintf("no swap_init_ok but ptep is %x, failed\n", *ptep);
				goto failed;
			}
		}
#endif
		// try to find a pte, if pte's PT(Page Table) isn't existed, then create a PT.
		// (notice the 3th parameter '1')
		if ((ptep = get_pte(mm->pgdir, addr, 1)) == NULL) { // 找到addr对应的页表项
			cprintf("get_pte in do_pgfault failed\n");
			goto failed;
		}

		// 什么时候会发生缺页中断？
		if (*ptep == 0) { // if the phy addr isn't exist, then alloc a page & map the phy addr with logical addr
			// 页表项为空，说明还没有为这个地址分配物理页
			// 分配了页之后，基本上缺页中断就消除了
			// 我有一个疑问，为什么会出现还没有分配页的情况？
			// pgdir_alloc_page函数有一个特性，那就是如果交换机制开启之后，会将申请的页标记为可以换出
			if (pgdir_alloc_page(mm->pgdir, addr, perm) == NULL) { // 那就分配一个页，并且实现了映射，也就是将addr映射到分配的页上面
				// 好吧，我还要说一声，那就是pgdir_alloc_page中可能会调用swap_out函数
				cprintf("pgdir_alloc_page in do_pgfault failed\n");
				goto failed;
			}
		}
		else { // if this pte is a swap entry, then load data from disk to a page with phy addr
			// and call page_insert to map the phy addr with logic addr
			// 页表项不为空，页已经存在了，那就是这个物理页被写入到了磁盘中，应该换入
			if (swap_init_ok) { // 也就是说开启了虚拟内存的机制
				struct Page *page = NULL;
				if ((ret = swap_in(mm, addr, &page)) != 0) { // 要换入一个页
					cprintf("swap_in in do pagefault failed\n");
					goto failed;
				}
				// 换页成功了！page记录了换入的数据的页
				// 最后只需要将这个addr映射到这个page就行了
				page_insert(mm->pgdir, page, addr, perm); // 将这个页映射到这个addr地址上面
				swap_map_swappable(mm, addr, page, 1); // 根据FIFO，这个地址应该是最晚被换出的,同时要将这个也标记为可以交换
				page->pra_vaddr = addr;// 记录下这个page对应的虚拟地址
			}
			else { // 不过swap还未初始化，只能报错了。
				cprintf("no swap_init_ok but ptep is %x, failed\n", *ptep);
				goto failed;
			}

		}
		ret = 0;
	failed:
		return ret;
}

