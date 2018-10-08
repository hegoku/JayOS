
#ifndef _ASM_GLOBAL_H_
#define _ASM_GLOBAL_H_

#include "kernel.h"

extern int disp_pos;

extern char gdt_ptr[6];
extern DESCRIPTOR gdt[GDT_SIZE];

extern char idt_ptr[6];
extern GATE idt[IDT_SIZE];

void DispStr(char* msg);
void MemCpy(void *pDest, void *pSrc, int iSize);
void DispColorStr(char * info, int color);

/* 中断处理函数 */
void divide_error();
void single_step_exception();
void nmi();
void breakpoint_exception();
void overflow();
void bounds_check();
void inval_opcode();
void copr_not_available();
void double_fault();
void copr_seg_overrun();
void inval_tss();
void segment_not_present();
void stack_exception();
void general_protection();
void page_fault();
void copr_error();

#endif