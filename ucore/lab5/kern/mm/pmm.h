#ifndef __KERN_MM_PMM_H__
#define __KERN_MM_PMM_H__

#include <defs.h>
#include <mmu.h>
#include <memlayout.h>
#include <atomic.h>
#include <assert.h>

// pmm_manager is a physical memory management class. A special pmm manager - XXX_pmm_manager
// only needs to implement the methods in pmm_manager class, then XXX_pmm_manager can be used
// by ucore to manage the total physical memory space.
// 这里是在用c来模拟类吗？
struct pmm_manager {
    const char *name;                                 // XXX_pmm_manager's name
    void (*init)(void);                               // initialize internal description&management data structure
                                                      // (free block list, number of free block) of XXX_pmm_manager 
    void (*init_memmap)(struct Page *base, size_t n); // setup description&management data structcure according to
                                                      // the initial free physical memory space 
	// 下面的函数是用来分配页面吗？
    struct Page *(*alloc_pages)(size_t n);            // allocate >=n pages, depend on the allocation algorithm 
	// 下面的函数用于释放页面
    void (*free_pages)(struct Page *base, size_t n);  // free >=n pages with "base" addr of Page descriptor structures(memlayout.h)
	// 确实很像一个类
    size_t (*nr_free_pages)(void);                    // return the number of free pages 
    void (*check)(void);                              // check the correctness of XXX_pmm_manager 
};

extern const struct pmm_manager *pmm_manager;
extern pde_t *boot_pgdir;
extern uintptr_t boot_cr3;

void pmm_init(void);

struct Page *alloc_pages(size_t n);
void free_pages(struct Page *base, size_t n);
size_t nr_free_pages(void);

#define alloc_page() alloc_pages(1)
#define free_page(page) free_pages(page, 1)

pte_t *get_pte(pde_t *pgdir, uintptr_t la, bool create);
struct Page *get_page(pde_t *pgdir, uintptr_t la, pte_t **ptep_store);
void page_remove(pde_t *pgdir, uintptr_t la);
int page_insert(pde_t *pgdir, struct Page *page, uintptr_t la, uint32_t perm);

void load_esp0(uintptr_t esp0);
void tlb_invalidate(pde_t *pgdir, uintptr_t la);
struct Page *pgdir_alloc_page(pde_t *pgdir, uintptr_t la, uint32_t perm);
void unmap_range(pde_t *pgdir, uintptr_t start, uintptr_t end);
void exit_range(pde_t *pgdir, uintptr_t start, uintptr_t end);
int copy_range(pde_t *to, pde_t *from, uintptr_t start, uintptr_t end, bool share);

void print_pgdir(void);

/* *
 * PADDR - takes a kernel virtual address (an address that points above KERNBASE),
 * where the machine's maximum 256MB of physical memory is mapped and returns the
 * corresponding physical address.  It panics if you pass it a non-kernel virtual address.
 * */
// 输入的kva是一个虚拟地址
// 然后这个宏的作用是返回对应的物理地址
#define PADDR(kva) ({                                                   \
            uintptr_t __m_kva = (uintptr_t)(kva);                       \
            if (__m_kva < KERNBASE) {                                   \
                panic("PADDR called with invalid kva %08lx", __m_kva);  \
            }                                                           \
            __m_kva - KERNBASE;                                         \
        })

/* *
 * KADDR - takes a physical address and returns the corresponding kernel virtual
 * address. It panics if you pass an invalid physical address.
 * */
// pa是一个物理地址
// PPN是取得高22位的值，也就是所谓的页的索引
// 返回了这个物理地址对应的虚拟地址
#define KADDR(pa) ({                                                    \
            uintptr_t __m_pa = (pa);                                    \
            size_t __m_ppn = PPN(__m_pa);                               \
            if (__m_ppn >= npage) {                                     \
                panic("KADDR called with invalid pa %08lx", __m_pa);    \
            }                                                           \
            (void *) (__m_pa + KERNBASE);                               \
        })

extern struct Page *pages;
extern size_t npage; // 页的数目，是吧。

// 好吧，我们来看一下page2ppn，page是一个 虚拟地址，pages也是一个虚拟地址，
// 好吧，这个函数实际上是找出page在pages中的索引
static inline ppn_t
page2ppn(struct Page *page) {
    return page - pages;
}

// page2pa吧，也就是完成page到physical address的转换
// page2ppn(page)找到page在pages中的索引号，然后左移12位，得到对应的物理地址
// 至少在ucore这个系统内，这种映射关系是行得通的，将Page结构在pages数组中的索引左移12位，可以得到对应的物理地址
static inline uintptr_t
page2pa(struct Page *page) {
    return page2ppn(page) << PGSHIFT; // PGSHIFT = 12
}

// 这个函数主要用于根据物理地址pa得到其对应的Page结构体的地址
static inline struct Page *
pa2page(uintptr_t pa) {
	// pa是一个物理地址,然后PPN(pa)返回这个地址在页表中的索引
    if (PPN(pa) >= npage) {
        panic("pa2page called with invalid pa");
    }
	// pages其实是页表的首址，页表是一大块连续的区域
    return &pages[PPN(pa)];
}

// 函数的具体功能是实现从page转换到kernel visual address
static inline void *
page2kva(struct Page *page) {
    return KADDR(page2pa(page)); // page2pa(page)返回page指定的实际物理页的首地址（也是这个物理页物理地址）
}

static inline struct Page *
kva2page(void *kva) {
    return pa2page(PADDR(kva));
}

// pte是page table entry,也就是页表的表项
static inline struct Page *
pte2page(pte_t pte) { 
    if (!(pte & PTE_P)) { // 指的是这个表项不存在
        panic("pte2page called with invalid pte");
    }
	// PTE_ADDR返回pte指向的页表项中记录的页的基地址,是物理地址
	// pa2page简称是physical address to page
	// 也就是返回pte记录的页对应的Page的地址
    return pa2page(PTE_ADDR(pte));
}

static inline struct Page *
pde2page(pde_t pde) {
	// PDE_ADDR用于获取二级页表的基址，这是一个物理地址
	// pa2page(PDE_ADDR(pde))用于得到对应物理地址对应的Page
    return pa2page(PDE_ADDR(pde));
}

static inline int
page_ref(struct Page *page) {
    return page->ref;
}

static inline void
set_page_ref(struct Page *page, int val) {
    page->ref = val;
}

static inline int
page_ref_inc(struct Page *page) {
    page->ref += 1;
    return page->ref;
}

static inline int
page_ref_dec(struct Page *page) {
    page->ref -= 1;
    return page->ref;
}

extern char bootstack[], bootstacktop[];

#endif /* !__KERN_MM_PMM_H__ */

