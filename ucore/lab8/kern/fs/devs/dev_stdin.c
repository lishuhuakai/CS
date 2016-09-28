#include <defs.h>
#include <stdio.h>
#include <wait.h>
#include <sync.h>
#include <proc.h>
#include <sched.h>
#include <dev.h>
#include <vfs.h>
#include <iobuf.h>
#include <inode.h>
#include <unistd.h>
#include <error.h>
#include <assert.h>

// 标准输入也是一个文件
#define STDIN_BUFSIZE               4096

static char stdin_buffer[STDIN_BUFSIZE];
static off_t p_rpos; // 读的位置
static off_t p_wpos; // 写的位置
static wait_queue_t __wait_queue, *wait_queue = &__wait_queue;

void
dev_stdin_write(char c) {
    bool intr_flag;
    if (c != '\0') { // 
        local_intr_save(intr_flag);
        {
            stdin_buffer[p_wpos % STDIN_BUFSIZE] = c; // 这是一个循环数组
            if (p_wpos - p_rpos < STDIN_BUFSIZE) {
                p_wpos ++;
            }
            if (!wait_queue_empty(wait_queue)) {
                wakeup_queue(wait_queue, WT_KBD, 1);
            }
        }
        local_intr_restore(intr_flag);
    }
}

// 向标准输入里面输入数据
static int
dev_stdin_read(char *buf, size_t len) {
    int ret = 0;
    bool intr_flag;
    local_intr_save(intr_flag); // 首先是关闭中断
    {
        for (; ret < len; ret ++, p_rpos ++) {
        try_again:
            if (p_rpos < p_wpos) {
                *buf ++ = stdin_buffer[p_rpos % STDIN_BUFSIZE]; // 读取数据
            }
            else { // 否则的话，没有数据可以读，所以要等待
                wait_t __wait, *wait = &__wait;
                wait_current_set(wait_queue, wait, WT_KBD); // 等待键盘的输入
                local_intr_restore(intr_flag);

                schedule();
				// 运行到这里，说明被唤醒
                local_intr_save(intr_flag);
                wait_current_del(wait_queue, wait);
                if (wait->wakeup_flags == WT_KBD) { // 总之就是不断尝试，直到读取到了数据
                    goto try_again;
                }
                break;
            }
        }
    }
    local_intr_restore(intr_flag); // 然后开启中断
    return ret;
}

static int
stdin_open(struct device *dev, uint32_t open_flags) {
    if (open_flags != O_RDONLY) { // 只允许读
        return -E_INVAL;
    }
    return 0;
}

static int
stdin_close(struct device *dev) {
    return 0;
}

static int
stdin_io(struct device *dev, struct iobuf *iob, bool write) {
    if (!write) {
        int ret;
        if ((ret = dev_stdin_read(iob->io_base, iob->io_resid)) > 0) {
            iob->io_resid -= ret;
        }
        return ret; // 返回已经读取的字节数
    }
    return -E_INVAL;
}

static int
stdin_ioctl(struct device *dev, int op, void *data) {
    return -E_INVAL; // 这个无法实现ioctl的功能
}

static void
stdin_device_init(struct device *dev) {
    dev->d_blocks = 0; // 字符设备
    dev->d_blocksize = 1;
    dev->d_open = stdin_open;
    dev->d_close = stdin_close;
    dev->d_io = stdin_io;
    dev->d_ioctl = stdin_ioctl;

    p_rpos = p_wpos = 0;
    wait_queue_init(wait_queue); // wait_queue其实是一个表头
}

void
dev_init_stdin(void) {
    struct inode *node;
    if ((node = dev_create_inode()) == NULL) {
        panic("stdin: dev_create_node.\n");
    }
    stdin_device_init(vop_info(node, device)); // node是一个inode的结构,vop_info取得in_info域中的device域的指针

    int ret;
    if ((ret = vfs_add_dev("stdin", node, 0)) != 0) { // 添加到设备列表之中
        panic("stdin: vfs_add_dev: %e.\n", ret);
    }
}

