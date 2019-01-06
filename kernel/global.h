
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

void DispStr(char* msg);
// void memcpy(void *pDest, void *pSrc, int iSize);
void DispColorStr(char * info, int color);
void out_byte(unsigned short port, unsigned char value);
unsigned char in_byte(unsigned short port);
// void MemSet(void* p_dst, char ch, int size);
void port_read(unsigned short port, void* buf, int n);
void port_write(unsigned short port, void* buf, int n);
void disable_int();
void enable_int();

void disp_int(int input);
// char *itoa(char *str, int num);
int printk(const char *fmt, ...);

extern int is_in_ring0;

int printk(const char * format, ...);
void panic(const char *info);
#endif