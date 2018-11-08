#ifndef	_STDLIB_H_
#define	_STDLIB_H_

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

char *itoa(char *str, int num);

#endif