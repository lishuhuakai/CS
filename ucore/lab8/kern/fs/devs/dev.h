#ifndef __KERN_FS_DEVS_DEV_H__
#define __KERN_FS_DEVS_DEV_H__

#include <defs.h>

struct inode;
struct iobuf;

/*
 * Filesystem-namespace-accessible device.
 * d_io is for both reads and writes; the iobuf will indicates the direction.
 */
struct device {
    size_t d_blocks; // 设备占用的数据块个数
    size_t d_blocksize; // 数据块的大小
    int (*d_open)(struct device *dev, uint32_t open_flags); // 打开设备的函数指针
    int (*d_close)(struct device *dev); // 关闭设备的函数指针
    int (*d_io)(struct device *dev, struct iobuf *iob, bool write); // 读写设备的函数指针
    int (*d_ioctl)(struct device *dev, int op, void *data); // 用ioctl方式控制设备的函数指针
};

#define dop_open(dev, open_flags)           ((dev)->d_open(dev, open_flags))
#define dop_close(dev)                      ((dev)->d_close(dev))
#define dop_io(dev, iob, write)             ((dev)->d_io(dev, iob, write))
#define dop_ioctl(dev, op, data)            ((dev)->d_ioctl(dev, op, data))

void dev_init(void);
struct inode *dev_create_inode(void);

#endif /* !__KERN_FS_DEVS_DEV_H__ */

