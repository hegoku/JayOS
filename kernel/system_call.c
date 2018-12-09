#include <system/system_call.h>
#include "../kernel/tty.h"
#include "../kernel/kernel.h"
#include <sys/types.h>
#include <system/fs.h>
#include "global.h"
#include <system/dev.h>
#include "../kernel/process.h"

static int sys_get_ticks();

void init_system_call(sys_call_handler sys_call_table[])
{
    sys_call_table[0] = sys_get_ticks;
    sys_call_table[SYS_CALL_EXIT] = sys_exit;
    sys_call_table[SYS_CALL_FORK] = sys_fork;
    sys_call_table[SYS_CALL_WRITE] = sys_write;
    sys_call_table[SYS_CALL_READ] = sys_read;
    sys_call_table[SYS_CALL_OPEN] = sys_open;
    sys_call_table[SYS_CALL_CLOSE] = sys_close;
    sys_call_table[SYS_CALL_WAITPID] = sys_wait;
    sys_call_table[SYS_CALL_MOUNT] = sys_mount;
    sys_call_table[SYS_CALL_STAT] = sys_stat;
    sys_call_table[SYS_CALL_LSEEK] = sys_lseek;
    sys_call_table[SYS_CALL_GETPID] = sys_getpid;
    sys_call_table[SYS_CALL_DUP] = sys_dup;
    sys_call_table[SYS_CALL_GETPPID] = sys_getppid;
}

static int sys_get_ticks()
{
    return ticks;
}