#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "system/desc.h"
#include "interrupt.h"
#include "tty.h"
#include "process.h"
// #define GDT_SIZE 128
// #define IDT_SIZE 256

#define TOP_OF_USER_STACK 0x6C00
#define TOP_OF_KERNEL_STACK 0x7C00

#define PROC_NUMBER 3

extern char gdt_ptr[6];
extern DESCRIPTOR gdt[GDT_SIZE];

extern char idt_ptr[6];
extern GATE idt[IDT_SIZE];
extern irq_handler irq_table[];
extern sys_call_handler sys_call_table[];

extern unsigned short SelectorKernelCs;
extern unsigned short SelectorKernelDs;
extern unsigned short SelectorVideo;
extern unsigned short SelectorUserCs;
extern unsigned short SelectorUserDs;
extern unsigned short SelectorTss;

int ticks;
TTY tty;

PROCESS process_table[PROC_NUMBER];
PROCESS *p_proc_ready;

extern int sys_call_0_param(int index);
extern int sys_call_1_param(int index, ...);
extern int sys_call_2_param(int index, ...);
extern int sys_call_3_param(int index, ...);

#endif