#ifndef _SYSTEM_SYSTEM_CALL_H
#define _SYSTEM_SYSTEM_CALL_H

#define SYS_CALL_NUMBER 3

#define SYS_CALL_READ 0
#define SYS_CALL_WRITE 1
#define SYS_CALL_OPEN 2
#define SYS_CALL_CLOSE 3

typedef void *sys_call_handler;

void init_system_call(sys_call_handler sys_call_table[]);

#endif
