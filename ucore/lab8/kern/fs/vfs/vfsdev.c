#include <defs.h>
#include <stdio.h>
#include <string.h>
#include <vfs.h>
#include <dev.h>
#include <inode.h>
#include <sem.h>
#include <list.h>
#include <kmalloc.h>
#include <unistd.h>
#include <error.h>
#include <assert.h>

// device info entry in vdev_list 
// 这个数据结构将device和inode联通起来
typedef struct {
    const char *devname; // 设备的名称
    struct inode *devnode; // 设备对应的inode节点
    struct fs *fs; // file system
    bool mountable; // 是否可以挂载
    list_entry_t vdev_link;
} vfs_dev_t;

#define le2vdev(le, member)                         \
    to_struct((le), vfs_dev_t, member)

static list_entry_t vdev_list;     // device info list in vfs layer
static semaphore_t vdev_list_sem; // 对设备的访问是互斥的

static void
lock_vdev_list(void) {
    down(&vdev_list_sem);
}

static void
unlock_vdev_list(void) {
    up(&vdev_list_sem);
}

void
vfs_devlist_init(void) {
    list_init(&vdev_list);
    sem_init(&vdev_list_sem, 1);
}

// vfs_cleanup - finally clean (or sync) fs
void
vfs_cleanup(void) {
    if (!list_empty(&vdev_list)) {
        lock_vdev_list();
        {
            list_entry_t *list = &vdev_list, *le = list;
            while ((le = list_next(le)) != list) {
                vfs_dev_t *vdev = le2vdev(le, vdev_link);
                if (vdev->fs != NULL) {
                    fsop_cleanup(vdev->fs);
                }
            }
        }
        unlock_vdev_list();
    }
}

/*
 * vfs_get_root - Given a device name (stdin, stdout, etc.), hand
 *                back an appropriate inode.
 */
// 给定一个设备的名称，得到该设备对应的inode节点
int
vfs_get_root(const char *devname, struct inode **node_store) {
    assert(devname != NULL);
    int ret = -E_NO_DEV;
    if (!list_empty(&vdev_list)) { // 从设备列表中开始寻找
        lock_vdev_list();
        {
			list_entry_t *list = &vdev_list;
			list_entry_t *le = list;
            while ((le = list_next(le)) != list) { // 开始遍历
                vfs_dev_t *vdev = le2vdev(le, vdev_link);
                if (strcmp(devname, vdev->devname) == 0) { // 如果设备名称是一致的话
                    struct inode *found = NULL;
                    if (vdev->fs != NULL) {
                        found = fsop_get_root(vdev->fs);
                    }
                    else if (!vdev->mountable) {
                        vop_ref_inc(vdev->devnode);
                        found = vdev->devnode;
                    }
                    if (found != NULL) {
                        ret = 0, *node_store = found;
                    }
                    else {
                        ret = -E_NA_DEV;
                    }
                    break;
                }
            }
        }
        unlock_vdev_list();
    }
    return ret;
}

/*
 * vfs_get_devname - Given a filesystem, hand back the name of the device it's mounted on.
 */
// 给定一个文件系统，返回设备的名称
const char *
vfs_get_devname(struct fs *fs) {
    assert(fs != NULL);
	list_entry_t *list = &vdev_list;
	list_entry_t *le = list;

    while ((le = list_next(le)) != list) {
        vfs_dev_t *vdev = le2vdev(le, vdev_link);
        if (vdev->fs == fs) { // 每一个设备都有自己对应的文件系统?
            return vdev->devname;
        }
    }
    return NULL;
}

/*
 * check_devname_confilct - Is there alreadily device which has the same name?
 */
// 用于检测设备的冲突,其实就是比较设备的名称而已
static bool
check_devname_conflict(const char *devname) {
    list_entry_t *list = &vdev_list, *le = list;
    while ((le = list_next(le)) != list) {
        vfs_dev_t *vdev = le2vdev(le, vdev_link);
        if (strcmp(vdev->devname, devname) == 0) {
            return 0;
        }
    }
    return 1;
}


/*
* vfs_do_add - Add a new device to the VFS layer's device table.
*
* If "mountable" is set, the device will be treated as one that expects
* to have a filesystem mounted on it, and a raw device will be created
* for direct access.
*/
// 添加一个新的设备到vfs里面,如果mountable设置了,那么设备将会被认为是有一个文件系统挂载在上面,
static int
vfs_do_add(const char *devname, struct inode *devnode, struct fs *fs, bool mountable) {
    assert(devname != NULL);
    assert((devnode == NULL && !mountable) || (devnode != NULL && check_inode_type(devnode, device)));
    if (strlen(devname) > FS_MAX_DNAME_LEN) { // 首先是检查设备名的长度
        return -E_TOO_BIG;
    }

    int ret = -E_NO_MEM;
    char *s_devname;
    if ((s_devname = strdup(devname)) == NULL) {
        return ret;
    }

    vfs_dev_t *vdev;
    if ((vdev = kmalloc(sizeof(vfs_dev_t))) == NULL) { // 动态分配一个节点
        goto failed_cleanup_name;
    }

    ret = -E_EXISTS;
    lock_vdev_list();
    if (!check_devname_conflict(s_devname)) { // 设备名不能冲突
        unlock_vdev_list();
        goto failed_cleanup_vdev;
    }
    vdev->devname = s_devname;
    vdev->devnode = devnode;
    vdev->mountable = mountable;
    vdev->fs = fs;

    list_add(&vdev_list, &(vdev->vdev_link)); // 添加到设备链表之中
    unlock_vdev_list();
    return 0;

failed_cleanup_vdev:
    kfree(vdev);
failed_cleanup_name:
    kfree(s_devname);
    return ret;
}

/*
 * vfs_add_fs - Add a new fs,  by name. See  vfs_do_add information for the description of
 *              mountable.
 */
int
vfs_add_fs(const char *devname, struct fs *fs) {
    return vfs_do_add(devname, NULL, fs, 0);
}

/*
 * vfs_add_dev - Add a new device, by name. See  vfs_do_add information for the description of
 *               mountable.
 */
int
vfs_add_dev(const char *devname, struct inode *devnode, bool mountable) {
    return vfs_do_add(devname, devnode, NULL, mountable);
}

/*
 * find_mount - Look for a mountable device named DEVNAME.
 *              Should already hold vdev_list lock.
 */
static int
find_mount(const char *devname, vfs_dev_t **vdev_store) {
    assert(devname != NULL);
    list_entry_t *list = &vdev_list, *le = list;
    while ((le = list_next(le)) != list) {
        vfs_dev_t *vdev = le2vdev(le, vdev_link);
        if (vdev->mountable && strcmp(vdev->devname, devname) == 0) {
            *vdev_store = vdev;
            return 0;
        }
    }
    return -E_NO_DEV;
}

/*
 * vfs_mount - Mount a filesystem. Once we've found the device, call MOUNTFUNC to
 *             set up the filesystem and hand back a struct fs.
 *
 * The DATA argument is passed through unchanged to MOUNTFUNC.
 */
// 所谓的mount,究竟是什么意思?
int
vfs_mount(const char *devname, int (*mountfunc)(struct device *dev, struct fs **fs_store)) {
    int ret;
    lock_vdev_list(); // 对于设备链表加锁,也就是在这个函数进行期间不能改变
    vfs_dev_t *vdev;
    if ((ret = find_mount(devname, &vdev)) != 0) {
        goto out;
    }
    if (vdev->fs != NULL) { // fs域代表文件系统
        ret = -E_BUSY;
        goto out;
    }
    assert(vdev->devname != NULL && vdev->mountable);

    struct device *dev = vop_info(vdev->devnode, device);
    if ((ret = mountfunc(dev, &(vdev->fs))) == 0) { // 挂载函数是干什么的?
        assert(vdev->fs != NULL);
        cprintf("vfs: mount %s.\n", vdev->devname);
    }

out:
    unlock_vdev_list();
    return ret;
}

/*
 * vfs_unmount - Unmount a filesystem/device by name.
 *               First calls FSOP_SYNC on the filesystem; then calls FSOP_UNMOUNT.
 */
int
vfs_unmount(const char *devname) {
    int ret;
    lock_vdev_list();
    vfs_dev_t *vdev;
    if ((ret = find_mount(devname, &vdev)) != 0) {
        goto out;
    }
    if (vdev->fs == NULL) {
        ret = -E_INVAL;
        goto out;
    }
    assert(vdev->devname != NULL && vdev->mountable);

    if ((ret = fsop_sync(vdev->fs)) != 0) { // 先同步文件
        goto out;
    }
    if ((ret = fsop_unmount(vdev->fs)) == 0) {
        vdev->fs = NULL;
        cprintf("vfs: unmount %s.\n", vdev->devname);
    }

out:
    unlock_vdev_list();
    return ret;
}

/*
 * vfs_unmount_all - Global unmount function.
 */
int
vfs_unmount_all(void) {
    if (!list_empty(&vdev_list)) {
        lock_vdev_list();
        {
            list_entry_t *list = &vdev_list, *le = list;
            while ((le = list_next(le)) != list) {
                vfs_dev_t *vdev = le2vdev(le, vdev_link);
                if (vdev->mountable && vdev->fs != NULL) {
                    int ret;
                    if ((ret = fsop_sync(vdev->fs)) != 0) {
                        cprintf("vfs: warning: sync failed for %s: %e.\n", vdev->devname, ret);
                        continue ;
                    }
                    if ((ret = fsop_unmount(vdev->fs)) != 0) {
                        cprintf("vfs: warning: unmount failed for %s: %e.\n", vdev->devname, ret);
                        continue ;
                    }
                    vdev->fs = NULL;
                    cprintf("vfs: unmount %s.\n", vdev->devname);
                }
            }
        }
        unlock_vdev_list();
    }
    return 0;
}

