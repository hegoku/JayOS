
#include <system/signal.h>
#include <system/schedule.h>
#include "../kernel/kernel.h"

void send_sig(unsigned char sig, PROCESS *p)
{
    p->signal = sig;
} 

void do_signal()
{

    if (current_process->status!=TASK_RUNNING) {
        schedule();
    }
}