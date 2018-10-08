#include "interrupt.h"
#include "asm_global.h"

void disp_int(int input);
char *itoa(char *str, int num);

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

char * itoa(char * str, int num)
{
	char *	p = str;
	char	ch;
	int	i;
	int	flag = 0;

	*p++ = '0';
	*p++ = 'x';

	if(num == 0){
		*p++ = '0';
	}
	else{	
		for(i=28;i>=0;i-=4){
			ch = (num >> i) & 0xF;
			if(flag || (ch > 0)){
				flag = 1;
				ch += '0';
				if(ch > '9'){
					ch += 7;
				}
				*p++ = ch;
			}
		}
	}

	*p = 0;

	return str;
}

void disp_int(int input)
{
	char output[16];
	itoa(output, input);
	DispStr(output);
}