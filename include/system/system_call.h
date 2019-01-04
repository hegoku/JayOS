#ifndef _SYSTEM_SYSTEM_CALL_H
#define _SYSTEM_SYSTEM_CALL_H

#define SYS_CALL_NUMBER 190

#define SYS_CALL_EXIT 1
#define SYS_CALL_FORK 2
#define SYS_CALL_READ 3
#define SYS_CALL_WRITE 4
#define SYS_CALL_OPEN 5
#define SYS_CALL_CLOSE 6
#define SYS_CALL_WAITPID 7
#define SYS_CALL_EXECVE 11
#define SYS_CALL_CHDIR 12
#define SYS_CALL_MOUNT 15
#define SYS_CALL_STAT 18
#define SYS_CALL_LSEEK 19
#define SYS_CALL_GETPID 20
#define SYS_CALL_ALARM 27
#define SYS_CALL_PAUSE 29
#define SYS_CALL_MKDIR 39
#define SYS_CALL_DUP 41
#define SYS_CALL_CHROOT 61
#define SYS_CALL_GETPPID 64
#define SYS_CALL_GETDENTS 141

typedef void *sys_call_handler;

// void init_system_call(sys_call_handler sys_call_table[]);

#endif
