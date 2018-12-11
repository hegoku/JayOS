#include "interrupt.h"
#include "global.h"

irq_handler irq_table[IRQ_NUMBER] = {
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq,
    spurious_irq
};

void init_8259A()
{
    out_byte(INT_M_CTL,	0x11);			// Master 8259, ICW1.
	out_byte(INT_S_CTL,	0x11);			// Slave  8259, ICW1.
	out_byte(INT_M_CTLMASK,	INT_VECTOR_IRQ0);	// Master 8259, ICW2. 设置 '主8259' 的中断入口地址为 0x20.
	out_byte(INT_S_CTLMASK,	INT_VECTOR_IRQ8);	// Slave  8259, ICW2. 设置 '从8259' 的中断入口地址为 0x28
	out_byte(INT_M_CTLMASK,	0x4);			// Master 8259, ICW3. IR2 对应 '从8259'.
	out_byte(INT_S_CTLMASK,	0x2);			// Slave  8259, ICW3. 对应 '主8259' 的 IR2.
	out_byte(INT_M_CTLMASK,	0x1);			// Master 8259, ICW4.
	out_byte(INT_S_CTLMASK,	0x1);			// Slave  8259, ICW4.

	// out_byte(INT_M_CTLMASK,	0xBD);	// Master 8259, OCW1.  开键盘和软盘
	out_byte(INT_M_CTLMASK,	0xFF);	// Master 8259, OCW1.  开键盘和软盘
	out_byte(INT_S_CTLMASK,	0xFF);	// Slave  8259, OCW1.

    // enable_irq(CLOCK_IRQ);
    // enable_irq(KEYBOARD_IRQ);
    // enable_irq(FLOPPY_IRQ);
    // enable_irq(CASCADE_IRQ);
    // enable_irq(AT_WINI_IRQ);

}

/*----------------------------------------------------------------------*
 初始化 386 中断门
 *======================================================================*/
void init_idt_desc(GATE* idt, unsigned char vector, unsigned char desc_type, void* handler, unsigned char privilege)
{
	GATE *	p_gate	= &idt[vector];
	unsigned int	base	= (unsigned int)handler;
	p_gate->offset_low	= base & 0xFFFF;
	p_gate->selector	= 0x8;
	p_gate->dcount		= 0;
	p_gate->attr		= desc_type | (privilege << 5);
	p_gate->offset_high	= (base >> 16) & 0xFFFF;
}

/*======================================================================*
                            exception_handler
 *----------------------------------------------------------------------*
 异常处理
 *======================================================================*/
void exception_handler(int vec_no,int err_code,int eip,int cs,int eflags)
{
	int i;
	int text_color = 0x74; /* 灰底红字 */

	char * err_msg[] = {"#DE Divide Error",
			    "#DB RESERVED",
			    "--  NMI Interrupt",
			    "#BP Breakpoint",
			    "#OF Overflow",
			    "#BR BOUND Range Exceeded",
			    "#UD Invalid Opcode (Undefined Opcode)",
			    "#NM Device Not Available (No Math Coprocessor)",
			    "#DF Double Fault",
			    "    Coprocessor Segment Overrun (reserved)",
			    "#TS Invalid TSS",
			    "#NP Segment Not Present",
			    "#SS Stack-Segment Fault",
			    "#GP General Protection",
			    "#PF Page Fault",
			    "--  (Intel reserved. Do not use.)",
			    "#MF x87 FPU Floating-Point Error (Math Fault)",
			    "#AC Alignment Check",
			    "#MC Machine Check",
			    "#XF SIMD Floating-Point Exception"
	};

	/* 通过打印空格的方式清空屏幕的前五行，并把 disp_pos 清零 */
	// disp_pos = 0;
	// for(i=0;i<80*5;i++){
	// 	disp_str(" ");
	// }
	// disp_pos = 0;

	DispColorStr("Exception! --> ", text_color);
	DispColorStr(err_msg[vec_no], text_color);
	DispColorStr("\n\n", text_color);
	DispColorStr("EFLAGS:", text_color);
	disp_int(eflags);
	DispColorStr("CS:", text_color);
	disp_int(cs);
	DispColorStr("EIP:", text_color);
	disp_int(eip);

	if(err_code != 0xFFFFFFFF){
		DispColorStr("Error code:", text_color);
		disp_int(err_code);
	}
}

void spurious_irq(int irq)
{
	DispStr("spurious_irq: ");
	disp_int(irq);
	DispStr("\n");
}