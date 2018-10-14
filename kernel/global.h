
#ifndef _GLOBAL_H_
#define _GLOBAL_H_

extern int disp_pos;

void DispStr(char* msg);
void MemCpy(void *pDest, void *pSrc, int iSize);
void DispColorStr(char * info, int color);
void out_byte(unsigned short port, unsigned char value);
unsigned char in_byte(unsigned short port);

void disp_int(int input);
char *itoa(char *str, int num);

#endif