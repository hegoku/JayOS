#include <sys/types.h>
#include "system/desc.h"
#include "process.h"
#include "kernel.h"
#include "global.h"

PROCESS create_process(DESCRIPTOR *gdt, PROCESS *p, unsigned int process_entry)
{

    DESCRIPTOR ldt=create_descriptor((unsigned int)p->ldts, 2*sizeof(DESCRIPTOR)-1, DA_LDT);
	p->ldt_sel=insert_descriptor(gdt, 7+p->pid, ldt, PRIVILEGE_KRNL);
    p->ldts[0] = create_descriptor(0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3);
    p->ldts[1] = create_descriptor(0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3);

    p->regs.cs = ((0 * 0x8)&SA_RPL_MASK&SA_TI_MASK) | SA_TIL | SA_RPL3;
    p->regs.ds = ((1 * 0x8)&SA_RPL_MASK&SA_TI_MASK) | SA_TIL | SA_RPL3;
    p->regs.es = ((1 * 0x8)&SA_RPL_MASK&SA_TI_MASK) | SA_TIL | SA_RPL3;
    p->regs.fs = ((1 * 0x8)&SA_RPL_MASK&SA_TI_MASK) | SA_TIL | SA_RPL3;
    p->regs.ss = ((1 * 0x8)&SA_RPL_MASK&SA_TI_MASK) | SA_TIL | SA_RPL3;
    p->regs.gs = (SelectorVideo&SA_RPL_MASK)| SA_RPL3;
    p->regs.eip = process_entry;
    p->regs.esp = TOP_OF_USER_STACK;;
    p->regs.eflags = 0x3202;

    p->status = 0;
}

int sys_fork()
{
    int pid = -1;
    int i;
    for (i = 0; i < PROC_NUMBER; i++) {
        if (current_process->file_table[i]==0) {
            pid = i;
            break;
        }
    }

    if (pid<0 || pid>=PROC_NUMBER) {
        printk("Process table is full\n");
        return -1;
    }
    
    process_table[pid].pid = pid;
    process_table[pid].p_name[0] = 'p';
    process_table[pid].p_name[1]='\0';
    // create_process(gdt, &process_table[pid], (unsigned int)task_tty);
    process_table[pid].regs.esp = TOP_OF_USER_STACK-0x400*pid;

    return pid;
}