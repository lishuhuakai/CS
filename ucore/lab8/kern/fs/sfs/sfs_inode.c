#include <defs.h>
#include <string.h>
#include <stdlib.h>
#include <list.h>
#include <stat.h>
#include <kmalloc.h>
#include <vfs.h>
#include <dev.h>
#include <sfs.h>
#include <inode.h>
#include <iobuf.h>
#include <bitmap.h>
#include <error.h>
#include <assert.h>

static const struct inode_ops sfs_node_dirops;  // dir operations
static const struct inode_ops sfs_node_fileops; // file operations

/*
 * lock_sin - lock the process of inode Rd/Wr
 */
static void
lock_sin(struct sfs_inode *sin) { // sfs_inode代表内存中的节点
    down(&(sin->sem)); // 加锁
}

/*
 * unlock_sin - unlock the process of inode Rd/Wr
 */
static void
unlock_sin(struct sfs_inode *sin) {
    up(&(sin->sem)); // 解锁
}

/*
 * sfs_get_ops - return function addr of fs_node_dirops/sfs_node_fileops
 */
static const struct inode_ops *
sfs_get_ops(uint16_t type) { // 返回对应函数的地址
    switch (type) {
    case SFS_TYPE_DIR: // 一种是文件夹类型
        return &sfs_node_dirops;
    case SFS_TYPE_FILE: // 一种是文件类型
        return &sfs_node_fileops;
    }
    panic("invalid file type %d.\n", type);
}

/*
 * sfs_hash_list - return inode entry in sfs->hash_list
 */
// sfs_fs是对于整个文件系统的一个描述
// 好吧，返回挂载在hash_list上面的一个entry
static list_entry_t *
sfs_hash_list(struct sfs_fs *sfs, uint32_t ino) { // inode entry是个啥玩意？
    return sfs->hash_list + sin_hashfn(ino);
}

/*
 * sfs_set_links - link inode sin in sfs->linked-list AND sfs->hash_link
 */
// 添加一个inode
// sfs_fs是对于整个文件系统的一个描述
static void
sfs_set_links(struct sfs_fs *sfs, struct sfs_inode *sin) { // 其实就是将sin添加到sfs->inode_list以及sfs的hash_list上去
    list_add(&(sfs->inode_list), &(sin->inode_link));
    list_add(sfs_hash_list(sfs, sin->ino), &(sin->hash_link));
}

/*
 * sfs_remove_links - unlink inode sin in sfs->linked-list AND sfs->hash_link
 */
static void
sfs_remove_links(struct sfs_inode *sin) {
    list_del(&(sin->inode_link));
    list_del(&(sin->hash_link));
}

/*
 * sfs_block_inuse - check the inode with NO. ino inuse info in bitmap
 */
// 用于测试ino是否正在使用
static bool
sfs_block_inuse(struct sfs_fs *sfs, uint32_t ino) { // 只是测试一下第ino块是否被使用
    if (ino != 0 && ino < sfs->super.blocks) {
        return !bitmap_test(sfs->freemap, ino); // sfs_sf确实是对整个文件系统的一个抽象
    }
    panic("sfs_block_inuse: called out of range (0, %u) %u.\n", sfs->super.blocks, ino);
}

/*
 * sfs_block_alloc -  check and get a free disk block
 */
static int
sfs_block_alloc(struct sfs_fs *sfs, uint32_t *ino_store) { // 先检查，然后试图得到一个空闲的磁盘块
    int ret;
    if ((ret = bitmap_alloc(sfs->freemap, ino_store)) != 0) { // 如果还有空闲的块的话，ino_store已经记录下了空闲块的索引
        return ret;
    }
	// 到了这里一般表示已经没有了空闲块,没有了空闲块，那应该怎么做呢？
    assert(sfs->super.unused_blocks > 0); // 好吧，要看一下超级块究竟是个什么玩意了
	sfs->super.unused_blocks--;
	sfs->super_dirty = 1;
    assert(sfs_block_inuse(sfs, *ino_store)); // 我们要判断*ino_store号节点已经被使用了
    return sfs_clear_block(sfs, *ino_store, 1); // 1代表只清空一块
}

/*
 * sfs_block_free - set related bits for ino block to 1(means free) in bitmap, add sfs->super.unused_blocks, set superblock dirty *
 */
static void
sfs_block_free(struct sfs_fs *sfs, uint32_t ino) {
    assert(sfs_block_inuse(sfs, ino)); // 首先要保证，这一块正处在使用状态
    bitmap_free(sfs->freemap, ino); // 首先将freemap中ino对应的位变为1，表示空闲
	sfs->super.unused_blocks++;
	sfs->super_dirty = 1; // super_dirty代表超级块以及freemap发生了改变
}

/*
 * sfs_create_inode - alloc a inode in memroy, and init din/ino/dirty/reclian_count/sem fields in sfs_inode in inode
 */
// 该函数根据从磁盘读取的sfs_disk_inode结构来构造相应的inode节点
static int
sfs_create_inode(struct sfs_fs *sfs, struct sfs_disk_inode *din, uint32_t ino, struct inode **node_store) {
    struct inode *node;
	// 在内存中分配一个inode
    if ((node = alloc_inode(sfs_inode)) != NULL) { // 动态申请一个inode结构
        vop_init(node, sfs_get_ops(din->type), info2fs(sfs, sfs)); // sfs_get_ops根据din的类型，返回对应的操作函数
        struct sfs_inode *sin = vop_info(node, sfs_inode); // sfs_inode结构
		sin->din = din; // 用于记录磁盘中的disk_inode结构
		sin->ino = ino; // 节点的编号
		sin->dirty = 0; // 表示还没有被修改
		sin->reclaim_count = 1;
        sem_init(&(sin->sem), 1);
        *node_store = node;
		// 如何由inode结构构造sfs_inode结构呢？
        return 0;
    }
    return -E_NO_MEM;
}

/*
 * lookup_sfs_nolock - according ino, find related inode
 *
 * NOTICE: le2sin, info2node MACRO
 */
// 该函数用于寻找对应的ino对应的inode块
// sfs是对整个文件系统的一个抽象
static struct inode *
lookup_sfs_nolock(struct sfs_fs *sfs, uint32_t ino) {
    struct inode *node;
	list_entry_t *list = sfs_hash_list(sfs, ino);
	list_entry_t *le = list;
    while ((le = list_next(le)) != list) { // 开始遍历
        struct sfs_inode *sin = le2sin(le, hash_link);
        if (sin->ino == ino) { // 节点的编号一致的话，说明找到了对应的inode
            node = info2node(sin, sfs_inode);
            if (vop_ref_inc(node) == 1) {
                sin->reclaim_count ++;
            }
            return node; // 找到了就返回
        }
    }
    return NULL; // 否则返回NULL
}

/*
 * sfs_load_inode - If the inode isn't existed, load inode related ino disk block data into a new created inode.
 *                  If the inode is in memory alreadily, then do nothing
 */
// 该函数主要用于加载inode
// 用于加载一个inode，如果inode不存在的话，那么就加载一个
// 如果已经存在了的话，就什么也不干
int
sfs_load_inode(struct sfs_fs *sfs, struct inode **node_store, uint32_t ino) {
	cprintf("\n==>sfs_load_inode\n");
    lock_sfs_fs(sfs);
    struct inode *node;
    if ((node = lookup_sfs_nolock(sfs, ino)) != NULL) { // 测试ino对应的inode是否已经存在在内存之中
        goto out_unlock; // 运行到了这里，说明已经存在了
    }
	// 运行到这一步，说明不存在
    int ret = -E_NO_MEM;
    struct sfs_disk_inode *din;
    if ((din = kmalloc(sizeof(struct sfs_disk_inode))) == NULL) {
        goto failed_unlock;
    }

    assert(sfs_block_inuse(sfs, ino));
	// 这里我有一点好奇，是不是每一磁盘块的开始的地方都有一个sfs_disk_inode结构?
	// 答案是否定的。
    if ((ret = sfs_rbuf(sfs, din, sizeof(struct sfs_disk_inode), ino, 0)) != 0) { // 往din里面读取信息
        goto failed_cleanup_din;
    }

    assert(din->nlinks != 0);
    if ((ret = sfs_create_inode(sfs, din, ino, &node)) != 0) { // 使用din的信息，构建一个inode
        goto failed_cleanup_din;
    }
	// sfs是sfs_fs结构，也就是记录对于整个文件系统的一种抽象
	// 下面的函数主要用于将这个node加入sfs的链表之中
    sfs_set_links(sfs, vop_info(node, sfs_inode)); // vop_info这个宏其实就是返回sfs_inode这个结构，也没有其它的意思

out_unlock:
    unlock_sfs_fs(sfs);
    *node_store = node;
    return 0;

failed_cleanup_din:
    kfree(din);
failed_unlock:
    unlock_sfs_fs(sfs);
    return ret;
}

/*
 * sfs_bmap_get_sub_nolock - according entry pointer entp and index, find the index of indrect disk block
 *                           return the index of indrect disk block to ino_store. no lock protect
 * @sfs:      sfs file system # 对文件系统的一个抽象
 * @entp:     the pointer of index of entry disk block # 指向的磁盘块的一个索引，应该是一级索引吧！
 * @index:    the index of block in indrect block # 非直接索引
 * @create:   BOOL, if the block isn't allocated, if create = 1 the alloc a block,  otherwise just do nothing
 * @ino_store: 0 OR the index of already inused block or new allocated block.
 */
// 一般函数名带有sub的都表示一级索引
static int
sfs_bmap_get_sub_nolock(struct sfs_fs *sfs, uint32_t *entp, uint32_t index, bool create, uint32_t *ino_store) {
    assert(index < SFS_BLK_NENTRY);
    int ret;
    uint32_t ent, ino = 0;
    off_t offset = index * sizeof(uint32_t);  // the offset of entry in entry block
	// if entry block is existd, read the content of entry block into  sfs->sfs_buffer
    if ((ent = *entp) != 0) { // ent代表块的编号,只要*entp的值不为0，即有效
        if ((ret = sfs_rbuf(sfs, &ino, sizeof(uint32_t), ent, offset)) != 0) { // 读取index指示的ent磁盘块上的值，其实就是一个索引32个bit
            return ret;
        }
        if (ino != 0 || !create) {
            goto out;
        }
    }
    else { // 如果*entp为0，表示还没有分配entry block
        if (!create) { // 如果不用分配的话，直接离开
            goto out;
        }
		//if entry block isn't existd, allocated a entry block (for indrect block)
        if ((ret = sfs_block_alloc(sfs, &ent)) != 0) { // sfs_block_alloc返回空闲块的一个索引,ent记录了这个索引
            return ret; // 执行到了这一步的话，说明出错了
        }
    }
    
    if ((ret = sfs_block_alloc(sfs, &ino)) != 0) { // ino得到被分配的块的索引
        goto failed_cleanup;
    }
	// 这里的话ent记录的是entry block的索引
    if ((ret = sfs_wbuf(sfs, &ino, sizeof(uint32_t), ent, offset)) != 0) { // 向ino写到ent所指的entry block的offset的位置
        sfs_block_free(sfs, ino); // 如果不成功的话，就要释放
        goto failed_cleanup;
    }

out:
    if (ent != *entp) { // 如果ent != *entp说明重新创建了entry block
        *entp = ent;
    }
    *ino_store = ino;
    return 0;

failed_cleanup:
    if (ent != *entp) {
        sfs_block_free(sfs, ent);
    }
    return ret;
}

/*
 * sfs_bmap_get_nolock - according sfs_inode and index of block, find the NO. of disk block
 *                       no lock protect
 * @sfs:      sfs file system
 * @sin:      sfs inode in memory
 * @index:    the index of block in inode
 * @create:   BOOL, if the block isn't allocated, if create = 1 the alloc a block,  otherwise just do nothing
 * @ino_store: 0 OR the index of already inused block or new allocated block.
 */

// 这里我要说明一下，一个inode一般而言代表一个文件，这里的index代表文件块的索引
// 这个函数主要是用于返回一个文件第index个文件块的索引，并用*ino_store记录下来
static int
sfs_bmap_get_nolock(struct sfs_fs *sfs, struct sfs_inode *sin, uint32_t index, bool create, uint32_t *ino_store) {
    struct sfs_disk_inode *din = sin->din;
    int ret;
    uint32_t ent, ino;
	// the index of disk block is in the fist SFS_NDIRECT  direct blocks
    if (index < SFS_NDIRECT) {
        if ((ino = din->direct[index]) == 0 && create) { // 我记得一共有12个直接索引，等于0代表为空吗？
            if ((ret = sfs_block_alloc(sfs, &ino)) != 0) { // 得到一个空闲的磁盘块，ino记录下了磁盘块的索引
                return ret;
            }
            din->direct[index] = ino; // 如果没有为空的
            sin->dirty = 1; // inode已经被改变了
        }
        goto out;
    }
    // the index of disk block is in the indirect blocks.
    index -= SFS_NDIRECT; // 也就两级索引，12级之后就是一级索引
    if (index < SFS_BLK_NENTRY) {
        ent = din->indirect; // 得到一级索引块
		// 
        if ((ret = sfs_bmap_get_sub_nolock(sfs, &ent, index, create, &ino)) != 0) { // ent得到的是一级索引块的索引
            return ret;
        }
        if (ent != din->indirect) { // 也就是索引值发生了改变，一般而言，这是分配了一个一级索引块
            assert(din->indirect == 0);
            din->indirect = ent;
            sin->dirty = 1; // inode已经变化了
        }
        goto out;
    } else {
		panic ("sfs_bmap_get_nolock - index out of range");
	}
out:
    assert(ino == 0 || sfs_block_inuse(sfs, ino));
    *ino_store = ino;
    return 0;
}

/*
 * sfs_bmap_free_sub_nolock - set the entry item to 0 (free) in the indirect block
 */
// ent表示entry item,一般是一级索引块的索引
static int
sfs_bmap_free_sub_nolock(struct sfs_fs *sfs, uint32_t ent, uint32_t index) {
    assert(sfs_block_inuse(sfs, ent) && index < SFS_BLK_NENTRY);
	// ent应该是代表entry block的索引
	// index代表的是entry block上面要释放的block的索引
    int ret;
    uint32_t ino, zero = 0;
    off_t offset = index * sizeof(uint32_t);
    if ((ret = sfs_rbuf(sfs, &ino, sizeof(uint32_t), ent, offset)) != 0) { // 读出在ent指示的一级索引块中，offset位置对应的索引，用ino记录下来
        return ret;
    }
    if (ino != 0) { // 表示确实被占用了
        if ((ret = sfs_wbuf(sfs, &zero, sizeof(uint32_t), ent, offset)) != 0) { // 向ent指示的一级索引块中，offset位置写0
            return ret;
        }
        sfs_block_free(sfs, ino); // 释放ino指示的这个block
    }
    return 0;
}

/*
 * sfs_bmap_free_nolock - free a block with logical index in inode and reset the inode's fields
 */
// 释放*sin所指代文件的第index个块
static int
sfs_bmap_free_nolock(struct sfs_fs *sfs, struct sfs_inode *sin, uint32_t index) {
    struct sfs_disk_inode *din = sin->din;
    int ret;
    uint32_t ent, ino;
    if (index < SFS_NDIRECT) {
        if ((ino = din->direct[index]) != 0) { // 直接索引
			// free the block
            sfs_block_free(sfs, ino); // 释放直接索引块
            din->direct[index] = 0;
            sin->dirty = 1;
        }
        return 0;
    }

    index -= SFS_NDIRECT;
    if (index < SFS_BLK_NENTRY) { // 非直接索引
        if ((ent = din->indirect) != 0) {
			// set the entry item to 0 in the indirect block
            if ((ret = sfs_bmap_free_sub_nolock(sfs, ent, index)) != 0) { // 释放一级索引块
                return ret;
            }
        }
        return 0;
    }
    return 0;
}

/*
 * sfs_bmap_load_nolock - according to the DIR's inode and the logical index of block in inode, find the NO. of disk block.
 * @sfs:      sfs file system
 * @sin:      sfs inode in memory
 * @index:    the logical index of disk block in inode
 * @ino_store:the NO. of disk block
 */
// 这个函数主要是用来得到sin对应的文件的第index个物理块的索引
// 将对应sfs_inode的第index个索引指向的block的索引值取出存到相应的指针指向的单元（ino_store）。
// 该函数只接受index <= inode->blocks的参数。当index == inode->blocks时，该函数理解为需要为
// inode增长一个block。并标记inode为dirty（所有对inode数据的修改都要做这样的操作，这样，当inode
// 不再使用的时候，sfs能够保证inode数据能够被写回到磁盘）。sfs_bmap_load_nolock调用的sfs_bmap_get_nolock 
// 来完成相应的操作，阅读sfs_bmap_get_nolock，了解它是如何工作的。（sfs_bmap_get_nolock 只由 sfs_bmap_load_nolock 调用）
static int
sfs_bmap_load_nolock(struct sfs_fs *sfs, struct sfs_inode *sin, uint32_t index, uint32_t *ino_store) {
	// sfs_fs是文件系统的抽象，记录了文件系统的一系列信息
	// sfs_inode是内存中的索引节点
    struct sfs_disk_inode *din = sin->din; // 得到sin对应的物理块上的信息
    assert(index <= din->blocks);
    int ret;
    uint32_t ino;
    bool create = (index == din->blocks); // 当index==din->blocks就要创建新的块
    if ((ret = sfs_bmap_get_nolock(sfs, sin, index, create, &ino)) != 0) { // 用于读取文件第index个文件块的索引,并且用ino记录下来
        return ret;
    }
    assert(sfs_block_inuse(sfs, ino));
    if (create) {
        din->blocks ++; // 物理块数加1
    }
    if (ino_store != NULL) {
        *ino_store = ino;
    }
    return 0;
}

/*
 * sfs_bmap_truncate_nolock - free the disk block at the end of file
 */
// 将多级数据索引表的最后一个entry释放掉。它可以认为是sfs_bmap_load_nolock中，index == inode->blocks 的逆操作。
// 当一个文件或目录被删除时，sfs会循环调用该函数直到inode->blocks减为 0，释放所有的数据页。函数通过sfs_bmap_free_nolock
// 来实现，它应该是sfs_bmap_get_nolock的逆操作。和sfs_bmap_get_nolock 一样，调用 sfs_bmap_free_nolock 也要格外小心。
static int
sfs_bmap_truncate_nolock(struct sfs_fs *sfs, struct sfs_inode *sin) { // 说得好听一点是为了截断
    struct sfs_disk_inode *din = sin->din;
    assert(din->blocks != 0);
    int ret;
    if ((ret = sfs_bmap_free_nolock(sfs, sin, din->blocks - 1)) != 0) { // 释放文件尾部的一个块
        return ret;
    }
    din->blocks --;
    sin->dirty = 1; // 代表inode已经发生了改变
    return 0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	接下来是一些关于文件夹的一些操作函数
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*
 * sfs_dirent_read_nolock - read the file entry from disk block which contains this entry
 * @sfs:      sfs file system
 * @sin:      sfs inode in memory
 * @slot:     the index of file entry
 * @entry:    file entry
 */

// 该函数主要用于读取sin指代的文件类型第slot个索引所指示的块起始位置的sfs_disk_entry结构的数据
// 将目录的第slot个entry读取到指定的内存空间。
static int
sfs_dirent_read_nolock(struct sfs_fs *sfs, struct sfs_inode *sin, int slot, struct sfs_disk_entry *entry) {
	// sfs_fs是对文件系统的一个抽象
	// sfs_inode一般记录了文件的一系列信息
	// slot一般代表索引
    assert(sin->din->type == SFS_TYPE_DIR && (slot >= 0 && slot < sin->din->blocks)); // 要求是文件夹类型，并且slot在范围内
    int ret;
    uint32_t ino;
	// according to the DIR's inode and the slot of file entry, find the index of disk block which contains this file entry
    if ((ret = sfs_bmap_load_nolock(sfs, sin, slot, &ino)) != 0) { // ino记录了包含file entry的块的索引
        return ret;
    }
    assert(sfs_block_inuse(sfs, ino));
	// read the content of file entry in the disk block 
	// 从ino指示的块中读入一个sfs_disk_entry块的数据
    if ((ret = sfs_rbuf(sfs, entry, sizeof(struct sfs_disk_entry), ino, 0)) != 0) { // 这个entry是sds_disk_entry类型的
        return ret;
    }
    entry->name[SFS_MAX_FNAME_LEN] = '\0';
    return 0;
}

#define sfs_dirent_link_nolock_check(sfs, sin, slot, lnksin, name)                  \
    do {                                                                            \
        int err;                                                                    \
        if ((err = sfs_dirent_link_nolock(sfs, sin, slot, lnksin, name)) != 0) {    \
            warn("sfs_dirent_link error: %e.\n", err);                              \
        }                                                                           \
    } while (0)

#define sfs_dirent_unlink_nolock_check(sfs, sin, slot, lnksin)                      \
    do {                                                                            \
        int err;                                                                    \
        if ((err = sfs_dirent_unlink_nolock(sfs, sin, slot, lnksin)) != 0) {        \
            warn("sfs_dirent_unlink error: %e.\n", err);                            \
        }                                                                           \
    } while (0)

/*
 * sfs_dirent_search_nolock - read every file entry in the DIR, compare file name with each entry->name
 *                            If equal, then return slot and NO. of disk of this file's inode
 * @sfs:        sfs file system
 * @sin:        sfs inode in memory
 * @name:       the filename
 * @ino_store:  NO. of disk of this file (with the filename)'s inode
 * @slot:       logical index of file entry (NOTICE: each file entry ocupied one  disk block)
 * @empty_slot: the empty logical index of file entry.
 */
// 该函数主要是用于在sin所指代的文件夹中寻找名为name的文件，如果找到了，用*slot记录下它文件夹中的索引，用*inode_store记录下该文件其实位置的物理块的索引
// 该函数是常用的查找函数。它在目录下查找name，并且返回相应的搜索结果（文件或文件夹）的inode的编号（也是磁盘编号），和相应的entry在该目录
// 的index编号以及目录下的数据页是否有空闲的entry。（SFS 实现里文件的数据页是连续的，不存在任何空洞；而对于目录，数据页不是连续的，当
// 某个entry删除的时候，SFS通过设置entry->ino为0将该entry所在的block标记为free，在需要添加新entry 的时候，SFS 优先使用这些free的
// entry，其次才会去在数据页尾追加新的entry。
static int
sfs_dirent_search_nolock(struct sfs_fs *sfs, struct sfs_inode *sin, const char *name, uint32_t *ino_store, int *slot, int *empty_slot) {
	// name是要查找的文件的名字
	// *inode_store一般是要求我们去改变的，也就是说，我们要用*inode_store记录下name所指代的文件的起始物理块的索引
	// *slot也是要求我们改变的，我们要求出name所指代的文件在文件夹中的索引号
	// *empty_slot记录下文件夹中为空的文件的索引
    assert(strlen(name) <= SFS_MAX_FNAME_LEN);
    struct sfs_disk_entry *entry; // disk_entry表示的是目录的描述信息
    if ((entry = kmalloc(sizeof(struct sfs_disk_entry))) == NULL) { // 分配一个disk_entry的空间
        return -E_NO_MEM;
    }

#define set_pvalue(x, v)            do { if ((x) != NULL) { *(x) = (v); } } while (0)
	int ret, i;
	int nslots = sin->din->blocks; // 获得目录文件所占块的数目
    set_pvalue(empty_slot, nslots); 
    for (i = 0; i < nslots; i ++) { // nslots代表目录文件所占用块的数目，一块一块遍历
        if ((ret = sfs_dirent_read_nolock(sfs, sin, i, entry)) != 0) { // 将sfs_disk_entry结构的信息读入entry中
            goto out;
        }
        if (entry->ino == 0) { 
            set_pvalue(empty_slot, i); // 空闲的slot的索引
            continue ;
        }
        if (strcmp(name, entry->name) == 0) { // 如果名字一致
            set_pvalue(slot, i); // 用*slot记录下索引号
            set_pvalue(ino_store, entry->ino); // 用*inode记录下对应文件所在块的索引
            goto out;
        }
    }
#undef set_pvalue
    ret = -E_NOENT;
out:
    kfree(entry);
    return ret;
}

/*
 * sfs_dirent_findino_nolock - read all file entries in DIR's inode and find a entry->ino == ino
 */
// 该函数的功能主要是从sin所指示的文件夹中读取文件起始位置为ino个物理块的文件的entry，成功返回0，将结构记录在*entry中，否则返回一个非零的数
static int
sfs_dirent_findino_nolock(struct sfs_fs *sfs, struct sfs_inode *sin, uint32_t ino, struct sfs_disk_entry *entry) {
	int ret, i;
	int nslots = sin->din->blocks;
    for (i = 0; i < nslots; i ++) {
        if ((ret = sfs_dirent_read_nolock(sfs, sin, i, entry)) != 0) {
            return ret;
        }
        if (entry->ino == ino) {
            return 0;
        }
    }
    return -E_NOENT;
}

/*
 * sfs_lookup_once - find inode corresponding the file name in DIR's sin inode 
 * @sfs:        sfs file system
 * @sin:        DIR sfs inode in memory
 * @name:       the file name in DIR
 * @node_store: the inode corresponding the file name in DIR
 * @slot:       the logical index of file entry
 */
// 该函数主要用于查询在*sin所指代的文件夹中名为name的文件，如果找到了，则加载该文件的信息到 *node_store之中，用*slot记录下该文件在文件夹中的索引
static int
sfs_lookup_once(struct sfs_fs *sfs, struct sfs_inode *sin, const char *name, struct inode **node_store, int *slot) {
	cprintf("\n==>sfs_lookup_once\n");
	int ret;
    uint32_t ino;
    lock_sin(sin); // 加锁
    {   // ino记录了文件起始位置物理块的索引, slot记录了该文件在file entry中的逻辑索引
        ret = sfs_dirent_search_nolock(sfs, sin, name, &ino, slot, NULL); 
    }
    unlock_sin(sin); // 解锁
    if (ret == 0) { // 表示sfs_dirent_search_nolock函数成功执行
		// load the content of inode with the the NO. of disk block
		// 顺带地，加载这个文件的一些信息到*node_store中,因为我们已经知道这个文件从ino号磁盘块开始
        ret = sfs_load_inode(sfs, node_store, ino); // 加载这个文件
    }
    return ret;
}

// sfs_opendir - just check the opne_flags, now support readonly
// 用于打开文件夹
static int
sfs_opendir(struct inode *node, uint32_t open_flags) { // inode是对文件的一个抽象描述？
	cprintf("\n==>sfs_opendir\n");
    switch (open_flags & O_ACCMODE) {
    case O_RDONLY: // 文件夹只能读，是吧！
        break;
    case O_WRONLY:
    case O_RDWR:
    default:
        return -E_ISDIR;
    }
    if (open_flags & O_APPEND) {
        return -E_ISDIR;
    }
    return 0;
}

// sfs_openfile - open file (no use)
static int
sfs_openfile(struct inode *node, uint32_t open_flags) {
	cprintf("\n==>sfs_openfile\n");
    return 0;
}

// sfs_close - close file
static int
sfs_close(struct inode *node) {
    return vop_fsync(node); // 我们假定vop_fsync关闭了文件
}

/*  
 * sfs_io_nolock - Rd/Wr a file content from offset position to offset+ length  disk blocks<-->buffer (in memroy)
 * @sfs:      sfs file system
 * @sin:      sfs inode in memory
 * @buf:      the buffer Rd/Wr
 * @offset:   the offset of file
 * @alenp:    the length need to read (is a pointer). and will RETURN the really Rd/Wr lenght
 * @write:    BOOL, 0 read, 1 write
 */
// 该函数主要是用于读写文件
static int
sfs_io_nolock(struct sfs_fs *sfs, struct sfs_inode *sin, void *buf, off_t offset, size_t *alenp, bool write) {
	// offset指示文件的偏移量
	// alenp指示需要读的长度
    struct sfs_disk_inode *din = sin->din; // sfs_disk_inode用于指示磁盘上的节点
    assert(din->type != SFS_TYPE_DIR); // 要求不是文件
	off_t endpos = offset + *alenp; // alenp指代需要读的长度, 所以endpos代表的是终止的位置
	off_t blkoff;
    *alenp = 0;
	// calculate the Rd/Wr end position
	// 计算读写的终止位置
    if (offset < 0 || offset >= SFS_MAX_FILE_SIZE || offset > endpos) {
        return -E_INVAL;
    }
    if (offset == endpos) { // 长度为0，不用读
        return 0;
    }
    if (endpos > SFS_MAX_FILE_SIZE) { 
        endpos = SFS_MAX_FILE_SIZE;
    }
    if (!write) { // 只需要读的话
        if (offset >= din->size) { // din->size代表的是文件的长度
            return 0;
        }
        if (endpos > din->size) {
            endpos = din->size;
        }
    }

    int (*sfs_buf_op)(struct sfs_fs *sfs, void *buf, size_t len, uint32_t blkno, off_t offset); // 定义两个指针
    int (*sfs_block_op)(struct sfs_fs *sfs, void *buf, uint32_t blkno, uint32_t nblks);
    if (write) {
		sfs_buf_op = sfs_wbuf; // 用于写任意长度的buf数据
		sfs_block_op = sfs_wblock; // 用于写块数据
    }
    else {
		sfs_buf_op = sfs_rbuf;
		sfs_block_op = sfs_rblock;
    }

    int ret = 0;
    size_t size, alen = 0;
    uint32_t ino;
	// 每一块的大小是4096个字节，也就是4KB
    uint32_t blkno = offset / SFS_BLKSIZE;          // The NO. of Rd/Wr begin block,先求得是第几块开始写
    uint32_t nblks = endpos / SFS_BLKSIZE - blkno;  // The size of Rd/Wr blocks,然后求得应该写入几块

  //LAB8:EXERCISE1 YOUR CODE HINT: call sfs_bmap_load_nolock, sfs_rbuf, sfs_rblock,etc. read different kind of blocks in file
	/*
	 * (1) If offset isn't aligned with the first block, Rd/Wr some content from offset to the end of the first block
	 *       NOTICE: useful function: sfs_bmap_load_nolock, sfs_buf_op
	 *               Rd/Wr size = (nblks != 0) ? (SFS_BLKSIZE - blkoff) : (endpos - offset)
	 * (2) Rd/Wr aligned blocks 
	 *       NOTICE: useful function: sfs_bmap_load_nolock, sfs_block_op
     * (3) If end position isn't aligned with the last block, Rd/Wr some content from begin to the (endpos % SFS_BLKSIZE) of the last block
	 *       NOTICE: useful function: sfs_bmap_load_nolock, sfs_buf_op	
	 */

	// offset是文件在块中开始的位置, offset % SFS_BLKSIZE表示开始的位置和边界的偏移量
	// 如果nblk != 0，则
	// +				      offset --->   +						 +  <--- SFS_BLKSIZE
	// +									+--------> size <------- +
	// +------------------------------------+------------------------+
    if ((blkoff = offset % SFS_BLKSIZE) != 0) { // 还带有一点儿偏移,也就是说第一个block并没有和块的边界对齐
        size = (nblks != 0) ? (SFS_BLKSIZE - blkoff) : (endpos - offset);
		// nblks 代表要读/写入的块的数目,如果nblks == 0,表示都在一个块里 读/写
		// size表示这次要写入数据的大小
        if ((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0) { // 这里的blkno代表的是索引, ino记录下了返回的值
            goto out;
        }
        if ((ret = sfs_buf_op(sfs, buf, size, ino, blkoff)) != 0) { // sfs_buf_op一般用于写入/读取size长度的数据, blkoff代表偏移量
            goto out;
        }
        alen += size;
        if (nblks == 0) {
            goto out;
        }
		buf += size;
		blkno++;
		nblks--;
    }

    size = SFS_BLKSIZE;
    while (nblks != 0) { // 接下来是一个一个读取，写入
		// blkno代表下一个文件块的索引
        if ((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0) { // ino得到blkno对应的文件块在磁盘上面的索引
            goto out;
        }
        if ((ret = sfs_block_op(sfs, buf, ino, 1)) != 0) {// 继续读取/写入(现在是一块一块读取或者写入)
            goto out;
        }
		alen += size;
		buf += size;
		blkno++; // 下一个文件块的索引(在sin中的索引)
		nblks--; // 需要读取，或者写入的块的数目
    }

    if ((size = endpos % SFS_BLKSIZE) != 0) { // 仍然可能没有读完/写完，需要一个结尾的过程
        if ((ret = sfs_bmap_load_nolock(sfs, sin, blkno, &ino)) != 0) {
            goto out;
        }
        if ((ret = sfs_buf_op(sfs, buf, size, ino, 0)) != 0) {
            goto out;
        }
        alen += size;
    }
out:
    *alenp = alen; // 记录下读入/写入的数据的长度
    if (offset + alen > sin->din->size) {
        sin->din->size = offset + alen;
        sin->dirty = 1;
    }
    return ret;
}

/*
 * sfs_io - Rd/Wr file. the wrapper of sfs_io_nolock
            with lock protect
 */
static inline int
sfs_io(struct inode *node, struct iobuf *iob, bool write) {
	cprintf("\n==>sfs_io\n");
    struct sfs_fs *sfs = fsop_info(vop_fs(node), sfs); // vop_fs返回node对应的in_fs域指向的fs结构
    struct sfs_inode *sin = vop_info(node, sfs_inode); // inode结构里面包含sfs_inode结构
    int ret;
    lock_sin(sin); // 总之是加锁
    {
        size_t alen = iob->io_resid;
        ret = sfs_io_nolock(sfs, sin, iob->io_base, iob->io_offset, &alen, write);
        if (alen != 0) { // alen记录下了读写的数据的长度
            iobuf_skip(iob, alen);
        }
    }
    unlock_sin(sin); // 解锁
    return ret;
}

// sfs_read - read file
// 读文件
static int
sfs_read(struct inode *node, struct iobuf *iob) {
	cprintf("\n==>sfs_read\n");
    return sfs_io(node, iob, 0);
}

// sfs_write - write file
// 写文件
static int
sfs_write(struct inode *node, struct iobuf *iob) {
    return sfs_io(node, iob, 1);
}

/*
 * sfs_fstat - Return nlinks/block/size, etc. info about a file. The pointer is a pointer to struct stat;
 */
static int
sfs_fstat(struct inode *node, struct stat *stat) { // 用于获取文件的一系列信息
	cprintf("\n==>sfs_fstat\n");
    int ret;
    memset(stat, 0, sizeof(struct stat));
    if ((ret = vop_gettype(node, &(stat->st_mode))) != 0) {
        return ret;
    }
    struct sfs_disk_inode *din = vop_info(node, sfs_inode)->din;
    stat->st_nlinks = din->nlinks; 
    stat->st_blocks = din->blocks; // 占用的块的数目
    stat->st_size = din->size;
    return 0;
}

/*
 * sfs_fsync - Force any dirty inode info associated with this file to stable storage.
 */
// 用于文件的同步
static int
sfs_fsync(struct inode *node) {
    struct sfs_fs *sfs = fsop_info(vop_fs(node), sfs); // 得到node对应的sfs_fs结构
    struct sfs_inode *sin = vop_info(node, sfs_inode); // 得到inode对应的sfs_inode结构
    int ret = 0;
    if (sin->dirty) { // 如果inode数据已经发生了改变
        lock_sin(sin);
        {
            if (sin->dirty) {
                sin->dirty = 0;
                if ((ret = sfs_wbuf(sfs, sin->din, sizeof(struct sfs_disk_inode), sin->ino, 0)) != 0) { // 需要写入到磁盘中去
                    sin->dirty = 1;
                }
            }
        }
        unlock_sin(sin);
    }
    return ret;
}

/*
 *sfs_namefile -Compute pathname relative to filesystem root of the file and copy to the specified io buffer.
 *  
 */

static int
sfs_namefile(struct inode *node, struct iobuf *iob) { // 一个inode节点
    struct sfs_disk_entry *entry;
    if (iob->io_resid <= 2 || (entry = kmalloc(sizeof(struct sfs_disk_entry))) == NULL) {
        return -E_NO_MEM;
    }

    struct sfs_fs *sfs = fsop_info(vop_fs(node), sfs);
    struct sfs_inode *sin = vop_info(node, sfs_inode);
	// 这里的sin指代一个文件夹

    int ret;
    char *ptr = iob->io_base + iob->io_resid; // 也就是说从buf的尾部开始
    size_t alen, resid = iob->io_resid - 2;
    vop_ref_inc(node);
    while (1) {
        struct inode *parent;
        if ((ret = sfs_lookup_once(sfs, sin, "..", &parent, NULL)) != 0) { // ".."表示的是上级目录,也就是向上
            goto failed;
        }
		// sin这个inode是对文件夹的一个描述
        uint32_t ino = sin->ino; // ino一般代表文件所在的磁盘号
        vop_ref_dec(node);
        if (node == parent) { // 虽然这种事情一般不会发生，但是如果到了根目录的话，根目录的父目录还是自己
            vop_ref_dec(node);
            break;
        }

        node = parent, sin = vop_info(node, sfs_inode);
        assert(ino != sin->ino && sin->din->type == SFS_TYPE_DIR);

        lock_sin(sin);
        {
            ret = sfs_dirent_findino_nolock(sfs, sin, ino, entry); // 从sin所指示的文件夹中读出起始位置为ino的文件的sfs_disk_entry的结构的信息
        }
        unlock_sin(sin);

        if (ret != 0) {
            goto failed;
        }

        if ((alen = strlen(entry->name) + 1) > resid) {
            goto failed_nomem;
        }
        resid -= alen, ptr -= alen;
        memcpy(ptr, entry->name, alen - 1);
        ptr[alen - 1] = '/';
		// 给我的感觉是，这个函数在从底向上构造出文件的绝对路径
    }
    alen = iob->io_resid - resid - 2;
    ptr = memmove(iob->io_base + 1, ptr, alen);
    ptr[-1] = '/', ptr[alen] = '\0';
    iobuf_skip(iob, alen);
    kfree(entry);
    return 0;

failed_nomem:
    ret = -E_NO_MEM;
failed:
    vop_ref_dec(node);
    kfree(entry);
    return ret;
}

/*
 * sfs_getdirentry_sub_noblock - get the content of file entry in DIR
 */
// 该函数主要用于得到在dir中file entry的内容
// 其实就是得到一个文件夹中第slot个文件的sfs_disk_entry的内容，记录到*entry中
static int
sfs_getdirentry_sub_nolock(struct sfs_fs *sfs, struct sfs_inode *sin, int slot, struct sfs_disk_entry *entry) {
    int ret, i, nslots = sin->din->blocks;
    for (i = 0; i < nslots; i ++) { // 开始遍历
        if ((ret = sfs_dirent_read_nolock(sfs, sin, i, entry)) != 0) { // 将sin所指示的文件夹中第i个文件的sfs_disk_entry的数据读入entry
            return ret;
        }
        if (entry->ino != 0) { // 磁盘索引号不为空，代表该文件存在
            if (slot == 0) { // 为什么要这么玩？
                return 0;
            }
            slot --;
        }
    }
    return -E_NOENT;
}

/*
 * sfs_getdirentry - according to the iob->io_offset, calculate the dir entry's slot in disk block,
                     get dir entry content from the disk 
 */

static int
sfs_getdirentry(struct inode *node, struct iobuf *iob) {
    struct sfs_disk_entry *entry;
    if ((entry = kmalloc(sizeof(struct sfs_disk_entry))) == NULL) { // 先分配空间
        return -E_NO_MEM;
    }

    struct sfs_fs *sfs = fsop_info(vop_fs(node), sfs); // 得到inode对应的sfs_fs结构
    struct sfs_inode *sin = vop_info(node, sfs_inode); // 得到inode对应的sfs_inode结构

    int ret, slot;
    off_t offset = iob->io_offset;
    if (offset < 0 || offset % sfs_dentry_size != 0) {
        kfree(entry);
        return -E_INVAL;
    }
    if ((slot = offset / sfs_dentry_size) > sin->din->blocks) {
        kfree(entry);
        return -E_NOENT;
    }
    lock_sin(sin);
    if ((ret = sfs_getdirentry_sub_nolock(sfs, sin, slot, entry)) != 0) {
        unlock_sin(sin);
        goto out;
    }
    unlock_sin(sin);
    ret = iobuf_move(iob, entry->name, sfs_dentry_size, 1, NULL);
out:
    kfree(entry);
    return ret;
}

/*
 * sfs_reclaim - Free all resources inode occupied . Called when inode is no longer in use. 
 */
// 释放所有的资源
static int
sfs_reclaim(struct inode *node) {
    struct sfs_fs *sfs = fsop_info(vop_fs(node), sfs);
    struct sfs_inode *sin = vop_info(node, sfs_inode);

    int  ret = -E_BUSY;
    uint32_t ent;
    lock_sfs_fs(sfs);
    assert(sin->reclaim_count > 0);
    if ((-- sin->reclaim_count) != 0 || inode_ref_count(node) != 0) {
        goto failed_unlock;
    }
    if (sin->din->nlinks == 0) { // 硬链接数目等于0的话
        if ((ret = vop_truncate(node, 0)) != 0) {
            goto failed_unlock;
        }
    }
    if (sin->dirty) { // 如果发生了改变的话，要同步到文件中去
        if ((ret = vop_fsync(node)) != 0) {
            goto failed_unlock;
        }
    }
    sfs_remove_links(sin);
    unlock_sfs_fs(sfs);

    if (sin->din->nlinks == 0) {
        sfs_block_free(sfs, sin->ino);
        if ((ent = sin->din->indirect) != 0) {
            sfs_block_free(sfs, ent);
        }
    }
    kfree(sin->din);
    vop_kill(node);
    return 0;

failed_unlock:
    unlock_sfs_fs(sfs);
    return ret;
}

/*
 * sfs_gettype - Return type of file. The values for file types are in sfs.h.
 */
// 用于得到文件的类型
static int
sfs_gettype(struct inode *node, uint32_t *type_store) {
    struct sfs_disk_inode *din = vop_info(node, sfs_inode)->din;
    switch (din->type) {
    case SFS_TYPE_DIR: // 文件夹类型
        *type_store = S_IFDIR;
        return 0;
    case SFS_TYPE_FILE: // 普通文件类型
        *type_store = S_IFREG;
        return 0;
    case SFS_TYPE_LINK: // 链接类型
        *type_store = S_IFLNK;
        return 0;
    }
    panic("invalid file type %d.\n", din->type);
}

/* 
 * sfs_tryseek - Check if seeking to the specified position within the file is legal.
 */
// pos是当前访问到的位置
static int
sfs_tryseek(struct inode *node, off_t pos) {
    if (pos < 0 || pos >= SFS_MAX_FILE_SIZE) {
        return -E_INVAL;
    }
    struct sfs_inode *sin = vop_info(node, sfs_inode); // 得到inode对应的sfs_inode的值
    if (pos > sin->din->size) {
        return vop_truncate(node, pos);
    }
    return 0;
}

/*
 * sfs_truncfile : reszie the file with new length
 */
// 重新调整文件的大小
static int
sfs_truncfile(struct inode *node, off_t len) {
    if (len < 0 || len > SFS_MAX_FILE_SIZE) {
        return -E_INVAL;
    }
    struct sfs_fs *sfs = fsop_info(vop_fs(node), sfs);
    struct sfs_inode *sin = vop_info(node, sfs_inode);
    struct sfs_disk_inode *din = sin->din;

    int ret = 0;
	//new number of disk blocks of file
	uint32_t nblks;
	uint32_t tblks = ROUNDUP_DIV(len, SFS_BLKSIZE); // tblks表示一共要占用多少个块
    if (din->size == len) {
        assert(tblks == din->blocks);
        return 0;
    }

    lock_sin(sin);
	// old number of disk blocks of file
    nblks = din->blocks; // nblks用于记录原来文件的块的数目
    if (nblks < tblks) { // 表示文件变大了
		// try to enlarge the file size by add new disk block at the end of file
		// 通过在文件末尾添加新的磁盘块，尝试这扩大文件的大小
        while (nblks != tblks) {
            if ((ret = sfs_bmap_load_nolock(sfs, sin, nblks, NULL)) != 0) {
                goto out_unlock;
            }
            nblks ++;
        }
    }
    else if (tblks < nblks) { // 新的文件变小了
		// try to reduce the file size 
		// 要尝试截断文件
        while (tblks != nblks) {
            if ((ret = sfs_bmap_truncate_nolock(sfs, sin)) != 0) {
                goto out_unlock;
            }
            nblks --;
        }
    }
    assert(din->blocks == tblks);
    din->size = len;
    sin->dirty = 1;

out_unlock:
    unlock_sin(sin);
    return ret;
}

/*
 * sfs_lookup - Parse path relative to the passed directory
 *              DIR, and hand back the inode for the file it
 *              refers to.
 */

// 如果返回值不是0的话，代表出错了
// 实验指导书上感觉有错误，我来说一下自己的理解: node描述了目录项
// path指代的是相对于这个目录项下的文件
// 函数最终要获得下这个文件的inode信息
static int
sfs_lookup(struct inode *node, char *path, struct inode **node_store) {
	// 这里的node一般代表一个设备文件,我们几个例子，假设为根目录'/'所对应的inode节点
	// 假定path是文件"testfile"的路径"test/testfile"
	// 不过，我有一个疑问，那就是，我们应该是现在'/'下找到'test'目录项，在'test'目录项下找到'testfile'这个文件
	// 但是很奇怪，这个函数不是这么干的.
	// 但是考虑到实际的情况，在ucore最多只有一级目录，我们也就释然了，也就是说只有这一种情况"/testfile"
	// 不会出现"/test/testfile"这样两层的情况，当然，你也可以扩展
	cprintf("\n==>sfs_lookup\n");
	cprintf("path = %s\n", path);
    struct sfs_fs *sfs = fsop_info(vop_fs(node), sfs);
    assert(*path != '\0' && *path != '/');
    vop_ref_inc(node); // 增加了引用之后不用害怕被别人释放了
    struct sfs_inode *sin = vop_info(node, sfs_inode);
    if (sin->din->type != SFS_TYPE_DIR) { // 也就是说*sin所指代的文件一定要是一个文件夹
        vop_ref_dec(node);
        return -E_NOTDIR;
    }
    struct inode *subnode; 
    int ret = sfs_lookup_once(sfs, sin, path, &subnode, NULL); // subnode记录了path所指示的文件的信息

    vop_ref_dec(node);
    if (ret != 0) {
        return ret;
    }
    *node_store = subnode; 
    return 0;
}

// The sfs specific DIR operations correspond to the abstract operations on a inode.
static const struct inode_ops sfs_node_dirops = { // 这些是关于文件夹的一些抽象操作
    .vop_magic                      = VOP_MAGIC, // 魔数
    .vop_open                       = sfs_opendir,
    .vop_close                      = sfs_close,
    .vop_fstat                      = sfs_fstat,
    .vop_fsync                      = sfs_fsync,
    .vop_namefile                   = sfs_namefile,
    .vop_getdirentry                = sfs_getdirentry,
    .vop_reclaim                    = sfs_reclaim,
    .vop_gettype                    = sfs_gettype,
    .vop_lookup                     = sfs_lookup,
};
/// The sfs specific FILE operations correspond to the abstract operations on a inode.
static const struct inode_ops sfs_node_fileops = {
    .vop_magic                      = VOP_MAGIC,
    .vop_open                       = sfs_openfile,
    .vop_close                      = sfs_close,
    .vop_read                       = sfs_read,
    .vop_write                      = sfs_write,
    .vop_fstat                      = sfs_fstat,
    .vop_fsync                      = sfs_fsync,
    .vop_reclaim                    = sfs_reclaim,
    .vop_gettype                    = sfs_gettype,
    .vop_tryseek                    = sfs_tryseek,
    .vop_truncate                   = sfs_truncfile,
};

