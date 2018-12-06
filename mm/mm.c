#include <system/mm.h>
#include <string.h>
#include <sys/types.h>
#include "../kernel/global.h"

#define MAX_SIZE 128*1024 //最大申请空间 128KB

static unsigned short MCRNumber;
static struct ARDStruct MemChkBuf[12];
unsigned long MemSize;
static unsigned long base_heap_addr; //堆基址
static unsigned long max_heap_addr; //堆顶
static char mmap[131072]; //一个占用512B, 1MB/512B=2048 2048/8=256

void load_memory_size()
{
    memcpy(MemChkBuf, (void *)(0x7e00+2), 256);
    MCRNumber = *(unsigned short *)(0x7e00);
    for (int i = 0; i < MCRNumber; i++)
    {
        if ( MemChkBuf[i].type==ARD_TYPE_FREE) {
            if (MemChkBuf[i].baseAddrLow+MemChkBuf[i].lengthLow>MemSize) {
                MemSize = MemChkBuf[i].baseAddrLow + MemChkBuf[i].lengthLow;
                if (MemChkBuf[i].baseAddrLow>0x10000) {
                    max_heap_addr = MemChkBuf[i].baseAddrLow + MemChkBuf[i].lengthLow;
                }
            }
        }
    }
    base_heap_addr = max_heap_addr - 1024 * 1024; //1MB的内核空间
    // disp_int(mmap);
    // printk("mmap:%x\n", mmap);
}

void *kmalloc(int size)
{
    if (size>MAX_SIZE) {
        return NULL;
    }
    int start_p = 0; //开始块的下标
    int count = 0; //连续了几块
    // int block = (size + 512 - 1) / 512; //需要申请多少块
    // int block = (size + 512 - 1) / 512; //需要申请多少块
    for (int i = 0; i < 1024*1024;)
    {
        int j = i / 8;
        unsigned char tmp = mmap[j]>>(8 - (i-j*8) - 1);
        i++;
        if ((tmp&1)==1) {
            count = 0;
            start_p = i;
        }
        else
        {
            count++;
        }
        if (count==size) {
            break;
        }
    }
    for (int i = start_p; i < (start_p+size);i++) {
        
        int j = i / 8;
        unsigned char s1 = mmap[j]>>(8 - (i-j*8) - 1);
        s1 = (s1 | 1)<<(8 - (i-j*8) - 1);
        mmap[j]=s1|mmap[j];
    }
    return (void *)(base_heap_addr + start_p);
}

void kfree(void *ptr, int size)
{
    int offset = (int)ptr - base_heap_addr;
    // printk("%x %d\n", (int)ptr, offset);
    for (int i = offset; i < (offset+size);i++) {
        int j = i / 8;
        unsigned char s1 = mmap[j] >> ((8 - (i - j * 8) - 1));
        unsigned char s2 = mmap[j] << ((i - j * 8) + 1);
        s1 = (s1 & 0b11111110) << ((8 - (i - j * 8) - 1));
        s2 = s2 >> ((i - j * 8) + 1);
        mmap[j] = s1 | s2;
    }
}

void *kzmalloc(int size)
{
    void *a = kmalloc(size);
    if (a) {
        memset(a, 0, size);
    }
    return a;
}