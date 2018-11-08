#include "system/desc.h"
#include "process.h"
#include "kernel.h"

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
    p->regs.eflags = 0x1202;
}