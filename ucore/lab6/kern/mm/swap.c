#include <swap.h>
#include <swapfs.h>
#include <swap_fifo.h>
#include <stdio.h>
#include <string.h>
#include <memlayout.h>
#include <pmm.h>
#include <mmu.h>
#include <default_pmm.h>
#include <kdebug.h>

// the valid vaddr for check is between 0~CHECK_VALID_VADDR-1
#define CHECK_VALID_VIR_PAGE_NUM 5
#define BEING_CHECK_VALID_VADDR 0X1000
#define CHECK_VALID_VADDR (CHECK_VALID_VIR_PAGE_NUM+1)*0x1000
// the max number of valid physical page for check
#define CHECK_VALID_PHY_PAGE_NUM 4
// the max access seq number
#define MAX_SEQ_NO 10

static struct swap_manager *sm;
size_t max_swap_offset;

volatile int swap_init_ok = 0;

// 下面指示用于交换的页面吗？
unsigned int swap_page[CHECK_VALID_VIR_PAGE_NUM];

// 换入和换出的seq_no
unsigned int swap_in_seq_no[MAX_SEQ_NO],swap_out_seq_no[MAX_SEQ_NO];

// 用于检测交换
static void check_swap(void);

// 这是要将页面交换出去的节奏吗？
int
swap_init(void)
{
     swapfs_init();

     if (!(1024 <= max_swap_offset && max_swap_offset < MAX_SWAP_OFFSET_LIMIT))
     {
          panic("bad max_swap_offset %08x.\n", max_swap_offset);
     }
     

     sm = &swap_manager_fifo; // 先进先出
     int r = sm->init(); // 初始化
     
     if (r == 0) // 初始化成功
     {
          swap_init_ok = 1; 
          cprintf("SWAP: manager = %s\n", sm->name);
          check_swap(); // 用于检测换页的效果
     }

     return r;
}

int
swap_init_mm(struct mm_struct *mm)
{
     return sm->init_mm(mm);
}

int
swap_tick_event(struct mm_struct *mm)
{
     return sm->tick_event(mm);
}

int
swap_map_swappable(struct mm_struct *mm, uintptr_t addr, struct Page *page, int swap_in)
{
     return sm->map_swappable(mm, addr, page, swap_in);
}

int
swap_set_unswappable(struct mm_struct *mm, uintptr_t addr)
{
     return sm->set_unswappable(mm, addr);
}

volatile unsigned int swap_out_num=0;

// 用于换出，是吧！
int
swap_out(struct mm_struct *mm, int n, int in_tick)
{
	// 应该换出n个页
     int i;
     for (i = 0; i != n; ++ i) 
     {
          uintptr_t v;
          //struct Page **ptr_page=NULL;
          struct Page *page;
          // cprintf("i %d, SWAP: call swap_out_victim\n",i);
          int r = sm->swap_out_victim(mm, &page, in_tick); // page记录了应该被换出的页面对应的Page结构的地址
          if (r != 0) {
                  cprintf("i %d, swap_out: call swap_out_victim failed\n",i);
                  break;
          }          
          //assert(!PageReserved(page));

          //cprintf("SWAP: choose victim page 0x%08x\n", page);
          // 我们要看一下究竟是如何实现页面换出的
          v = page->pra_vaddr; // pra_vaddr究竟是什么东西 ,应该是一个线程地址，不过究竟是什么的线性地址呢？其实是这个页面对应的虚拟地址
          pte_t *ptep = get_pte(mm->pgdir, v, 0); // 得到对应的页表项的地址
          assert((*ptep & PTE_P) != 0); // 这样的页表项应该是存在的

		  // 我们来看一下吧，具体swapfs_write干了一些什么事情吧！
		  // page->pra_vaddr / PGSIZE + 1恰好是这个虚拟地址的页号，左移八位的话，得到了啥？
		  // 参数类型是pte_t也就是page table entry,为什么要左移8位
		  // 好吧，其实不重要，我们只需要知道，这么做告诉了磁盘应该往哪一个扇区开始写入数据
          if (swapfs_write((page->pra_vaddr / PGSIZE + 1) << 8, page) != 0) { // 将页面写入交换区，是磁盘吧？
                    cprintf("SWAP: failed to save\n");
                    sm->map_swappable(mm, v, page, 0);
                    continue;
          }
          else { // 到这里的话，写入成功
                    cprintf("swap_out: i %d, store page in vaddr 0x%x to disk swap entry %d\n", i, v, page->pra_vaddr/PGSIZE+1);
					// 既然写入了，那么肯定要标记一下，是吧，方便cpu在访问的时候触发中断
					// 既然是往page table entry里面写入数据的话，高20位是基址，在ucore这个系统里，实现的是一一映射
					// 因此基址是(page->pra_vaddr / PGSIZE + 1) >> 8
					// 总之下面的做法将p为设置为了0，也就是标记了缺页
                    *ptep = (page->pra_vaddr / PGSIZE + 1) << 8; // (page->pra_vaddr / PGSIZE + 1)得到了虚拟地址的页号，页号左移8位，得到了
                    free_page(page); // 立马释放这个页，从此就多了可用的页
          }
          
          tlb_invalidate(mm->pgdir, v); // 使得更改生效
     }
     return i;
}

// 用于换入,这些函数应该在中断时间中被调用，因为一旦发现了缺页中断，立马就有一个换入换出的过程
int
swap_in(struct mm_struct *mm, uintptr_t addr, struct Page **ptr_result)
{
	// 我们继续来看这个函数

     struct Page *result = alloc_page(); // 重新分配一个页面
     assert(result!=NULL); 

     pte_t *ptep = get_pte(mm->pgdir, addr, 0); // addr是一个线性地址，得到对应的页表项的地址
     // cprintf("SWAP: load ptep %x swap entry %d to vaddr 0x%08x, page %x, No %d\n", ptep, (*ptep)>>8, addr, result, (result-pages));
    
     int r;
	 // 我还有一个疑问，那就是我们究竟应该如何
	 // (*ptep)是作为索引的，告诉磁盘应该到那个扇区去读数据
	 // result是作为目的地址的，也就是将数据读到result对应的页去
	 // 最后只差一步了，那就是映射，也就是将addr映射到result这个page才行
	 // 具体怎么做的，请查看vmm.c do_pgfault函数
     if ((r = swapfs_read((*ptep), result)) != 0) // 将写入到result对应的页去吧！
     {
        assert(r!=0);
     }
	 // 又没有发现很有意思的一件事情，那就是(*ptep) >> 8得到了disk swap entry, *ptep是页表项,好吧，其实我也不懂，我只知道低8位是各种符号位
     cprintf("swap_in: load disk swap entry %d with swap_page in vadr 0x%x\n", (*ptep) >> 8, addr);
     *ptr_result = result; // 应该这个result返回之后还有一个映射
     return 0;
}



static inline void
check_content_set(void)
{
     *(unsigned char *)0x1000 = 0x0a; // 然后就开始写入数据了
     assert(pgfault_num==1); // 这会引发一个缺页中断
     *(unsigned char *)0x1010 = 0x0a;
     assert(pgfault_num==1);
     *(unsigned char *)0x2000 = 0x0b; // 引发缺页中断
     assert(pgfault_num==2);
     *(unsigned char *)0x2010 = 0x0b;
     assert(pgfault_num==2);
     *(unsigned char *)0x3000 = 0x0c; // 缺页中断
     assert(pgfault_num==3);
     *(unsigned char *)0x3010 = 0x0c;
     assert(pgfault_num==3);
     *(unsigned char *)0x4000 = 0x0d; // 缺页中断
     assert(pgfault_num==4);
     *(unsigned char *)0x4010 = 0x0d;
     assert(pgfault_num==4);
}

static inline int
check_content_access(void)
{
    int ret = sm->check_swap(); // 用来测试页面的交换，是吧！
    return ret;
}

struct Page * check_rp[CHECK_VALID_PHY_PAGE_NUM];
pte_t * check_ptep[CHECK_VALID_PHY_PAGE_NUM]; // 页表项的地址
unsigned int check_swap_addr[CHECK_VALID_VIR_PAGE_NUM];

extern free_area_t free_area;

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)

// 用于检测交换
static void
check_swap(void)
{
    //backup mem env
     int ret, count = 0, total = 0, i;
     list_entry_t *le = &free_list; // free_list是空闲块的列表
     while ((le = list_next(le)) != &free_list) { // 开始遍历
        struct Page *p = le2page(le, page_link);
        assert(PageProperty(p));
        count ++, total += p->property; // total是空闲页的总数, count是块的总数
     }
     assert(total == nr_free_pages());
     cprintf("BEGIN check_swap: count %d, total %d\n",count,total);
     
     //now we set the phy pages env     
     struct mm_struct *mm = mm_create(); // 构建一个链表的头部
     assert(mm != NULL);

     extern struct mm_struct *check_mm_struct;
     assert(check_mm_struct == NULL);

     check_mm_struct = mm;

     pde_t *pgdir = mm->pgdir = boot_pgdir; // page directory ,为什么是指向boot_pgdir
     assert(pgdir[0] == 0);

	 //BEING_CHECK_VALID_VADDR = 0x1000, CHECK_VALID_VADDR = 0x6000
	 // 这个玩意究竟是干什么的，我暂时还在研究之中
     struct vma_struct *vma = vma_create(BEING_CHECK_VALID_VADDR, CHECK_VALID_VADDR, VM_WRITE | VM_READ);
     assert(vma != NULL);

     insert_vma_struct(mm, vma); // 将vma插入mm链表之中

     //setup the temp Page Table vaddr 0~4MB
     cprintf("setup Page Table for vaddr 0X1000, so alloc a page\n");
     pte_t *temp_ptep = NULL;
     temp_ptep = get_pte(mm->pgdir, BEING_CHECK_VALID_VADDR, 1); // 首先是得到0x1000这个地址对应的页表项的地址
     assert(temp_ptep!= NULL);
     cprintf("setup Page Table vaddr 0~4MB OVER!\n");
     
     for (i = 0; i < CHECK_VALID_PHY_PAGE_NUM; i++) {
          check_rp[i] = alloc_page(); // 分配一个页面,check_rp是一个Page类型的数组，我想说的是，通过alloc_page()分配的页面貌似都是不能够被换出的，因为他们压根就没有别加入到链表之中
          assert(check_rp[i] != NULL);
          assert(!PageProperty(check_rp[i]));
     }
     list_entry_t free_list_store = free_list; // 先保存起来
     list_init(&free_list); // 好吧，模拟页已经分配完了
     assert(list_empty(&free_list));
     
     //assert(alloc_page() == NULL);
     
     unsigned int nr_free_store = nr_free;
     nr_free = 0; // 空闲的页数已经变为0啦。
     for (i = 0;i < CHECK_VALID_PHY_PAGE_NUM; i++) {
        free_pages(check_rp[i],1); // 现在开始释放页面 
     }
	 // 到这里的话nr_free应该等于4
     assert(nr_free==CHECK_VALID_PHY_PAGE_NUM);
     
     cprintf("set up init env for check_swap begin!\n"); // 环境已经搭建好了
     //setup initial vir_page<->phy_page environment for page relpacement algorithm 

     
     pgfault_num=0; // 设置缺页错误的次数为0
     
     check_content_set(); 
     assert(nr_free == 0); // 空闲的四个页全部被分配了         
     for(i = 0; i < MAX_SEQ_NO ; i++)  // 这里应该是初始化工作
         swap_out_seq_no[i] = swap_in_seq_no[i]=-1;
     
     for (i= 0; i < CHECK_VALID_PHY_PAGE_NUM; i++) {
         check_ptep[i] = 0;
         check_ptep[i] = get_pte(pgdir, (i+1)*0x1000, 0); // 得到对应地址的页表的地址
         //cprintf("i %d, check_ptep addr %x, value %x\n", i, check_ptep[i], *check_ptep[i]);
         assert(check_ptep[i] != NULL);
         assert(pte2page(*check_ptep[i]) == check_rp[i]); // 应该是这样的
         assert((*check_ptep[i] & PTE_P));  
     }
     cprintf("set up init env for check_swap over!\n");
     // now access the virt pages to test  page relpacement algorithm 
	 // 好吧，在check_content_access函数中测试了FIFO算法
     ret = check_content_access();
     assert(ret==0);
     
     //restore kernel mem env
     for (i = 0; i < CHECK_VALID_PHY_PAGE_NUM; i++) {
         free_pages(check_rp[i], 1);
     } 
	 // 测试做得不错
     //free_page(pte2page(*temp_ptep));
    free_page(pde2page(pgdir[0]));
     pgdir[0] = 0;
     mm->pgdir = NULL;
     mm_destroy(mm);
     check_mm_struct = NULL;
     
     nr_free = nr_free_store;
     free_list = free_list_store;

     
     le = &free_list;
     while ((le = list_next(le)) != &free_list) {
         struct Page *p = le2page(le, page_link);
         count --, total -= p->property;
     }
     cprintf("count is %d, total is %d\n",count,total);
     //assert(count == 0);
     
     cprintf("check_swap() succeeded!\n");
}
