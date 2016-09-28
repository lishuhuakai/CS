#include <defs.h>
#include <mmu.h>
#include <memlayout.h>
#include <clock.h>
#include <trap.h>
#include <x86.h>
#include <stdio.h>
#include <assert.h>
#include <console.h>
#include <kdebug.h>

#define TICK_NUM 100
// 也就是说时钟每秒钟滴答100次是吧！

static void print_ticks() {
    cprintf("%d ticks\n",TICK_NUM);
#ifdef DEBUG_GRADE
    cprintf("End of Test.\n");
    panic("EOT: kernel seems ok.");
#endif
}

/* *
 * Interrupt descriptor table:
 *
 * Must be built at run time because shifted function addresses can't
 * be represented in relocation records.
 * */
 // 这里是中断描述符表
static struct gatedesc idt[256] = {{0}};

// 好吧，我貌似有了一点记忆，第一个字节是idt的大小，然后是32位的基地址，是吧！
static struct pseudodesc idt_pd = {
    sizeof(idt) - 1, (uintptr_t)idt
};

/* idt_init - initialize IDT to each of the entry points in kern/trap/vectors.S */
// idt是interrupt description table的缩写吧。
void
idt_init(void) {
     /* LAB1 YOUR CODE : STEP 2 */
     /* (1) Where are the entry addrs of each Interrupt Service Routine (ISR)? 中断处理程序的地址
      *     All ISR's entry addrs are stored in __vectors. where is uintptr_t __vectors[] ?
      *     __vectors[] is in kern/trap/vector.S which is produced by tools/vector.c
      *     (try "make" command in lab1, then you will find vector.S in kern/trap DIR)
      *     You can use  "extern uintptr_t __vectors[];" to define this extern variable which will be used later.
      * (2) Now you should setup the entries of ISR in Interrupt Description Table (IDT).
      *     Can you see idt[256] in this file? Yes, it's IDT! you can use SETGATE macro to setup each item of IDT
      * (3) After setup the contents of IDT, you will let CPU know where is the IDT by using 'lidt' instruction.
      *     You don't know the meaning of this instruction? just google it! and check the libs/x86.h to know more.
      *     Notice: the argument of lidt is idt_pd. try to find it!
      */
	extern uintptr_t __vectors[];
	int i;
	for (i = 0; i < sizeof(idt) / sizeof(struct gatedesc); i++) {
		// DPL代表的是特权级
		// idt[i] 是地址,而__vectors[i]是对应处理函数的地址
		SETGATE(idt[i], 0, GD_KTEXT, __vectors[i], DPL_KERNEL); // 这里是真的要设置255个中断项啊！
	}
	// 设置转移的入口？
	SETGATE(idt[T_SWITCH_TOK], 0, GD_KTEXT, __vectors[T_SWITCH_TOK], DPL_USER);
	// 加载idt
	lidt(&idt_pd);
}

// 下面的这个函数用于获取trap的名称
static const char *
trapname(int trapno) {
    static const char * const excnames[] = {
        "Divide error",
        "Debug",
        "Non-Maskable Interrupt",
        "Breakpoint",
        "Overflow",
        "BOUND Range Exceeded",
        "Invalid Opcode",
        "Device Not Available",
        "Double Fault",
        "Coprocessor Segment Overrun",
        "Invalid TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection",
        "Page Fault",
        "(unknown trap)",
        "x87 FPU Floating-Point Error",
        "Alignment Check",
        "Machine-Check",
        "SIMD Floating-Point Exception"
    };

    if (trapno < sizeof(excnames)/sizeof(const char * const)) {
        return excnames[trapno];
    }
    if (trapno >= IRQ_OFFSET && trapno < IRQ_OFFSET + 16) {
        return "Hardware Interrupt";
    }
    return "(unknown trap)";
}

/* trap_in_kernel - test if trap happened in kernel */
// trapframe是个什么玩意
bool
trap_in_kernel(struct trapframe *tf) {
    return (tf->tf_cs == (uint16_t)KERNEL_CS);
}

static const char *IA32flags[] = {
    "CF", NULL, "PF", NULL, "AF", NULL, "ZF", "SF",
    "TF", "IF", "DF", "OF", NULL, NULL, "NT", NULL,
    "RF", "VM", "AC", "VIF", "VIP", "ID", NULL, NULL,
};

void
print_trapframe(struct trapframe *tf) {
    cprintf("trapframe at %p\n", tf);
    print_regs(&tf->tf_regs);
    cprintf("  ds   0x----%04x\n", tf->tf_ds);
    cprintf("  es   0x----%04x\n", tf->tf_es);
    cprintf("  fs   0x----%04x\n", tf->tf_fs);
    cprintf("  gs   0x----%04x\n", tf->tf_gs);
    cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
    cprintf("  err  0x%08x\n", tf->tf_err);
    cprintf("  eip  0x%08x\n", tf->tf_eip);
    cprintf("  cs   0x----%04x\n", tf->tf_cs);
    cprintf("  flag 0x%08x ", tf->tf_eflags);

    int i, j;
    for (i = 0, j = 1; i < sizeof(IA32flags) / sizeof(IA32flags[0]); i ++, j <<= 1) {
        if ((tf->tf_eflags & j) && IA32flags[i] != NULL) {
            cprintf("%s,", IA32flags[i]);
        }
    }
    cprintf("IOPL=%d\n", (tf->tf_eflags & FL_IOPL_MASK) >> 12);

    if (!trap_in_kernel(tf)) {
        cprintf("  esp  0x%08x\n", tf->tf_esp);
        cprintf("  ss   0x----%04x\n", tf->tf_ss);
    }
}

void
print_regs(struct pushregs *regs) {
    cprintf("  edi  0x%08x\n", regs->reg_edi);
    cprintf("  esi  0x%08x\n", regs->reg_esi);
    cprintf("  ebp  0x%08x\n", regs->reg_ebp);
    cprintf("  oesp 0x%08x\n", regs->reg_oesp);
    cprintf("  ebx  0x%08x\n", regs->reg_ebx);
    cprintf("  edx  0x%08x\n", regs->reg_edx);
    cprintf("  ecx  0x%08x\n", regs->reg_ecx);
    cprintf("  eax  0x%08x\n", regs->reg_eax);
}

/* trap_dispatch - dispatch based on what type of trap occurred */
// 我们继续来看trap_dispatch函数吧！

struct trapframe switchk2u, *switchu2k;
static void
trap_dispatch(struct trapframe *tf) { 
    char c;
	// 基本的思想是根据中断号，来调用对应的函数
    switch (tf->tf_trapno) {
    case IRQ_OFFSET + IRQ_TIMER:
        /* LAB1 YOUR CODE : STEP 3 */
        /* handle the timer interrupt */
        /* (1) After a timer interrupt, you should record this event using a global variable (increase it), such as ticks in kern/driver/clock.c
         * (2) Every TICK_NUM cycle, you can print some info using a funciton, such as print_ticks().
         * (3) Too Simple? Yes, I think so!
         */
		ticks++;
		if (ticks % TICK_NUM == 0) {
			print_ticks();
		}
        break;
    case IRQ_OFFSET + IRQ_COM1:
        c = cons_getc();
        cprintf("serial [%03d] %c\n", c, c);
        break;
    case IRQ_OFFSET + IRQ_KBD:
        c = cons_getc();
        cprintf("kbd [%03d] %c\n", c, c);
        break;
    //LAB1 CHALLENGE 1 : YOUR CODE you should modify below codes.
    case T_SWITCH_TOU:
		if (tf->tf_cs != USER_CS) {
			// 如果不是处于用户态
			switchk2u = *tf;
			switchk2u.tf_cs = USER_CS;
			switchk2u.tf_ds = switchk2u.tf_es = switchk2u.tf_ss = U SER_DS;
			switchk2u.tf_esp = (uint32_t)tf + sizeof(struct trapframe) - 8;
			// esp是一个偏移量
			
			// 设置eflags标志
			switchk2u.tf_eflags |= FL_IOPL_MASK;
			// set temporary stackavail
			// then iret will jump to the right stackavail
			// 技巧性非常高，好不好。
			// tf是一个指针，
			*((uint32_t *)tf - 1) = (uint32_t)&switch2u;
		}
    case T_SWITCH_TOK:
		if (tf->tf_cs != KERNEL_CS) {
			// 如果不是处于kernel状态
			tf->tf_cs = KERNEL_CS;
			tf->tf_ds = tf->tf_es = KERNEL_DS;
			tf->tf_eflags &= ~FL_IOPL_MASK; // 去除这个标志
			switchu2k = (struct trapframe *)(tf->tf_esp - (sizeof(struct trapframe) - 8));
			*((uint32_t *)tf - 1) = (uint32_t)switchu2k;
		}
		//cprintf("Ok, now we detected that a interrupt occured!\n");
        //panic("T_SWITCH_** ??\n"); // 好吧，一旦在处理中断的时候，cpu会忽略其他的中断
        break;
    case IRQ_OFFSET + IRQ_IDE1:
    case IRQ_OFFSET + IRQ_IDE2:
        /* do nothing */
        break;
    default:
        // in kernel, it must be a mistake
        if ((tf->tf_cs & 3) == 0) {
            print_trapframe(tf);
            panic("unexpected trap in kernel.\n");
        }
    }
}

/* *
 * trap - handles or dispatches an exception/interrupt. if and when trap() returns,
 * the code in kern/trap/trapentry.S restores the old CPU state saved in the
 * trapframe and then uses the iret instruction to return from the exception.
 * */
 // 好吧，其实挺有意思的，我们来仔细分析一下吧！
 // 我们首先来看一下栈里面有什么东西，好吧！
 // 首先是trapframe的定义
 /******************************************************
 struct pushregs { // 这些都是通过pushl压入的通用寄存器的值
    uint32_t reg_edi; 
    uint32_t reg_esi;
    uint32_t reg_ebp;
    uint32_t reg_oesp;       
    uint32_t reg_ebx;
    uint32_t reg_edx;
    uint32_t reg_ecx;
    uint32_t reg_eax;
};

 struct trapframe {
    struct pushregs tf_regs;

    uint16_t tf_gs;
    uint16_t tf_padding0;
    uint16_t tf_fs;
    uint16_t tf_padding1;
    uint16_t tf_es;
    uint16_t tf_padding2;
    uint16_t tf_ds;
    uint16_t tf_padding3;
    uint32_t tf_trapno; # 中断号
   
    uint32_t tf_err;  # 错误号
	# 下面这些寄存器的值是硬件自动压入的，中断执行返回后，会自动恢复
    uintptr_t tf_eip; #  
    uint16_t tf_cs; 
    uint16_t tf_padding4;
    uint32_t tf_eflags; 
    # 下面的值是我们自己定义的
    uintptr_t tf_esp; 
    uint16_t tf_ss; 
    uint16_t tf_padding5;
};
**********************************************************/
// 我们回过头来从vectors.S和trapentry.S两个文件的压栈顺序来看一下压入的参数
// 我感到有一些疑惑的是，esp到哪里去了？esp不是作为一个参数压栈了吗？
// 我懂了，我们看一下参数，参数要求是一个指针，esp确实是参数，esp = *tf
// 我们知道，压入esp的时候，esp恰好指向这个参数的开始位置，好厉害。
void
trap(struct trapframe *tf) {
    // dispatch based on what type of trap occurred
    trap_dispatch(tf);
}

