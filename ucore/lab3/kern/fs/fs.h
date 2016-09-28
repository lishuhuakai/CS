#ifndef __KERN_FS_FS_H__
#define __KERN_FS_FS_H__

#include <mmu.h>

#define SECTSIZE            512
// PGSIZE / SECSIZE求得的是一个页需要几个扇区
#define PAGE_NSECT          (PGSIZE / SECTSIZE)

#define SWAP_DEV_NO         1

#endif /* !__KERN_FS_FS_H__ */

