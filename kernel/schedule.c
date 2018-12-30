#include "kernel.h"
#include "global.h"
#include <system/page.h>
#include <system/schedule.h>

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
        // printk("%d %d %d %d|",current_process->is_free, current_process->status, current_process->pid, prev->pid);
        load_cr3(current_process->page_dir->entry);
        switch_to(prev, current_process, prev);
    }
}

int sys_pause()
{
	current_process->status = TASK_INTERRUPTIBLE;
	schedule();
	return 0;
}