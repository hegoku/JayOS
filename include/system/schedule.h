#ifndef __SYSTEM_SCHEDULE_H
#define __SYSTEM_SCHEDULE_H

#include <system/time.h>

#define switch_to(prev, next, last) \
    { \
        __asm__ volatile("pushfl\n\t" \
        "pushl %%eax\n\t" \
        "pushl %%ecx\n\t" \
        "pushl %%edx\n\t" \
        "pushl %%ebx\n\t" \
        "pushl %%ebp\n\t" \
        "pushl %%esi\n\t" \
        "pushl %%edi\n\t" \
        "movl %%esp, %[prev_sp]\n\t" \
        "movl %[next_sp], %%esp\n\t" \
        "movl $1f, %[prev_ip]\n\t" \
        "jmp %[next_ip]\n\t" \
        "1:\t" \
        "popl %%edi\n\t" \
        "popl %%esi\n\t" \
        "popl %%ebp\n\t" \
        "popl %%ebx\n\t" \
        "popl %%edx\n\t" \
        "popl %%ecx\n\t" \
        "popl %%eax\n\t" \
        "popfl\n\t" \
        : [prev_sp] "=m" ((prev)->kernel_regs.esp), \
          [prev_ip] "=m" ((prev)->kernel_regs.eip) \
        : [next_sp] "m" ((next)->kernel_regs.esp), \
          [next_ip] "m" ((next)->kernel_regs.eip) \
        : "memory" \
        ); \
    }

extern unsigned int ticks;

#define pass_seconds() (ticks/HZ) //过了多少秒
// #define	MAX_TICKS	0x7FFFABCD
#define	MAX_TICKS	0xFFFFFFFF

void timer_init();
void schedule();
int sys_alarm(unsigned int seconds);
int sys_pause();

#endif