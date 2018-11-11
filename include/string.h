#ifndef	_STRING_H_
#define	_STRING_H_

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

void memcpy(void * dest, const void * src, int n); //func.inc
void memset(void* p_dst, char ch, size_t n); //func.inc
int	strcmp(const char * s1, const char *s2);
char* strcpy(char* p_dst, char* p_src);
size_t strlen(const char *str);
#endif