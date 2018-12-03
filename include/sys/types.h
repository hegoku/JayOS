#ifndef _SYS_TYPES_H
#define _SYS_TYPES_H

#ifndef _SSIZE_T
#define _SSIZE_T
typedef signed int ssize_t;
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef _PID_T
#define _PID_T
typedef unsigned int pid_t;
#endif

#ifndef _OFF_T
#define _OFF_T
typedef unsigned long off_t;
#endif

#ifndef NULL
#define NULL ((void *) 0)
#endif

#endif