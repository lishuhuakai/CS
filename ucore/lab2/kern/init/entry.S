#include <mmu.h>
#include <memlayout.h>

#define REALLOC(x) (x - KERNBASE)
# 好吧，不知道从什么时候开始，kernel 的入口地址变成了kern_entry了。
.text
.globl kern_entry
kern_entry:
    # reload temperate gdt (second time) to remap all physical memory
    # virtual_addr 0~4G=linear_addr&physical_addr -KERNBASE~4G-KERNBASE 
	# 因为kernel里的虚拟地址从0xC0100000开始，而实际加载的物理地址为0x00100000,并且这个时候，物理地址和虚拟地址还是一一映射的关系
	# 所以为了找到正确的__gdtdesc的加载地址，所以要减去一个KERNBASE,也就是0xC0000000
    lgdt REALLOC(__gdtdesc)

    movl $KERNEL_DS, %eax # 接下来的话，将kernel的ds段的选择子放入各个段寄存器
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    ljmp $KERNEL_CS, $relocated

relocated:

    # set ebp, esp
    movl $0x0, %ebp
    # the kernel stack region is from bootstack -- bootstacktop,
    # the kernel stack size is KSTACKSIZE (8KB)defined in memlayout.h
    movl $bootstacktop, %esp
    # now kernel stack is ready , call the first C function
    call kern_init	# 现在开始调用kern_init函数

# should never get here
spin:
    jmp spin

.data
.align PGSIZE
    .globl bootstack
bootstack:
    .space KSTACKSIZE # 一共只有8kb的大小
    .globl bootstacktop
bootstacktop:

.align 4
__gdt:
    SEG_NULL
	# 代码段的基地址是-0xc00000000,为什么是这个奇葩的数值？
	# 这里有一个很深的原由，我这里来说一下，首先，kernel是被bootloader加载至0x100000这个物理地址
	# 然而，kernel在链接的时候，指定其虚拟地址从0xC0100000开始,我们看到了端倪，为了使虚拟地址映射到正确的物理地址，我们要减去一个0xc0000000
    SEG_ASM(STA_X | STA_R, - KERNBASE, 0xFFFFFFFF)      # code segment
    SEG_ASM(STA_W, - KERNBASE, 0xFFFFFFFF)              # data segment
__gdtdesc:
    .word 0x17                                          # sizeof(__gdt) - 1
    .long REALLOC(__gdt)

