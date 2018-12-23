
#include <system/signal.h>
#include <system/schedule.h>

void send_sig(unsigned char sig, PROCESS *p)
{
    p->signal = sig;

} 