#include "kernel.h"
#include "global.h"
#include <system/page.h>
#include <system/schedule.h>
#include <system/time.h>
#include <system/signal.h>
#include <sys/types.h>
#include <system/process.h>

unsigned int ticks = 1;

void timer_init()
{
    out_byte(0x43, 0x34);
    out_byte(0x40, (unsigned char)(TIMER_FREQ / HZ));
    out_byte(0x40, (unsigned char)((TIMER_FREQ / HZ)>>8));
}

void schedule()
{
    if (is_in_ring0 != 0)
    {
        // a[0]='1';
        // // i=sprintf(buf, "!");
        // tty_write(&tty, a, 1);
        return;
    }
    
    // disp_int((int)current_process);
    PROCESS *prev = current_process;
    for (int i = 0; i < PROC_NUMBER;i++) {
        if (process_table[i].is_free == 1)
        {
            if (process_table[i].alarm && process_table[i].alarm < ticks)
            {
                process_table[i].signal |= (1 << (SIGALRM - 1));
                process_table[i].alarm = 0;
            }
            if (process_table[i].signal && process_table[i].status == TASK_INTERRUPTIBLE)
            {
                process_table[i].status = TASK_RUNNING;
            }
        }
    }
    do
    {
        current_process++;

        // if (disp_pos>80*25) {
        //     disp_pos = 0;
        // }
        if (current_process >= process_table + PROC_NUMBER)
        {
            current_process = process_table;
        }
    } while (current_process->is_free != 1 || current_process->status != TASK_RUNNING);
    if (prev!=current_process) {
        // printk("1:%x %x %d %d\n",current_process->kernel_regs.esp, prev->kernel_regs.esp, current_process->pid, prev->pid);
        load_cr3(current_process->page_dir->entry);
        switch_to(prev, current_process, prev);
        // printk("2:%x %x %d %d\n",current_process->kernel_regs.esp, prev->kernel_regs.esp, current_process->pid, prev->pid);
    }
}

int sys_pause()
{
	current_process->status = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}

int sys_alarm(unsigned int seconds)
{
	current_process->alarm = (seconds>0)?(ticks+HZ*seconds):0;
	return seconds;
}

void interruptible_sleep_on(PROCESS **p)
{
	PROCESS *tmp;

	if (!p)
		return;
	// if (current_process == &(init_task.task))
	// 	panic("task[0] trying to sleep");
	tmp=*p;
	*p=current_process;
repeat:	current_process->status = TASK_INTERRUPTIBLE;
	schedule();
	if (*p && *p != current_process) {
		(**p).status=0;
		goto repeat;
	}
	*p=NULL;
	if (tmp)
		tmp->status=0;
}

void wake_up(PROCESS **p)
{
	if (p && *p) {
		(**p).status=0;
		*p=NULL;
	}
}