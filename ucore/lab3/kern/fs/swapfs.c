#include <swap.h>
#include <swapfs.h>
#include <mmu.h>
#include <fs.h>
#include <ide.h>
#include <pmm.h>
#include <assert.h>

void
swapfs_init(void) { // 我确实不知道这个函数究竟是干什么的,当然，你也不用知道，大概就是初始化ide什么的
    static_assert((PGSIZE % SECTSIZE) == 0);
    if (!ide_device_valid(SWAP_DEV_NO)) {
        panic("swap fs isn't available.\n");
    }
    max_swap_offset = ide_device_size(SWAP_DEV_NO) / (PGSIZE / SECTSIZE);
}

int
swapfs_read(swap_entry_t entry, struct Page *page) { // 将页面读入，是吧！
	// PAGE_NSECT是一个页面需要写入的扇区的数目,恰好是8
	// swap_offset(entry) * PAGE_NSECT得到扇区号
	// page2kva(page)得到虚拟地址，应该是从内存中读取
	// 最后的8是应该写入的扇区的数目
    return ide_read_secs(SWAP_DEV_NO, swap_offset(entry) * PAGE_NSECT, page2kva(page), PAGE_NSECT);
}

int
swapfs_write(swap_entry_t entry, struct Page *page) { // 将这个页面交换出去，是吧
	// swap_offset(entry) * PAGE_NSECT表示我们应该从哪一个扇区开始读
	// page2kva(page)表示，我们从磁盘读取的数据应该写到哪里去
	// 最后的8表示应该读入的扇区的数目
    return ide_write_secs(SWAP_DEV_NO, swap_offset(entry) * PAGE_NSECT, page2kva(page), PAGE_NSECT);
}

