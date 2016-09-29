#include <defs.h>
#include <string.h>
#include <vfs.h>
#include <inode.h>
#include <unistd.h>
#include <error.h>
#include <assert.h>


// open file in vfs, get/create inode for file with filename path.
// vfs -- visual file system
// 给定一个path，在vfs中打开文件
int
vfs_open(char *path, uint32_t open_flags, struct inode **node_store) {
	cprintf("\n==>vfs_open\n");
	cprintf("path = %s\n", path);
    bool can_write = 0;
    switch (open_flags & O_ACCMODE) { // 检测打开标志
    case O_RDONLY:
        break;
    case O_WRONLY:
    case O_RDWR:
        can_write = 1;
        break;
    default:
        return -E_INVAL;
    }

    if (open_flags & O_TRUNC) { // 截断,暂时还是不要添加那么复杂的参数吧，ucore到现在已经非常复杂了
        if (!can_write) {
            return -E_INVAL;
        }
    }

    int ret; 
    struct inode *node;
    bool excl = (open_flags & O_EXCL) != 0;
    bool create = (open_flags & O_CREAT) != 0;
    ret = vfs_lookup(path, &node); // 根据path，得到对应文件的inode信息，记录在node之中

    if (ret != 0) { // 如果ret != 0,表示出现了错误
        if (ret == -16 && (create)) {
            char *name;
            struct inode *dir;
            if ((ret = vfs_lookup_parent(path, &dir, &name)) != 0) {
                return ret;
            }
            ret = vop_create(dir, name, excl, &node);
        } else return ret;
    } else if (excl && create) {
        return -E_EXISTS;
    }
    assert(node != NULL);
	// vop_open函数这个宏实际上调用的是node的in_ops域中对应的函数,
	// 如果node代表普通文件，在ucore中实际调用sfs_openfile函数，什么事情也没干
	// 如果node代表文件夹，则实际调用sfs_opendir,它只是对打开参数做了一些检测
    if ((ret = vop_open(node, open_flags)) != 0) { 
        vop_ref_dec(node);
        return ret;
    }

    vop_open_inc(node);
    if (open_flags & O_TRUNC || create) {
        if ((ret = vop_truncate(node, 0)) != 0) {
            vop_open_dec(node);
            vop_ref_dec(node);
            return ret;
        }
    }
    *node_store = node;
    return 0;
}

// close file in vfs
// 在vfs中关闭文件
int
vfs_close(struct inode *node) {
    vop_open_dec(node);
    vop_ref_dec(node);
    return 0;
}

// unimplement
int
vfs_unlink(char *path) {
    return -E_UNIMP;
}

// unimplement
int
vfs_rename(char *old_path, char *new_path) {
    return -E_UNIMP;
}

// unimplement
int
vfs_link(char *old_path, char *new_path) {
    return -E_UNIMP;
}

// unimplement
int
vfs_symlink(char *old_path, char *new_path) {
    return -E_UNIMP;
}

// unimplement
int
vfs_readlink(char *path, struct iobuf *iob) {
    return -E_UNIMP;
}

// unimplement
int
vfs_mkdir(char *path){
    return -E_UNIMP;
}
