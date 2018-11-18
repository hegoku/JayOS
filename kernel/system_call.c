#include <system/system_call.h>
#include "../kernel/tty.h"
#include "../kernel/kernel.h"
#include <sys/types.h>
#include <system/fs.h>

static int sys_get_ticks();
static ssize_t sys_write(int fildes, const void *buf, unsigned int nbyte);

void init_system_call(sys_call_handler sys_call_table[])
{
    sys_call_table[0] = sys_get_ticks;
    sys_call_table[SYS_CALL_WRITE] = sys_write;
    sys_call_table[SYS_CALL_OPEN] = sys_open;
}

static int sys_get_ticks()
{
    return ticks;
}

static ssize_t sys_write(int fildes, const void *buf, unsigned int nbyte)
{
    return tty_write(&tty_table[current_process->tty], (char *)buf, nbyte);
}