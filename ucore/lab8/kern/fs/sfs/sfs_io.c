#include <defs.h>
#include <string.h>
#include <dev.h>
#include <sfs.h>
#include <iobuf.h>
#include <bitmap.h>
#include <assert.h>

//Basic block-level I/O routines

/* sfs_rwblock_nolock - Basic block-level I/O routine for Rd/Wr one disk block,
 *                      without lock protect for mutex process on Rd/Wr disk block
 * @sfs:   sfs_fs which will be process
 * @buf:   the buffer uesed for Rd/Wr
 * @blkno: the NO. of disk block
 * @write: BOOL: Read or Write
 * @check: BOOL: if check (blono < sfs super.blocks)
 */

// 这里又是一层抽象


static int
sfs_rwblock_nolock(struct sfs_fs *sfs, void *buf, uint32_t blkno, bool write, bool check) {
    assert((blkno != 0 || !check) && blkno < sfs->super.blocks);
    struct iobuf __iob, *iob = iobuf_init(&__iob, buf, SFS_BLKSIZE, blkno * SFS_BLKSIZE);
    return dop_io(sfs->dev, iob, write);
}

// 我们只需要看这几个说明了实际用途的函数即可
/* sfs_rwblock - Basic block-level I/O routine for Rd/Wr N disk blocks ,
 *               with lock protect for mutex process on Rd/Wr disk block
 * @sfs:   sfs_fs which will be process
 * @buf:   the buffer uesed for Rd/Wr
 * @blkno: the NO. of disk block
 * @nblks: Rd/Wr number of disk block
 * @write: BOOL: Read - 0 or Write - 1
 */
static int
sfs_rwblock(struct sfs_fs *sfs, void *buf, uint32_t blkno, uint32_t nblks, bool write) {
    int ret = 0;
    lock_sfs_io(sfs); // 读写都要竞争，是吧！
    {
        while (nblks != 0) {
            if ((ret = sfs_rwblock_nolock(sfs, buf, blkno, write, 1)) != 0) {
                break;
            }
            blkno ++, nblks --;
            buf += SFS_BLKSIZE;
        }
    }
    unlock_sfs_io(sfs);
    return ret;
}

/* sfs_rblock - The Wrap of sfs_rwblock function for Rd N disk blocks ,
 *
 * @sfs:   sfs_fs which will be process
 * @buf:   the buffer uesed for Rd/Wr
 * @blkno: the NO. of disk block
 * @nblks: Rd/Wr number of disk block
 */
// 这里只是做了简单的一层包裹，sfs -- simple file system
int
sfs_rblock(struct sfs_fs *sfs, void *buf, uint32_t blkno, uint32_t nblks) {
    return sfs_rwblock(sfs, buf, blkno, nblks, 0); // 0代表读
}

/* sfs_wblock - The Wrap of sfs_rwblock function for Wr N disk blocks ,
 *
 * @sfs:   sfs_fs which will be process
 * @buf:   the buffer uesed for Rd/Wr
 * @blkno: the NO. of disk block
 * @nblks: Rd/Wr number of disk block
 */
int
sfs_wblock(struct sfs_fs *sfs, void *buf, uint32_t blkno, uint32_t nblks) { // 写入多少块
    return sfs_rwblock(sfs, buf, blkno, nblks, 1); // 1代表写
}

/* sfs_rbuf - The Basic block-level I/O routine for  Rd( non-block & non-aligned io) one disk block(using sfs->sfs_buffer)
 *            with lock protect for mutex process on Rd/Wr disk block
 * @sfs:    sfs_fs which will be process
 * @buf:    the buffer uesed for Rd
 * @len:    the length need to Rd
 * @blkno:  the NO. of disk block
 * @offset: the offset in the content of disk block
 */
int
sfs_rbuf(struct sfs_fs *sfs, void *buf, size_t len, uint32_t blkno, off_t offset) { // 读入多少个字节,偏移量代表从这块那个位置开始读取
    assert(offset >= 0 && offset < SFS_BLKSIZE && offset + len <= SFS_BLKSIZE);
    int ret;
    lock_sfs_io(sfs);
    {
        if ((ret = sfs_rwblock_nolock(sfs, sfs->sfs_buffer, blkno, 0, 1)) == 0) {
            memcpy(buf, sfs->sfs_buffer + offset, len);
        }
    }
    unlock_sfs_io(sfs);
    return ret;
}

/* sfs_wbuf - The Basic block-level I/O routine for  Wr( non-block & non-aligned io) one disk block(using sfs->sfs_buffer)
 *            with lock protect for mutex process on Rd/Wr disk block
 * @sfs:    sfs_fs which will be process
 * @buf:    the buffer uesed for Wr
 * @len:    the length need to Wr
 * @blkno:  the NO. of disk block
 * @offset: the offset in the content of disk block
 */
int
sfs_wbuf(struct sfs_fs *sfs, void *buf, size_t len, uint32_t blkno, off_t offset) { // 写入多少个字节,偏移量代表从这块数据的哪个位置开始写入
    assert(offset >= 0 && offset < SFS_BLKSIZE && offset + len <= SFS_BLKSIZE);
    int ret;
    lock_sfs_io(sfs); // 争抢磁盘io
    {
        if ((ret = sfs_rwblock_nolock(sfs, sfs->sfs_buffer, blkno, 0, 1)) == 0) {
            memcpy(sfs->sfs_buffer + offset, buf, len);
            ret = sfs_rwblock_nolock(sfs, sfs->sfs_buffer, blkno, 1, 1);
        }
    }
    unlock_sfs_io(sfs);
    return ret;
}

/*
 * sfs_sync_super - write sfs->super (in memory) into disk (SFS_BLKN_SUPER, 1) with lock protect.
 */
int
sfs_sync_super(struct sfs_fs *sfs) { // 用于同步超级块吗?
    int ret;
    lock_sfs_io(sfs);
    {
        memset(sfs->sfs_buffer, 0, SFS_BLKSIZE);
        memcpy(sfs->sfs_buffer, &(sfs->super), sizeof(sfs->super));
        ret = sfs_rwblock_nolock(sfs, sfs->sfs_buffer, SFS_BLKN_SUPER, 1, 0); // 写入第0块，话说，超级块究竟是一个什么东西？
    }
    unlock_sfs_io(sfs);
    return ret;
}

/*
 * sfs_sync_freemap - write sfs bitmap into disk (SFS_BLKN_FREEMAP, nblks)  without lock protect.
 */
int
sfs_sync_freemap(struct sfs_fs *sfs) {
    uint32_t nblks = sfs_freemap_blocks(&(sfs->super));
    return sfs_wblock(sfs, bitmap_getdata(sfs->freemap, NULL), SFS_BLKN_FREEMAP, nblks);
}

/*
 * sfs_clear_block - write zero info into disk (blkno, nblks)  with lock protect.
 * @sfs:   sfs_fs which will be process
 * @blkno: the NO. of disk block
 * @nblks: Rd/Wr number of disk block
 */
// 我们来看一下clear_block究竟干了一些什么事情吧！

int
sfs_clear_block(struct sfs_fs *sfs, uint32_t blkno, uint32_t nblks) {
    int ret;
    lock_sfs_io(sfs);
    {
        memset(sfs->sfs_buffer, 0, SFS_BLKSIZE); // 4096字节
        while (nblks != 0) {
			// 下面的1代表写入
            if ((ret = sfs_rwblock_nolock(sfs, sfs->sfs_buffer, blkno, 1, 1)) != 0) { // 其实就是清0,是吧。
                break;
            }
            blkno ++, nblks --;
        }
    }
    unlock_sfs_io(sfs);
    return ret;
}

