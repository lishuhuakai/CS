#ifndef __KERN_MM_MEMLAYOUT_H__
#define __KERN_MM_MEMLAYOUT_H__

/* This file contains the definitions for memory management in our OS. */

/* global segment number */
#define SEG_KTEXT   1
#define SEG_KDATA   2
#define SEG_UTEXT   3
#define SEG_UDATA   4
#define SEG_TSS     5

/* global descrptor numbers */
#define GD_KTEXT    ((SEG_KTEXT) << 3)      // kernel text
#define GD_KDATA    ((SEG_KDATA) << 3)      // kernel data
#define GD_UTEXT    ((SEG_UTEXT) << 3)      // user text
#define GD_UDATA    ((SEG_UDATA) << 3)      // user data
#define GD_TSS      ((SEG_TSS) << 3)        // task segment selector

#define DPL_KERNEL  (0)
#define DPL_USER    (3)

#define KERNEL_CS   ((GD_KTEXT) | DPL_KERNEL)
#define KERNEL_DS   ((GD_KDATA) | DPL_KERNEL)
#define USER_CS     ((GD_UTEXT) | DPL_USER)
#define USER_DS     ((GD_UDATA) | DPL_USER)

/* *
 * Virtual memory map:                                          Permissions
 *                                                              kernel/user
 *
 *     4G ------------------> +---------------------------------+
 *                            |                                 |
 *                            |         Empty Memory (*)        |
 *                            |                                 |
 *                            +---------------------------------+ 0xFB000000
 *                            |   Cur. Page Table (Kern, RW)    | RW/-- PTSIZE
 *     VPT -----------------> +---------------------------------+ 0xFAC00000
 *                            |        Invalid Memory (*)       | --/--
 *     KERNTOP -------------> +---------------------------------+ 0xF8000000
 *                            |                                 |
 *                            |    Remapped Physical Memory     | RW/-- KMEMSIZE
 *                            |                                 |
 *     KERNBASE ------------> +---------------------------------+ 0xC0000000
 *                            |                                 |
 *                            |                                 |
 *                            |                                 |
 *                            ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * (*) Note: The kernel ensures that "Invalid Memory" is *never* mapped.
 *     "Empty Memory" is normally unmapped, but user programs may map pages
 *     there if desired.
 *
 * */

/* All physical memory mapped at this address */
// 全部的物理内存都被映射到了这个地址
#define KERNBASE            0xC0000000
#define KMEMSIZE            0x38000000                  // the maximum amount of physical memory
#define KERNTOP             (KERNBASE + KMEMSIZE) // 内核程序的最高的地址，是吧！

/* *
 * Virtual page table. Entry PDX[VPT] in the PD (Page Directory) contains
 * a pointer to the page directory itself, thereby turning the PD into a page
 * table, which maps all the PTEs (Page Table Entry) containing the page mappings
 * for the entire virtual address space into that 4 Meg region starting at VPT.
 * */
 // visual page table 的地址
#define VPT                 0xFAC00000

#define KSTACKPAGE          2                           // # of pages in kernel stack
// 好吧，我猜测，这是kernel的stack的大小，居然只有8kb
#define KSTACKSIZE          (KSTACKPAGE * PGSIZE)       // sizeof kernel stack

#ifndef __ASSEMBLER__

#include <defs.h>
#include <atomic.h>
#include <list.h>

typedef uintptr_t pte_t; // page table
typedef uintptr_t pde_t; // page directory

// some constants for bios interrupt 15h AX = 0xE820
#define E820MAX             20      // number of entries in E820MAP
#define E820_ARM            1       // address range memory
#define E820_ARR            2       // address range reserved

// 下面的这个结构是干什么？
struct e820map {
    int nr_map; // 这样的结构一共有多少个
    struct {
        uint64_t addr; // 地址
        uint64_t size; // 大小
        uint32_t type; // 类型
    } __attribute__((packed)) map[E820MAX];
};

/* *
 * struct Page - Page descriptor structures. Each Page describes one
 * physical page. In kern/mm/pmm.h, you can find lots of useful functions
 * that convert Page to other data types, such as phyical address.
 * */
// 按照上面的说法，一个page结构体描述了一个物理页
// 好吧，这些玩意都是自己定义的，用于做管理用
struct Page {
    int ref;                        // page frame's reference counter # 引用计数，什么玩意？
    uint32_t flags;                 // array of flags that describe the status of the page frame
    unsigned int property;          // the num of free block, used in first fit pm manager # 空闲块的数目
    list_entry_t page_link;         // free list link
};

/* Flags describing the status of a page frame */
#define PG_reserved                 0       // if this bit=1: the Page is reserved for kernel, cannot be used in alloc/free_pages; otherwise, this bit=0 
#define PG_property                 1       // if this bit=1: the Page is the head page of a free memory block(contains some continuous_addrress pages), 
											// and can be used in alloc_pages; 
											// if this bit=0: if the Page is the the head page of a free memory block, 
											// then this Page and the memory block is alloced. Or this Page isn't the head page.

#define SetPageReserved(page)       set_bit(PG_reserved, &((page)->flags)) // 好吧，设置这个页是留给kernel的，属于reserved的页
#define ClearPageReserved(page)     clear_bit(PG_reserved, &((page)->flags)) // 也就是这个页现在可以自由分配了
#define PageReserved(page)          test_bit(PG_reserved, &((page)->flags)) // 测试是否是保留的页
#define SetPageProperty(page)       set_bit(PG_property, &((page)->flags)) // 如果为1的话，表示这是头表
#define ClearPageProperty(page)     clear_bit(PG_property, &((page)->flags))
#define PageProperty(page)          test_bit(PG_property, &((page)->flags))


/* *
* to_struct - get the struct from a ptr
* @ptr:    a struct pointer of member
* @type:   the type of the struct this is embedded in
* @member: the name of the member within the struct
* 
* #define to_struct(ptr, type, member)                               \
*     ((type *)((char *)(ptr) - offsetof(type, member)))
* */

/* Return the offset of 'member' relative to the beginning of a struct type 
 * #define offsetof(type, member)                                      \
 *     ((size_t)(&((type *)0)->member))
 * */

/* * 
 * 我们来看一个调用的实际例子,le是struct Page的一个实例的page_link成员的地址，而page_link是struct Page的一个成员
 * le2page(le, page_link);
 * 这个玩意展开后变成了这个样子: to_struct((le), struct Page, page_link);
 * 我们继续展开: ((struct Page *)((char *)(le) - offsetof((Struct Page), page_link)))
 * offset((struct Page), page_link)返回page_link这个成员在struct Page中的偏移地址(以字节为单位)，假设为off吧。
 * 所以，我们将le往前挪动off个单位，那么le就指向这个struct Page的实例了。
 * */

// convert list entry to page
#define le2page(le, member)                 \
    to_struct((le), struct Page, member)

/* free_area_t - maintains a doubly linked list to record free (unused) pages */
// 用一个双向链表来表示page
typedef struct {
    list_entry_t free_list;         // the list header # 链表的头部
	// 下面用来表示可以分配的页的数目，是吧！
    unsigned int nr_free;           // # of free pages in this free list
} free_area_t;

#endif /* !__ASSEMBLER__ */

#endif /* !__KERN_MM_MEMLAYOUT_H__ */

