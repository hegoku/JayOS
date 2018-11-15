#ifndef _SYSTEM_SYSTEM_CALL_H
#define _SYSTEM_SYSTEM_CALL_H

#define SYS_CALL_NUMBER 2

typedef void *sys_call_handler;

void init_system_call(sys_call_handler sys_call_table[]);

#endif
