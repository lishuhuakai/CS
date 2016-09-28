#include <asm.h>

# Start the CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.

.set PROT_MODE_CSEG,        0x8                     # kernel code segment selector
.set PROT_MODE_DSEG,        0x10                    # kernel data segment selector
.set CR0_PE_ON,             0x1                     # protected mode enable flag
.set SMAP,                  0x534d4150

# start address should be 0:7c00, in real mode, the beginning address of the running bootloader
.globl start
start:
.code16                                             # Assemble for 16-bit mode
    cli                                             # Disable interrupts 关中断
    cld                                             # String operations increment

    # Set up the important data segment registers (DS, ES, SS).
    xorw %ax, %ax                                   # Segment number zero
    movw %ax, %ds                                   # -> Data Segment
    movw %ax, %es                                   # -> Extra Segment 好吧，es也是0
    movw %ax, %ss                                   # -> Stack Segment

    # Enable A20:
    #  For backwards compatibility with the earliest PCs, physical
    #  address line 20 is tied low, so that addresses higher than
    #  1MB wrap around to zero by default. This code undoes this.
seta20.1:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.1

    movb $0xd1, %al                                 # 0xd1 -> port 0x64
    outb %al, $0x64                                 # 0xd1 means: write data to 8042's P2 port

seta20.2:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.2

    movb $0xdf, %al                                 # 0xdf -> port 0x60
    outb %al, $0x60                                 # 0xdf = 11011111, means set P2's A20 bit(the 1 bit) to 1

probe_memory:
	# 这里的意思是，将0写到0x8000对应的位置
    movl $0, 0x8000									# 难怪一直没有找到方法，原来在这里就已经在进行探测了。好，很好！
    xorl %ebx, %ebx									# 第一次调用的时候bx要赋值为0
    movw $0x8004, %di								# es = 0,然后di = 0x8004,我好奇的是，前面的4个字节用来干什么？
start_probe:										# 现在开始探测了
    movl $0xE820, %eax								# 首先将0xE820放入eax寄存器
    movl $20, %ecx									# 这个玩意是默认的20字节的大小
    movl $SMAP, %edx								# 好吧，压入0534D415oh，也即是smap
    int $0x15										# 调用中断来探测可用的内存地址
    jnc cont
    movw $12345, 0x8000
    jmp finish_probe
cont:
    addw $20, %di									# 往后挪动20个字节
    incl 0x8000										# 数目加1
    cmpl $0, %ebx
    jnz start_probe									# 如果ebx不为0，那么说明还可以继续探测
finish_probe:

    # Switch from real to protected mode, using a bootstrap GDT
    # and segment translation that makes virtual addresses
    # identical to physical addresses, so that the
    # effective memory map does not change during the switch.
	# 主要是讲
	# 好吧，我大概看懂了这大概是什么意思了，下面是开
    lgdt gdtdesc
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax # 开启保护模式
    movl %eax, %cr0

    # Jump to next instruction, but in 32-bit code segment.
    # Switches processor into 32-bit mode.
    ljmp $PROT_MODE_CSEG, $protcseg			# PROT_MODE_CSEG指的是代码段的选择子

.code32                                             # Assemble for 32-bit mode
protcseg:
    # Set up the protected-mode data segment registers
    movw $PROT_MODE_DSEG, %ax                       # Our data segment selector
    movw %ax, %ds                                   # -> DS: Data Segment
    movw %ax, %es                                   # -> ES: Extra Segment
    movw %ax, %fs                                   # -> FS
    movw %ax, %gs                                   # -> GS
    movw %ax, %ss                                   # -> SS: Stack Segment

    # Set up the stack pointer and call into C. The stack region is from 0--start(0x7c00)
    movl $0x0, %ebp
    movl $start, %esp # 好吧，栈顶是0x7c00
    call bootmain

    # If bootmain returns (it shouldn't), loop.
spin:
    jmp spin

.data					# 这里是数据区
# Bootstrap GDT
.p2align 2                                          # force 4 byte alignment
gdt:
    SEG_NULLASM                                     # null seg
	# 好吧，基址是0，然后界限是0xffffffff
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # code seg for bootloader and kernel
	# 数据段的基址是0，然后界限是0xffffffff
    SEG_ASM(STA_W, 0x0, 0xffffffff)                 # data seg for bootloader and kernel

gdtdesc:
    .word 0x17                                      # sizeof(gdt) - 1
    .long gdt                                       # address gdt
