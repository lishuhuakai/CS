#ifndef __KERN_FS_FS_H__
#define __KERN_FS_FS_H__

#include <defs.h>
#include <mmu.h>
#include <sem.h>
#include <atomic.h>

#define SECTSIZE            512
#define PAGE_NSECT          (PGSIZE / SECTSIZE)

#define SWAP_DEV_NO         1
#define DISK0_DEV_NO        2
#define DISK1_DEV_NO        3

void fs_init(void);
void fs_cleanup(void);

struct inode;
struct file;

/*
 * process's file related informaction
 */
// 这些玩意层层叠叠的，非常复杂
struct files_struct {
    struct inode *pwd;      // inode of present working directory # 当前的工作目录
    struct file *fd_array;  // opened files array # 文件的列表
    int files_count;        // the number of opened files # 打开的文件夹的数目
    semaphore_t files_sem;  // lock protect sem
};

#define FILES_STRUCT_BUFSIZE                       (PGSIZE - sizeof(struct files_struct))
#define FILES_STRUCT_NENTRY                        (FILES_STRUCT_BUFSIZE / sizeof(struct file))

void lock_files(struct files_struct *filesp);
void unlock_files(struct files_struct *filesp);

struct files_struct *files_create(void);
void files_destroy(struct files_struct *filesp);
void files_closeall(struct files_struct *filesp);
int dup_files(struct files_struct *to, struct files_struct *from);

static inline int
files_count(struct files_struct *filesp) {
    return filesp->files_count;
}

static inline int
files_count_inc(struct files_struct *filesp) {
    filesp->files_count += 1;
    return filesp->files_count;
}

static inline int
files_count_dec(struct files_struct *filesp) {
    filesp->files_count -= 1;
    return filesp->files_count;
}

#endif /* !__KERN_FS_FS_H__ */

