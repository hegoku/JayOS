#ifndef _PROCESS_H
#define _PROCESS_H

#include <system/compiler_types.h>
#include <system/desc.h>
#include <system/fs.h>

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
    unsigned int parent_pid;
    char name[256];
    unsigned char status;
    unsigned char is_free;
    struct file_descriptor *file_table[PROC_FILES_MAX_COUNT];
    struct dir_entry *root;
    struct dir_entry *pwd;
    unsigned long base_addr;
} PROCESS;

PROCESS create_process(DESCRIPTOR *gdt, PROCESS *p, unsigned int process_entry);
void sys_exit(int status);
pid_t sys_fork();
pid_t sys_wait(int *status);
pid_t sys_getpid();
pid_t sys_getppid();
int sys_execve(const char __user *filename, const char __user *argv[], const char __user *envp[]);

int copy_from_user(void *to, const void __user *from, unsigned long n);
int copy_to_user(void __user *to, const void *from, unsigned long n);
int strncpy_from_user(void *to, const void __user *from);

#endif