#include <system/system_call.h>
#include "../kernel/tty.h"
#include "../kernel/kernel.h"
#include <sys/types.h>
#include <system/fs.h>
#include <system/dev.h>
#include <system/process.h>
#include <system/schedule.h>

static int sys_get_ticks();

sys_call_handler sys_call_table[65] = {
    sys_get_ticks,
    sys_exit,
    sys_fork,
    sys_read,
    sys_write,
    sys_open,
    sys_close,
    sys_waitpid, //7
    NULL,
    NULL,
    NULL,
    sys_execve,
    NULL,
    NULL,
    NULL,
    sys_mount, //15
    NULL,
    NULL,
    sys_stat, //18
    sys_lseek,
    sys_getpid, //20
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    sys_alarm, //27
    NULL,
    sys_pause, //29
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    sys_mkdir, //39
    NULL,
    sys_dup, //41
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    sys_chroot, //61
    NULL,
    NULL,
    sys_getppid //64
};

// void init_system_call(sys_call_handler sys_call_table[])
// {
//     sys_call_table[0] = sys_get_ticks;
//     sys_call_table[SYS_CALL_EXIT] = sys_exit;
//     sys_call_table[SYS_CALL_FORK] = sys_fork;
//     sys_call_table[SYS_CALL_READ] = sys_read;
//     sys_call_table[SYS_CALL_WRITE] = sys_write;
//     sys_call_table[SYS_CALL_OPEN] = sys_open;
//     sys_call_table[SYS_CALL_CLOSE] = sys_close;
//     sys_call_table[SYS_CALL_WAITPID] = sys_wait;
//     sys_call_table[SYS_CALL_MOUNT] = sys_mount;
//     sys_call_table[SYS_CALL_STAT] = sys_stat;
//     sys_call_table[SYS_CALL_LSEEK] = sys_lseek;
//     sys_call_table[SYS_CALL_GETPID] = sys_getpid;
//     sys_call_table[SYS_CALL_DUP] = sys_dup;
//     sys_call_table[SYS_CALL_GETPPID] = sys_getppid;
// }

static int sys_get_ticks()
{
    return ticks;
}