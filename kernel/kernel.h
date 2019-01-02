#ifndef _KERNEL_H_
#define _KERNEL_H_

#include "system/desc.h"
#include "interrupt.h"
#include <system/system_call.h>
#include "tty.h"
#include <system/process.h>
// #define GDT_SIZE 128
// #define IDT_SIZE 256

// #define TOP_OF_USER_STACK 0x6C00
// #define TOP_OF_KERNEL_STACK 0x7C00
// #define TOP_OF_KERNEL_STACK 0x7C00
#define BIOS_ADDR 0x7E00 //bios参数地址

#define GDT_SEL_KERNEL_CODE (0x8|SA_RPL0) //因为loader的 GDT_SEL_CODE 选择子为 8
#define GDT_SEL_KERNEL_DATA (0x10|SA_RPL0)
#define GDT_SEL_VIDEO (0x18|SA_RPL3)
#define GDT_SEL_USER_CODE (0x20|SA_RPL3)
#define GDT_SEL_USER_DATA (0x28|SA_RPL3)
#define GDT_SEL_TSS (0x30|SA_RPL0)

#define PROC_NUMBER 5

extern char gdt_ptr[6];
extern DESCRIPTOR gdt[GDT_SIZE];

extern char idt_ptr[6];
extern GATE idt[IDT_SIZE];
extern irq_handler irq_table[];
extern sys_call_handler sys_call_table[];

extern PROCESS *current_process;
extern PROCESS process_table[];

extern int is_in_ring0;

#endif