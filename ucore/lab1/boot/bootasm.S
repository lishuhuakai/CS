#include <asm.h>

# Start the CPU: switch to 32-bit protected mode, jump into C.
# The BIOS loads this code from the first sector of the hard disk into
# memory at physical address 0x7c00 and starts executing in real mode
# with %cs=0 %ip=7c00.
# 启动cpu，切换到32位保护模式，跳转到c代码
# BIOS从硬盘的第一个扇区加载这个代码到
# 物理地址为0x7c00的地方，cs = 0, ip = 7c00

# 下面的3条.set指令类似于宏定义
# 内核代码段选择子
.set PROT_MODE_CSEG,        0x8                     # kernel code segment selector
# 内核数据段选择子
.set PROT_MODE_DSEG,        0x10                    # kernel data segment selector
# 保护模式使能模式
.set CR0_PE_ON,             0x1                     # protected mode enable flag

# start address should be 0:7c00, in real mode, the beginning address of the running bootloader
# 定义一个全局的名字start
.globl start
# 好吧，这里是16位的代码
start:
.code16                                             # Assemble for 16-bit mode
# 关中断
    cli                                             # Disable interrupts
# 清方向标志
	cld                                             # String operations increment

    # Set up the important data segment registers (DS, ES, SS).
	# 下面的代码主要用于设置段寄存器
	# 总之从效果上来看，其实都是清零啦！
    xorw %ax, %ax                                   # Segment number zero
    movw %ax, %ds                                   # -> Data Segment
    movw %ax, %es                                   # -> Extra Segment
    movw %ax, %ss                                   # -> Stack Segment

    # Enable A20:
    #  For backwards compatibility with the earliest PCs, physical
    #  address line 20 is tied low, so that addresses higher than
    #  1MB wrap around to zero by default. This code undoes this.
	# 打开a20的地址线
	# 为了兼容早期的PC机，第20根地址线在实模式下不能使用
	# 所以超过1MB的地址，默认就会返回到地址0，重新从0循环计数
	# 下面的代码打开A20地址线
seta20.1:
	# inb指令，从端口读取一个字节，也就是说上面的代码的意思是从0x64端口读取一个字节到al寄存器里
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
	# testb指令可以当做and指令，只不过它会影响操作数
    testb $0x2, %al
	# 如果发现al的第二位为0，就继续执行
    jnz seta20.1 # 如果不等于0的话，跳到seta20.1去执行

	# 将0xd1写入到al中
    movb $0xd1, %al                                 # 0xd1 -> port 0x64
	# 也写入到0x64端口
    outb %al, $0x64                                 # 0xd1 means: write data to 8042's P2 port

seta20.2:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
	# 好吧，其实就是一直查询，一直到0x64位置的第二位为0为止
    jnz seta20.2 # 其实我不知道这一段到底是什么意思，应该是不断轮询

	# 将0xdf写入0x60对应的端口之中
    movb $0xdf, %al                                 # 0xdf -> port 0x60
    outb %al, $0x60                                 # 0xdf = 11011111, means set P2's A20 bit(the 1 bit) to 1

    # Switch from real to protected mode, using a bootstrap GDT
    # and segment translation that makes virtual addresses
    # identical to physical addresses, so that the
    # effective memory map does not change during the switch.
	# 从实模式转换到保护模式
	
	# 将全局描述符表描述符加载到全局描述符表寄存器
    lgdt gdtdesc	# load gdt是吧？
	
	# cr0的第0位为1，表示处于保护模式
	# cr0的第0为为0，表示处于实模式
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax
	# 好吧，下面的汇编就开启了实模式
    movl %eax, %cr0

    # Jump to next instruction, but in 32-bit code segment.
    # Switches processor into 32-bit mode.
	# 跳转到32位模式中的下一条指令
	# 将处理器切换为32位工作模式
	# 下面的这条代码会将$PROT_MODE_CSEG加载到cs中，cs对应的告诉缓冲存储器会加载代码段描述符
	# 同样将$protecseg加载到ip中
    ljmp $PROT_MODE_CSEG, $protcseg

.code32                                             # Assemble for 32-bit mode
protcseg:
    # Set up the protected-mode data segment registers
	# 设置保护模式下的数据寄存器
	# 将数据选择子装入到ax中
    movw $PROT_MODE_DSEG, %ax                       # Our data segment selector 数据段的选择子是吧！
    # 将ax装入到其他数据段寄存器中，在装入的同时，
	# 数据段描述符会自动的加入到这些段寄存器对应的高速缓冲寄存器中
	movw %ax, %ds                                   # -> DS: Data Segment
    movw %ax, %es                                   # -> ES: Extra Segment
    movw %ax, %fs                                   # -> FS
    movw %ax, %gs                                   # -> GS
    movw %ax, %ss                                   # -> SS: Stack Segment

    # Set up the stack pointer and call into C. The stack region is from 0--start(0x7c00)
	# 准备好stack pointer，然后跳入c代码中，栈的区域是0--0x7c00
    movl $0x0, %ebp
    movl $start, %esp
    call bootmain # 好吧，现在就跳到c函数里去执行了。

    # If bootmain returns (it shouldn't), loop.
spin:
    jmp spin

# Bootstrap GDT
.p2align 2                                          # force 4 byte alignment
# 好吧，我们来看一下全局的gdt吧！global describle table
gdt:
    SEG_NULLASM                                     # null seg
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # code seg for bootloader and kernel
    SEG_ASM(STA_W, 0x0, 0xffffffff)                 # data seg for bootloader and kernel

gdtdesc:
    .word 0x17                                      # sizeof(gdt) - 1
    .long gdt                                       # address gdt
