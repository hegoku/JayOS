#ifndef _SYSTEM_SYSTEM_CALL_H
#define _SYSTEM_SYSTEM_CALL_H

#define SYS_CALL_NUMBER 190

#define SYS_CALL_READ 3
#define SYS_CALL_WRITE 4
#define SYS_CALL_OPEN 5
#define SYS_CALL_CLOSE 6
#define SYS_CALL_MOUNT 15
#define SYS_CALL_STAT 18
#define SYS_CALL_LSEEK 19

typedef void *sys_call_handler;

void init_system_call(sys_call_handler sys_call_table[]);

#endif
