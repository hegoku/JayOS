#ifndef _PROCESS_H
#define _PROCESS_H

typedef struct s_stackframe {
    unsigned int gs;
    unsigned int fs;
    unsigned int es;
    unsigned int ds;
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int kernel_esp; //popad will ignore it
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
    unsigned int retaddr; //return addr from kernel.asm::save()
    unsigned int eip;
    unsigned int cs;
    unsigned int eflags;
    unsigned int esp;
    unsigned int ss;
} STACK_FRAME;

typedef struct s_proc {
    STACK_FRAME regs;
    unsigned short ldt_sel;
    DESCRIPTOR ldts[2];
    unsigned int pid;
    char p_name[16];
} PROCESS;

PROCESS create_process(DESCRIPTOR *gdt, PROCESS *p, unsigned int process_entry);

#endif