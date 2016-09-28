#include <defs.h>
#include <string.h>
#include <vfs.h>
#include <proc.h>
#include <file.h>
#include <unistd.h>
#include <iobuf.h>
#include <inode.h>
#include <stat.h>
#include <dirent.h>
#include <error.h>
#include <assert.h>

// fd是文件描述符吗
#define testfd(fd)                          ((fd) >= 0 && (fd) < FILES_STRUCT_NENTRY)

// get_fd_array - get current process's open files table
// 得到当前进程的打开文件的表
static struct file *
get_fd_array(void) {
    struct files_struct *filesp = current->filesp; // 当前进程的files_struct，其实是一个表头
    assert(filesp != NULL && files_count(filesp) > 0);
    return filesp->fd_array;
}

// fd_array_init - initialize the open files table
// 用于初始化open files table
void
fd_array_init(struct file *fd_array) {
    int fd;
    struct file *file = fd_array;
    for (fd = 0; fd < FILES_STRUCT_NENTRY; fd ++, file ++) {
        file->open_count = 0;
        file->status = FD_NONE, file->fd = fd;
    }
}

// fs_array_alloc - allocate a free file item (with FD_NONE status) in open files table
// 分配一个空的file item
static int
fd_array_alloc(int fd, struct file **file_store) {
//    panic("debug");
	cprintf("\n==>fd_array_alloc\n");
    struct file *file = get_fd_array(); // 得到当前进程的文件描述表
    if (fd == NO_FD) { // 还没有文件描述符
        for (fd = 0; fd < FILES_STRUCT_NENTRY; fd ++, file ++) {
            if (file->status == FD_NONE) { // 找到第一个还未被分配的item
                goto found;
            }
        }
        return -E_MAX_OPEN;
    }
    else {
        if (testfd(fd)) {
            file += fd;
            if (file->status == FD_NONE) {
                goto found;
            }
            return -E_BUSY;
        }
        return -E_INVAL;
    }
found:
    assert(fopen_count(file) == 0);
	file->status = FD_INIT;
	file->node = NULL; // 注意，这个inode节点还是为空，inode节点可是与文件息息相关的
    *file_store = file; // 记录下这个file
    return 0;
}

// fd_array_free - free a file item in open files table
static void
fd_array_free(struct file *file) {
    assert(file->status == FD_INIT || file->status == FD_CLOSED);
    assert(fopen_count(file) == 0);
    if (file->status == FD_CLOSED) {
        vfs_close(file->node);
    }
    file->status = FD_NONE;
}

static void
fd_array_acquire(struct file *file) {
    assert(file->status == FD_OPENED);
    fopen_count_inc(file);
}

// fd_array_release - file's open_count--; if file's open_count-- == 0 , then call fd_array_free to free this file item
static void
fd_array_release(struct file *file) {
    assert(file->status == FD_OPENED || file->status == FD_CLOSED);
    assert(fopen_count(file) > 0);
    if (fopen_count_dec(file) == 0) {
        fd_array_free(file);
    }
}

// fd_array_open - file's open_count++, set status to FD_OPENED
void
fd_array_open(struct file *file) {
    assert(file->status == FD_INIT && file->node != NULL);
    file->status = FD_OPENED;
    fopen_count_inc(file);
}

// fd_array_close - file's open_count--; if file's open_count-- == 0 , then call fd_array_free to free this file item
void
fd_array_close(struct file *file) {
    assert(file->status == FD_OPENED);
    assert(fopen_count(file) > 0);
    file->status = FD_CLOSED;
    if (fopen_count_dec(file) == 0) {
        fd_array_free(file);
    }
}

//fs_array_dup - duplicate file 'from'  to file 'to'
void
fd_array_dup(struct file *to, struct file *from) {
    //cprintf("[fd_array_dup]from fd=%d, to fd=%d\n",from->fd, to->fd);
    assert(to->status == FD_INIT && from->status == FD_OPENED);
    to->pos = from->pos; // 还需要记录当前文件当前的位置
    to->readable = from->readable; // 记录是否可读
    to->writable = from->writable; // 是否可写
    struct inode *node = from->node; // 指向一个抽象的文件描述inode
	vop_ref_inc(node); // 对于这个node的引用计数加1
	vop_open_inc(node); // 对这个node的打开计数加1，这个增加的是inode的open_count
    to->node = node; 
    fd_array_open(to);  // 而fd_array_open(to)增加的是file的open_count
}

// fd2file - use fd as index of fd_array, return the array item (file)
static inline int
fd2file(int fd, struct file **file_store) {
    if (testfd(fd)) { // 应该是首先要判断fd是一个文件描述符
        struct file *file = get_fd_array() + fd; // 找到对应的file
        if (file->status == FD_OPENED && file->fd == fd) {
            *file_store = file; // 然后记录下file
            return 0;
        }
    }
    return -E_INVAL;
}

// file_testfd - test file is readble or writable?
// 用于测试文件描述符
bool
file_testfd(int fd, bool readable, bool writable) {
    int ret;
    struct file *file;
    if ((ret = fd2file(fd, &file)) != 0) { // 如果没有找到对应的文件
        return 0;
    }
    if (readable && !file->readable) { // 如果不可读
        return 0;
    }
    if (writable && !file->writable) { // 如果不可写
        return 0;
    }
    return 1;
}

// open file
int
file_open(char *path, uint32_t open_flags) { // 给出一个path，以及打开的标志
	cprintf("\n==> file_open\n");
    bool readable = 0, writable = 0;
    switch (open_flags & O_ACCMODE) {
    case O_RDONLY: readable = 1; break;
    case O_WRONLY: writable = 1; break;
    case O_RDWR:
        readable = writable = 1;
        break;
    default:
        return -E_INVAL;
    }

    int ret;
    struct file *file;
    if ((ret = fd_array_alloc(NO_FD, &file)) != 0) {
        return ret;
    }

    struct inode *node;
    if ((ret = vfs_open(path, open_flags, &node)) != 0) {
        fd_array_free(file);
        return ret;
    }

    file->pos = 0;
    if (open_flags & O_APPEND) {
        struct stat __stat, *stat = &__stat;
        if ((ret = vop_fstat(node, stat)) != 0) {
            vfs_close(node);
            fd_array_free(file);
            return ret;
        }
        file->pos = stat->st_size;
    }

    file->node = node;
    file->readable = readable;
    file->writable = writable;
    fd_array_open(file);
    return file->fd;
}

// close file
int
file_close(int fd) {
    int ret;
    struct file *file;
    if ((ret = fd2file(fd, &file)) != 0) {
        return ret;
    }
    fd_array_close(file);
    return 0;
}

// read file
int
file_read(int fd, void *base, size_t len, size_t *copied_store) {
    int ret;
    struct file *file;
    *copied_store = 0;
    if ((ret = fd2file(fd, &file)) != 0) {
        return ret;
    }
    if (!file->readable) {
        return -E_INVAL;
    }
    fd_array_acquire(file); // fd_array_acquire大概只是给file分配了一个fd，并没有实际关联到实际的文件上
	//assert(file->node == NULL); // 也就是说,执行到这里的时候file->node == NULL
	//cprintf("file -> node == NULL ? %d\n", file->node == NULL);
    struct iobuf __iob, *iob = iobuf_init(&__iob, base, len, file->pos);
    ret = vop_read(file->node, iob); // 这一步是要执行读入inode

    size_t copied = iobuf_used(iob);
    if (file->status == FD_OPENED) {
        file->pos += copied;
    }
    *copied_store = copied;
    fd_array_release(file);
    return ret;
}

// write file
int
file_write(int fd, void *base, size_t len, size_t *copied_store) {
    int ret;
    struct file *file;
    *copied_store = 0;
    if ((ret = fd2file(fd, &file)) != 0) {
        return ret;
    }
    if (!file->writable) {
        return -E_INVAL;
    }
    fd_array_acquire(file);

    struct iobuf __iob, *iob = iobuf_init(&__iob, base, len, file->pos);
    ret = vop_write(file->node, iob);

    size_t copied = iobuf_used(iob);
    if (file->status == FD_OPENED) {
        file->pos += copied;
    }
    *copied_store = copied;
    fd_array_release(file);
    return ret;
}

// seek file
int
file_seek(int fd, off_t pos, int whence) {
    struct stat __stat, *stat = &__stat;
    int ret;
    struct file *file;
    if ((ret = fd2file(fd, &file)) != 0) {
        return ret;
    }
    fd_array_acquire(file);

    switch (whence) {
    case LSEEK_SET: break;
    case LSEEK_CUR: pos += file->pos; break;
    case LSEEK_END:
        if ((ret = vop_fstat(file->node, stat)) == 0) {
            pos += stat->st_size;
        }
        break;
    default: ret = -E_INVAL;
    }

    if (ret == 0) {
        if ((ret = vop_tryseek(file->node, pos)) == 0) {
            file->pos = pos;
        }
//    cprintf("file_seek, pos=%d, whence=%d, ret=%d\n", pos, whence, ret);
    }
    fd_array_release(file);
    return ret;
}

// stat file
int
file_fstat(int fd, struct stat *stat) {
	cprintf("\n==>file_fstat\n");
    int ret;
    struct file *file;
    if ((ret = fd2file(fd, &file)) != 0) {
        return ret;
    }
    fd_array_acquire(file);
    ret = vop_fstat(file->node, stat);
    fd_array_release(file);
    return ret;
}

// sync file
int
file_fsync(int fd) {
    int ret;
    struct file *file;
    if ((ret = fd2file(fd, &file)) != 0) {
        return ret;
    }
    fd_array_acquire(file);
    ret = vop_fsync(file->node);
    fd_array_release(file);
    return ret;
}

// get file entry in DIR
int
file_getdirentry(int fd, struct dirent *direntp) {
    int ret;
    struct file *file;
    if ((ret = fd2file(fd, &file)) != 0) {
        return ret;
    }
    fd_array_acquire(file);

    struct iobuf __iob, *iob = iobuf_init(&__iob, direntp->name, sizeof(direntp->name), direntp->offset);
    if ((ret = vop_getdirentry(file->node, iob)) == 0) {
        direntp->offset += iobuf_used(iob);
    }
    fd_array_release(file);
    return ret;
}

// duplicate file
int
file_dup(int fd1, int fd2) {
    int ret;
    struct file *file1, *file2;
    if ((ret = fd2file(fd1, &file1)) != 0) {
        return ret;
    }
    if ((ret = fd_array_alloc(fd2, &file2)) != 0) {
        return ret;
    }
    fd_array_dup(file2, file1);
    return file2->fd;
}


