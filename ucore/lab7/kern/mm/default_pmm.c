#include <pmm.h>
#include <list.h>
#include <string.h>
#include <default_pmm.h>

/* In the first fit algorithm, the allocator keeps a list of free blocks (known as the free list) and,
   on receiving a request for memory, scans along the list for the first block that is large enough to
   satisfy the request. If the chosen block is significantly larger than that requested, then it is 
   usually split, and the remainder added to the list as another free block.
   Please see Page 196~198, Section 8.2 of Yan Wei Min's chinese book "Data Structure -- C programming language"
*/
// LAB2 EXERCISE 1: YOUR CODE
// you should rewrite functions: default_init,default_init_memmap,default_alloc_pages, default_free_pages.
/*
 * Details of FFMA
 * (1) Prepare: In order to implement the First-Fit Mem Alloc (FFMA), we should manage the free mem block use some list.
 *              The struct free_area_t is used for the management of free mem blocks. At first you should
 *              be familiar to the struct list in list.h. struct list is a simple doubly linked list implementation.
 *              You should know howto USE: list_init, list_add(list_add_after), list_add_before, list_del, list_next, list_prev
 *              Another tricky method is to transform a general list struct to a special struct (such as struct page):
 *              you can find some MACRO: le2page (in memlayout.h), (in future labs: le2vma (in vmm.h), le2proc (in proc.h),etc.)
 * (2) default_init: you can reuse the  demo default_init fun to init the free_list and set nr_free to 0.
 *              free_list is used to record the free mem blocks. nr_free is the total number for free mem blocks.
 * (3) default_init_memmap:  CALL GRAPH: kern_init --> pmm_init-->page_init-->init_memmap--> pmm_manager->init_memmap
 *              This fun is used to init a free block (with parameter: addr_base, page_number).
 *              First you should init each page (in memlayout.h) in this free block, include:
 *                  p->flags should be set bit PG_property (means this page is valid. In pmm_init fun (in pmm.c),
 *                  the bit PG_reserved is setted in p->flags)
 *                  if this page  is free and is not the first page of free block, p->property should be set to 0.
 *                  if this page  is free and is the first page of free block, p->property should be set to total num of block.
 *                  p->ref should be 0, because now p is free and no reference.
 *                  We can use p->page_link to link this page to free_list, (such as: list_add_before(&free_list, &(p->page_link)); )
 *              Finally, we should sum the number of free mem block: nr_free+=n
 * (4) default_alloc_pages: search find a first free block (block size >=n) in free list and reszie the free block, return the addr
 *              of malloced block.
 *              (4.1) So you should search freelist like this:
 *                       list_entry_t le = &free_list;
 *                       while((le=list_next(le)) != &free_list) {
 *                       ....
 *                 (4.1.1) In while loop, get the struct page and check the p->property (record the num of free block) >=n?
 *                       struct Page *p = le2page(le, page_link);
 *                       if(p->property >= n){ ...
 *                 (4.1.2) If we find this p, then it' means we find a free block(block size >=n), and the first n pages can be malloced.
 *                     Some flag bits of this page should be setted: PG_reserved =1, PG_property =0
 *                     unlink the pages from free_list
 *                     (4.1.2.1) If (p->property >n), we should re-caluclate number of the the rest of this free block,
 *                           (such as: le2page(le,page_link))->property = p->property - n;)
 *                 (4.1.3)  re-caluclate nr_free (number of the the rest of all free block)
 *                 (4.1.4)  return p
 *               (4.2) If we can not find a free block (block size >=n), then return NULL
 * (5) default_free_pages: relink the pages into  free list, maybe merge small free blocks into big free blocks.
 *               (5.1) according the base addr of withdrawed blocks, search free list, find the correct position
 *                     (from low to high addr), and insert the pages. (may use list_next, le2page, list_add_before)
 *               (5.2) reset the fields of pages, such as p->ref, p->flags (PageProperty)
 *               (5.3) try to merge low addr or high addr blocks. Notice: should change some pages's p->property correctly.
 */
free_area_t free_area; // 这是一个全局的变量

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)

static void
default_init(void) {
    list_init(&free_list);
    nr_free = 0; // 空闲的页的数目为0，是吧！
}

static void
default_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
	// n是页数
    struct Page *p = base;
    for (; p != base + n; p ++) {
        assert(PageReserved(p));
        p->flags = p->property = 0;
		SetPageProperty(p);
        set_page_ref(p, 0); // 引用计数设置为0
		// 也就是在free_list的前面插入，不过考虑到这是一个双向链表，所以实际的意思是，插到链表的尾部
		list_add_before(&free_list, &(p->page_link));
    }
	// 因为base是头表
	// property这个玩意只有在free block首地址对应的Page有用，其余都被设置为了0
	// 用于指示这个free block一共有多少个free page
    base->property = n; // 可用的页面数是n
    nr_free += n; // 空闲的页面数目增加了n
}

static struct Page *
default_alloc_pages(size_t n) {
	// 我们来仔细分析一下究竟是如何来实现页面的分配的吧！n是请求的页面数
    assert(n > 0);
    if (n > nr_free) { // nr_free是空闲页面的总数
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
	list_entry_t *len;
    while ((le = list_next(le)) != &free_list) { // 从现在开始遍历
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) { // 如果空闲块的个数大于请求的个数n,这其实就是首次适应算法，是吧！
            //page = p;
            //break;
			int i;
			for (i = 0; i < n; ++i) {
				len = list_next(le); // list entry next
				struct Page *pp = le2page(le, page_link);
				SetPageReserved(pp); // setPageReserved主要是设置这个页面已经被占用了
				ClearPageProperty(pp); // 表示这个页面已经不是头表了
				list_del(le); 
				le = len;
			} // 一共分配n块嘛
			if (p->property > n) {
				(le2page(le, page_link))->property = p->property - n;
			}
			ClearPageProperty(p);
			SetPageReserved(p);
			nr_free -= n;
			return p;
        }
    }
	return NULL;
}

static void
default_free_pages(struct Page *base, size_t n) {
	// 用于释放页
	// 我们来仔细看一下释放page的函数吧！
	assert(n > 0);
	assert(PageReserved(base));

	list_entry_t *le = &free_list; // le指向空闲段链表的头部
	struct Page *p;
	while ((le = list_next(le)) != &free_list) { // 开始遍历
		p = le2page(le, page_link);
		if (p > base) { // free_list里面的数据都是按照地址从小到大排列的吧！
			break;
		}
	}
	// le现在指向一个恰好在base页面之后的page
	for (p = base; p < base + n; p++) { // 不断地在前面插入
		list_add_before(le, &(p->page_link));
	}
	base->flags = 0;
	set_page_ref(base, 0); // 引用计数变为了0
	ClearPageProperty(base); // 
	SetPageProperty(base); // 只需要这句就行了吧！
	base->property = n;

	p = le2page(le, page_link);
	if (base + n == p) { // 也就是说，前后可以连接起来
		base->property += p->property;
		p->property = 0;
	}

	le = list_prev(&(base->page_link));
	p = le2page(le, page_link); // 找到free_list中base之前的那个free page
	if (le != &free_list && p == base - 1) { // 如果两个也可以连起来
		while (le != &free_list) {
			if (p->property) {
				p->property += base->property;
				base->property = 0;
				break;
			}
			le = list_prev(le);
			p = le2page(le, page_link);
		}
	}

	nr_free += n; // 空闲页的计数加n
	return;
}

static size_t
default_nr_free_pages(void) {
	// 空闲的页面数目
    return nr_free;
}

static void
basic_check(void) {
	// 这个最基本的测试应该是能够通过的
    struct Page *p0, *p1, *p2;
    p0 = p1 = p2 = NULL;
    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(p0 != p1 && p0 != p2 && p1 != p2); // 要保证每个页面的地址都不一样，是吧！
	// page->ref究竟是一个什么玩意？
    assert(page_ref(p0) == 0 && page_ref(p1) == 0 && page_ref(p2) == 0);

    assert(page2pa(p0) < npage * PGSIZE);
    assert(page2pa(p1) < npage * PGSIZE);
    assert(page2pa(p2) < npage * PGSIZE);

    list_entry_t free_list_store = free_list; // 首先用free_list_store保存下free_list的值
    list_init(&free_list);
    assert(list_empty(&free_list));

    unsigned int nr_free_store = nr_free;
    nr_free = 0;

    assert(alloc_page() == NULL);

    free_page(p0);
    free_page(p1);
    free_page(p2);
    assert(nr_free == 3);

    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(alloc_page() == NULL);

    free_page(p0);
    assert(!list_empty(&free_list));

    struct Page *p;
    assert((p = alloc_page()) == p0);
    assert(alloc_page() == NULL);

    assert(nr_free == 0);
    free_list = free_list_store;
    nr_free = nr_free_store;

    free_page(p);
    free_page(p1);
    free_page(p2);
}

// LAB2: below code is used to check the first fit allocation algorithm (your EXERCISE 1) 
// NOTICE: You SHOULD NOT CHANGE basic_check, default_check functions!
static void
default_check(void) {
	// 好吧，我们现在来看一下默认的测试吧！
    int count = 0, total = 0;
    list_entry_t *le = &free_list; // free_list应该是一个文件头吧！
    while ((le = list_next(le)) != &free_list) { // 开始遍历
        struct Page *p = le2page(le, page_link);
        assert(PageProperty(p));
        count ++, total += p->property; // counter表示一个有多少个page，total表示一共有多少个空闲页
    }
    assert(total == nr_free_pages());

    basic_check();

    struct Page *p0 = alloc_pages(5), *p1, *p2; // 分配5个页面给p0
    assert(p0 != NULL);
    assert(!PageProperty(p0));

    list_entry_t free_list_store = free_list;
    list_init(&free_list);
    assert(list_empty(&free_list));
    assert(alloc_page() == NULL); // 然后分配一个页面应该为空，确实应该为空

    unsigned int nr_free_store = nr_free;  // 空闲的页面的数目
    nr_free = 0; // 然后将他们全部变为0

    free_pages(p0 + 2, 3); // 这个玩意能free吗？
    assert(alloc_pages(4) == NULL); // 因为nr_free=3
    assert(PageProperty(p0 + 2) && p0[2].property == 3);
    assert((p1 = alloc_pages(3)) != NULL); // 给p1分配3块,现在nr_free=0
    assert(alloc_page() == NULL);
    assert(p0 + 2 == p1); 

    p2 = p0 + 1;
    free_page(p0); // 好吧，释放一个页面, nr_free=1
    free_pages(p1, 3); // 现在nr_free = 4
    assert(PageProperty(p0) && p0->property == 1);
    assert(PageProperty(p1) && p1->property == 3);
	/**   p0    p2    p1				end
	 *     |     |     |     |     |     |       
	 *     -------------------------------
 	 *     |  N           N     N     N  |
	 *	   |                             |
	 *     -------------------------------
	 */
    assert((p0 = alloc_page()) == p2 - 1); // 这里出错了。我倒是要看一看啦！这里应该不会出错啊，参照上面的图，重新分配的话，会分配p0，恰好是p2-1
    free_page(p0);
    assert((p0 = alloc_pages(2)) == p2 + 1); // 这里是测试first fit的
	/** start  p2    p0     		   end
	*     |     |     |     |     |     |
	*     -------------------------------
	*     |  N                       N  |
	*	  |                             |
	*     -------------------------------
	*/

    free_pages(p0, 2);
    free_page(p2);
	/** start  p2    p0     		   end
	*     |     |     |     |     |     |
	*     -------------------------------
	*     |  N     N     N     N     N  |
	*	  |                             |
	*     -------------------------------
	*/

    assert((p0 = alloc_pages(5)) != NULL);
    assert(alloc_page() == NULL);

    assert(nr_free == 0);
    nr_free = nr_free_store; // 恢复原来的样子

    free_list = free_list_store;
    free_pages(p0, 5);

    le = &free_list;
    while ((le = list_next(le)) != &free_list) { // 又要开始遍历了,这里主要是测试经过这么折腾之后，没有出现页丢失的情况
        struct Page *p = le2page(le, page_link);
        count --, total -= p->property;
    }
    assert(count == 0);
    assert(total == 0);
}

const struct pmm_manager default_pmm_manager = {
    .name = "default_pmm_manager",
    .init = default_init, // 我倒是要看一下默认的初始化函数
    .init_memmap = default_init_memmap,
    .alloc_pages = default_alloc_pages,
    .free_pages = default_free_pages,
    .nr_free_pages = default_nr_free_pages,
    .check = default_check, // 原来是调用default_check函数来实行检查
};

