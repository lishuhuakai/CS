#include <defs.h>
#include <string.h>
#include <vfs.h>
#include <inode.h>
#include <error.h>
#include <assert.h>

/*
 * get_device- Common code to pull the device name, if any, off the front of a
 *             path and choose the inode to begin the name lookup relative to.
 */
// get_device 函数主要是用于找到
static int
get_device(char *path, char **subpath, struct inode **node_store) {
    int i, slash = -1, colon = -1; // slash / colon :
    for (i = 0; path[i] != '\0'; i ++) { // 开始遍历
        if (path[i] == ':') { colon = i; break; } // 记录下冒号的位置
        if (path[i] == '/') { slash = i; break; } // 记录下 slash 的位置
    }

    if (colon < 0 && slash != 0) { 
        /* *
         * No colon before a slash, so no device name specified, and the slash isn't leading
         * or is also absent, so this is a relative path or just a bare filename. Start from
         * the current directory, and use the whole thing as the subpath.
         * */
		/// 也就是说，在slash的前边没有colon，所以并没有指定设备的名称，并且slash不在开始的位置，或者并没有slash
		/// 这意味着这是一个相对路径，或者只是一个文件名，所以从当前的文件夹开始
		/// 
        *subpath = path; // 子路径并没有发生改变
        return vfs_get_curdir(node_store);
    }
    if (colon > 0) {
        /* device:path - get root of device's filesystem */
        path[colon] = '\0'; // 这是要得到设备名的节奏

        /* device:/path - skip slash, treat as device:path */
        while (path[++ colon] == '/');
        *subpath = path + colon; //取得子路径
        return vfs_get_root(path, node_store); // node_store记录下对应设备的inode信息
    }

    /* *
     * we have either /path or :path
     * /path is a path relative to the root of the "boot filesystem"
     * :path is a path relative to the root of the current filesystem
     * */

	 // 如果path = "/test/testfile"，则i = 0, slash = 0 colon = -1 
    int ret;
    if (*path == '/') {
        if ((ret = vfs_get_bootfs(node_store)) != 0) { // 一般来说，函数返回之后，node_sotre是磁盘的'/'对应的inode
            return ret;
        }
    }
    else {
        assert(*path == ':');
        struct inode *node;
        if ((ret = vfs_get_curdir(&node)) != 0) { // 得到当前目录对应的inode-
            return ret;
        }
        /* The current directory may not be a device, so it must have a fs. */
        assert(node->in_fs != NULL);
        *node_store = fsop_get_root(node->in_fs);
        vop_ref_dec(node);
    }

    /* ///... or :/... */
    while (*(++path) == '/');// path向前移动,去除path前的'/'
    *subpath = path;
    return 0;
}

/*
 * vfs_lookup - get the inode according to the path filename
 */
// 根据路径的文件名，得到对应的inode
int
vfs_lookup(char *path, struct inode **node_store) {
	cprintf("\n==>vfs_lookup\n");
	cprintf("path = %s\n", path);
    int ret;
    struct inode *node;
    if ((ret = get_device(path, &path, &node)) != 0) { // node用于记录path对应的设备的inode信息,这里的path可能会被改变
		// 举个例子如果path = "/test/text.c" ,调用之后 path = "test/text.c"
		// 但是在ucore中，由于文件最多只有一层，所以做了一点简化，不可能出现/test/text.c这种情况，只有/test.c这种情况
		// 当然，你可以扩展ucore的文件系统
        return ret;
    }
	cprintf("path = %s\n", path);
    if (*path != '\0') {
		// node表示文件夹类型的文件对应的inode
		// path在ucore中代表在node指代的这个文件夹下面的一个文件的文件名
		// node_store用于存储宏vop_lookup返回的path文件对应的inode节点
        ret = vop_lookup(node, path, node_store); // 调用lookup函数的时候会将文件的信息加载到node_store中去
        vop_ref_dec(node);
        return ret;
    }
    *node_store = node;
    return 0;
}

/*
 * vfs_lookup_parent - Name-to-vnode translation.
 *  (In BSD, both of these are subsumed by namei().)
 */
// 用于查找父节点
// 貌似在ucore中只有一个，那就是'/'对应的inode
int
vfs_lookup_parent(char *path, struct inode **node_store, char **endp){
    int ret;
    struct inode *node;
    if ((ret = get_device(path, &path, &node)) != 0) {
        return ret;
    }
    *endp = path;
    *node_store = node;
    return 0;
}
