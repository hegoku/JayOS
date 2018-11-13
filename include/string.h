#ifndef	_STRING_H_
#define	_STRING_H_

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

char* strcpy(char* p_dst, char* p_src); //string.asm
size_t strlen(const char *str); //string.asm

void *memset(void *str, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
char *strchr(const char *str, int c);
char *strncpy(char *dest, const char *src, size_t n);
size_t strcspn(const char *str1, const char *str2);
int	strcmp(const char * s1, const char *s2);
#endif