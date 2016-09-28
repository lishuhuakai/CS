#include <defs.h>
#include <kmalloc.h>
#include <sem.h>
#include <vfs.h>
#include <dev.h>
#include <file.h>
#include <sfs.h>
#include <inode.h>
#include <assert.h>
//called when init_main proc start
// file system
// 用于初始化文件系统
void
fs_init(void) {
    vfs_init();
    dev_init();
    sfs_init();
}

void
fs_cleanup(void) {
    vfs_cleanup();
}

void
lock_files(struct files_struct *filesp) {
    down(&(filesp->files_sem)); // 所谓的开锁，解锁倒是非常简单
}

void
unlock_files(struct files_struct *filesp) {
    up(&(filesp->files_sem));
}
//Called when a new proc init
// 当一个新的进程初始化的时候会调用这个过程
struct files_struct *
files_create(void) {
    //cprintf("[files_create]\n");
    static_assert((int)FILES_STRUCT_NENTRY > 128);
    struct files_struct *filesp;
    if ((filesp = kmalloc(sizeof(struct files_struct) + FILES_STRUCT_BUFSIZE)) != NULL) { // 恰好是分配一页的数据
        filesp->pwd = NULL; // 当前的工作目录
        filesp->fd_array = (void *)(filesp + 1); //第一个是表头文件files_struct，其余才是file
        filesp->files_count = 0;
        sem_init(&(filesp->files_sem), 1); // 只有一个资源
        fd_array_init(filesp->fd_array);
    }
    return filesp;
}
//Called when a proc exit
void
files_destroy(struct files_struct *filesp) { // 用于销毁文件
//    cprintf("[files_destroy]\n");
    assert(filesp != NULL && files_count(filesp) == 0);
    if (filesp->pwd != NULL) { // 如果当前目录不为空
        vop_ref_dec(filesp->pwd);
    }
    int i;
    struct file *file = filesp->fd_array;
    for (i = 0; i < FILES_STRUCT_NENTRY; i ++, file ++) {
        if (file->status == FD_OPENED) {
            fd_array_close(file); // 关闭每一个文件
        }
        assert(file->status == FD_NONE);
    }
    kfree(filesp);
}

void
files_closeall(struct files_struct *filesp) {
//    cprintf("[files_closeall]\n");
    assert(filesp != NULL && files_count(filesp) > 0);
    int i;
    struct file *file = filesp->fd_array;
    //skip the stdin & stdout
    for (i = 2, file += 2; i < FILES_STRUCT_NENTRY; i ++, file ++) {
        if (file->status == FD_OPENED) {
            fd_array_close(file);
        }
    }
}

// 我们来看一下复制files_struct的函数dup_fs
int
dup_fs(struct files_struct *to, struct files_struct *from) {
//    cprintf("[dup_fs]\n");
    assert(to != NULL && from != NULL);
    assert(files_count(to) == 0 && files_count(from) > 0);
    if ((to->pwd = from->pwd) != NULL) {
        vop_ref_inc(to->pwd); // 增加引用就可以了
    }
    int i;
    struct file *to_file = to->fd_array, *from_file = from->fd_array;
    for (i = 0; i < FILES_STRUCT_NENTRY; i ++, to_file ++, from_file ++) {
        if (from_file->status == FD_OPENED) {
            /* alloc_fd first */
            to_file->status = FD_INIT;
            fd_array_dup(to_file, from_file);
        }
    }
    return 0;
}

