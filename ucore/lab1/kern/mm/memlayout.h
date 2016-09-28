#ifndef __KERN_MM_MEMLAYOUT_H__
#define __KERN_MM_MEMLAYOUT_H__

/* This file contains the definitions for memory management in our OS. */

/* global segment number */
// 下面是段
#define SEG_KTEXT    1
#define SEG_KDATA    2
#define SEG_UTEXT    3
#define SEG_UDATA    4
#define SEG_TSS      5

/* global descriptor numbers */
#define GD_KTEXT    ((SEG_KTEXT) << 3)        // kernel text，总之就是凑成选择子，是吧！
#define GD_KDATA    ((SEG_KDATA) << 3)        // kernel data
#define GD_UTEXT    ((SEG_UTEXT) << 3)        // user text
#define GD_UDATA    ((SEG_UDATA) << 3)        // user data
#define GD_TSS        ((SEG_TSS) << 3)        // task segment selector

#define DPL_KERNEL    (0) // kernel的特权级别为0
#define DPL_USER    (3)   // 用户的特权级别为3

#define KERNEL_CS    ((GD_KTEXT) | DPL_KERNEL)  // kernel的代码段选择子
#define KERNEL_DS    ((GD_KDATA) | DPL_KERNEL)  
#define USER_CS        ((GD_UTEXT) | DPL_USER)  // user的代码段选择子
#define USER_DS        ((GD_UDATA) | DPL_USER)

#endif /* !__KERN_MM_MEMLAYOUT_H__ */

