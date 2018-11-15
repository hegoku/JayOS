#include <system/system_call.h>
#include "../kernel/tty.h"
#include <sys/types.h>

extern int ticks;
extern TTY tty;

static int sys_get_ticks();
static ssize_t sys_write(int fildes, const void *buf, unsigned int nbyte);

void init_system_call(sys_call_handler sys_call_table[])
{
    sys_call_table[0] = sys_get_ticks;
    sys_call_table[1] = sys_write;
}

static int sys_get_ticks()
{
    return ticks;
}

static ssize_t sys_write(int fildes, const void *buf, unsigned int nbyte)
{
    return tty_write(&tty, (char *)buf, nbyte);
}