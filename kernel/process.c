#include <system/compiler_types.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <system/fs.h>
#include <system/desc.h>
#include "process.h"
#include "kernel.h"
#include "global.h"
#include <system/mm.h>
#include <system/elf.h>
#include <system/page.h>

#define INDEX_LDT_CS 0
#define INDEX_LDT_DS 1

#define PROC_IMAGE_SIZE_DEFAULT 128 * 1024 //一个进程占用128KB内存
#define PROCS_BASE 0x250000 //用户进程起始地址 1MB+128KB

#define	reassembly(high, high_shift, mid, mid_shift, low)	\
	(((high) << (high_shift)) +				\
	 ((mid)  << (mid_shift)) +				\
	 (low))

static int alloc_mem(int pid, int memsize);

PROCESS create_process(DESCRIPTOR *gdt, PROCESS *p, unsigned int process_entry)
{

    DESCRIPTOR ldt=create_descriptor((unsigned int)p->ldts, 2*sizeof(DESCRIPTOR)-1, DA_LDT);
	p->ldt_sel=insert_descriptor(gdt, 7+p->pid, ldt, PRIVILEGE_KRNL);
    // p->ldts[INDEX_LDT_CS] = create_descriptor(0, (PROCS_BASE-1)>>LIMIT_4K_SHIFT, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3);
    // p->ldts[INDEX_LDT_DS] = create_descriptor(0, (PROCS_BASE-1)>>LIMIT_4K_SHIFT, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3);
    p->ldts[INDEX_LDT_CS] = create_descriptor(0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3);
    p->ldts[INDEX_LDT_DS] = create_descriptor(0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3);

    p->regs.cs = ((INDEX_LDT_CS * 0x8)&SA_RPL_MASK&SA_TI_MASK) | SA_TIL | SA_RPL3;
    p->regs.ds = ((INDEX_LDT_DS * 0x8)&SA_RPL_MASK&SA_TI_MASK) | SA_TIL | SA_RPL3;
    p->regs.es = ((INDEX_LDT_DS * 0x8)&SA_RPL_MASK&SA_TI_MASK) | SA_TIL | SA_RPL3;
    p->regs.fs = ((INDEX_LDT_DS * 0x8)&SA_RPL_MASK&SA_TI_MASK) | SA_TIL | SA_RPL3;
    p->regs.ss = ((INDEX_LDT_DS * 0x8)&SA_RPL_MASK&SA_TI_MASK) | SA_TIL | SA_RPL3;
    p->regs.gs = (GDT_SEL_VIDEO & SA_RPL_MASK) | SA_RPL3;
    p->regs.eip = process_entry;
    // p->regs.esp = TOP_OF_USER_STACK-0x400*p->pid;
    p->regs.esp = PROC_IMAGE_SIZE_DEFAULT-1;
    p->regs.eflags = 0x3202;

    p->base_addr = 0;
    p->page_dir = NULL;
    // init_process_page(&(p->page_dir));

    p->status = 0;
    p->is_free = 0;
}

pid_t sys_fork()
{
    int pid = -1;
    int i;
    for (i = 0; i < PROC_NUMBER; i++) {
        if (process_table[i].is_free==0) {
            pid = i;
            break;
        }
    }

    if (pid<0 || pid>=PROC_NUMBER) {
        printk("Process table is full\n");
        return -1;
    }

    unsigned short my_idt_sel = process_table[pid].ldt_sel;
	struct PageDir *my_cr3 = process_table[pid].page_dir;
	process_table[pid] = process_table[current_process->pid];
	process_table[pid].ldt_sel = my_idt_sel;
	// process_table[pid].page_dir = my_cr3;
	process_table[pid].pid = pid;
	sprintf(process_table[pid].name, "%s_%d", process_table[current_process->pid].name, pid);
    process_table[pid].parent_pid = current_process->pid;
	process_table[pid].regs.eax = 0;
    if (my_cr3==NULL) {
        process_table[pid].page_dir = create_dir();
        printk("c:%x %x\n", process_table[pid].page_dir, process_table[pid].page_dir->entry);
    }
    else
    {
        process_table[pid].page_dir = my_cr3;
    }

    /* duplicate the process: T, D & S */
	// DESCRIPTOR *ppd;

	// /* Text segment */
	// ppd = &process_table[current_process->pid].ldts[INDEX_LDT_CS];
	// /* base of T-seg, in bytes */
	// int caller_T_base  = reassembly(ppd->base_high, 24,
	// 				ppd->base_mid,  16,
	// 				ppd->base_low);
	// /* limit of T-seg, in 1 or 4096 bytes,
	//    depending on the G bit of descriptor */
	// int caller_T_limit = reassembly(0, 0,
	// 				(ppd->limit_high_attr2 & 0xF), 16,
	// 				ppd->limit_low);
	// /* size of T-seg, in bytes */
	// int caller_T_size  = ((caller_T_limit + 1) *
	// 		      ((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ?
	// 		       4096 : 1));
    // // int caller_T_size  = caller_T_limit + 1;

	// /* Data & Stack segments */
	// ppd = &process_table[current_process->pid].ldts[INDEX_LDT_DS];
	// /* base of D&S-seg, in bytes */
	// int caller_D_S_base  = reassembly(ppd->base_high, 24,
	// 				  ppd->base_mid,  16,
	// 				  ppd->base_low);
	// /* limit of D&S-seg, in 1 or 4096 bytes,
	//    depending on the G bit of descriptor */
	// int caller_D_S_limit = reassembly((ppd->limit_high_attr2 & 0xF), 16,
	// 				  0, 0,
	// 				  ppd->limit_low);
	// /* size of D&S-seg, in bytes */
	// int caller_D_S_size  = ((caller_T_limit + 1) *
	// 			((ppd->limit_high_attr2 & (DA_LIMIT_4K >> 8)) ?
	// 			 4096 : 1));
    // int caller_D_S_size  = caller_T_limit + 1;

	/* we don't separate T, D & S segments, so we have: */

	/* base of child proc, T, D & S segments share the same space,
	   so we allocate memory just once */
	// process_table[pid].base_addr = alloc_mem(pid, caller_T_size);
    copy_page(current_process->page_dir, &(process_table[pid].page_dir));
    /* int child_limit = caller_T_limit; */
    // printk("{MM} 0x%x <- 0x%x (0x%x bytes) limit:%x\n",
	//        (void*)process_table[pid].base_addr, caller_T_base, caller_T_size, caller_T_limit);
	/* child is a copy of the parent */
    // memcpy((void*)process_table[pid].base_addr, (void*)caller_T_base, caller_T_size);
	/* child's LDT */
	// process_table[pid].ldts[INDEX_LDT_CS]=create_descriptor(process_table[pid].base_addr,
	// 	  (process_table[pid].base_addr + PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
	// 	//   (PROC_IMAGE_SIZE_DEFAULT - 1)>>LIMIT_4K_SHIFT,
	// 	  DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3);
	// process_table[pid].ldts[INDEX_LDT_DS]=create_descriptor(process_table[pid].base_addr,
	// 	  (process_table[pid].base_addr + PROC_IMAGE_SIZE_DEFAULT - 1) >> LIMIT_4K_SHIFT,
	// 	//   (PROC_IMAGE_SIZE_DEFAULT - 1)>>LIMIT_4K_SHIFT,
	// 	  DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3);

    //文件描述符
    for (i = 0; i < PROC_FILES_MAX_COUNT; i++)
    {
        if (process_table[pid].file_table[i]) {
            process_table[pid].file_table[i]->inode->used_count++;
            process_table[pid].file_table[i]->used_count++;
        }
    }

	return pid;
}

void sys_exit(int status)
{

}

pid_t sys_getpid()
{
    return current_process->pid;
}

pid_t sys_getppid()
{
    return current_process->parent_pid;
}

pid_t sys_wait(int *status)
{
    return 0;
}

int sys_execve(const char __user *filename, const char __user *argv[], const char __user *envp[])
{
    struct dir_entry *dir;
    char *kfilename=kzmalloc(256);
    int res;
    strncpy_from_user(kfilename, filename);

    if (namei(kfilename, &dir)) {
        printk("%s not exist\n", kfilename);
        return -1;
    }

    if (dir->inode->mode!=FILE_MODE_REG) {
        printk("%s not a regular file\n", kfilename);
        return -1;
    }

    char *program = kzmalloc(dir->inode->size);
    struct file_descriptor *file = kzmalloc(sizeof(struct file_descriptor));
    file->inode = dir->inode;
    file->op = dir->inode->f_op;
    file->pos = 0;
    if (dir->inode->f_op->read)
    {
        res=file->op->read(file, program, dir->inode->size);
    }
    kfree(file, sizeof(struct file_descriptor));
    if (res==0)
    {
        kfree(program, dir->inode->size);
        return -1;
    }

    struct elf32_hdr *elf_hdr = (struct elf32_hdr *)program;
    for (int i = 0; i < elf_hdr->e_phnum; i++) {
        struct elf32_phdr *phdr = (struct elf32_phdr *)(program + elf_hdr->e_phoff + (i * elf_hdr->e_phentsize));
        if (phdr->p_type==PT_LOAD) {
            printk("%x %x %d\n", phdr->p_vaddr, program+phdr->p_offset, phdr->p_filesz);
            // memcpy((void*)process_table[current_process->pid].base_addr+phdr->p_vaddr, (char*)program+phdr->p_offset, phdr->p_filesz);
            memcpy((void *)phdr->p_vaddr, (void *)(program + phdr->p_offset), phdr->p_filesz);
        } else if(phdr->p_type==PT_INTERP){
            
        }
    }
    printk("e:%x\n", elf_hdr->e_entry);
    process_table[current_process->pid].regs.eip = elf_hdr->e_entry-0x150000;
    process_table[current_process->pid].regs.esp = PROC_IMAGE_SIZE_DEFAULT - 1;
    strcpy(process_table[current_process->pid].name, kfilename);
    kfree(program, dir->inode->size);
    return 0;
}

static int alloc_mem(int pid, int memsize)
{
	// if (memsize > PROC_IMAGE_SIZE_DEFAULT) {
	// 	printk("unsupported memory request: %d. "
	// 	      "(should be less than %d)\n",
	// 	      memsize,
	// 	      PROC_IMAGE_SIZE_DEFAULT);
    //     return -1;
    // }

    int base = PROCS_BASE +
		pid * PROC_IMAGE_SIZE_DEFAULT;

	// if (base + memsize >= MemSize) {
    //     printk("memory allocation failed. pid:%d\n", pid);
    //     return -1;
    // }

    return base;
}

inline int copy_from_user(void *to, const void __user *from, unsigned long n)
{
    if (__builtin_constant_p(n)) {
        switch(n) {
        case 1:
            *(unsigned char*)to = *(unsigned char *)from;
            return 0;
        case 2:
            *(unsigned short*)to = *(unsigned short *)from;
            return 0;
        case 4:
            *(unsigned int*)to = *(unsigned int *)from;
            return 0;
        case 8:
            *(unsigned long*)to = *(unsigned long *)from;
            return 0;
        default:
            break;
        }
    }
    // unsigned int seg_base = current_process->ldts[INDEX_LDT_DS].base_high << 24 | current_process->ldts[INDEX_LDT_DS].base_mid << 16 | current_process->ldts[INDEX_LDT_DS].base_low;
    // printk("%x %x %x\n",seg_base, from, seg_base+from);
    // memcpy(to, (void*)(seg_base + (unsigned int)from), n);
    memcpy(to, from, n);
    return 0;
}

inline int copy_to_user(void __user *to, const void *from, unsigned long n)
{
    if (__builtin_constant_p(n)) {
        switch(n) {
        case 1:
            *(unsigned char*)to = *(unsigned char *)from;
            return 0;
        case 2:
            *(unsigned short*)to = *(unsigned short *)from;
            return 0;
        case 4:
            *(unsigned int*)to = *(unsigned int *)from;
            return 0;
        case 8:
            *(unsigned long*)to = *(unsigned long *)from;
            return 0;
        default:
            break;
        }
    }
    // unsigned int seg_base=current_process->ldts[INDEX_LDT_DS].base_high << 24 | current_process->ldts[INDEX_LDT_DS].base_mid << 16 | current_process->ldts[INDEX_LDT_DS].base_low;
    // memcpy((void*)(seg_base + (unsigned int)to), from, n);
    memcpy(to, from, n);
    return 0;
}

inline int strncpy_from_user(void *to, const void __user *from)
{
    // unsigned int seg_base = current_process->ldts[INDEX_LDT_DS].base_high << 24 | current_process->ldts[INDEX_LDT_DS].base_mid << 16 | current_process->ldts[INDEX_LDT_DS].base_low;
    // char *a = (char *)(seg_base + (unsigned int)from);
    char *a = (void*)from;
    char *p = to;
    while(*a!='\0') {
        *p = *a;
        a++;
        p++;
    }
    return 0;
}