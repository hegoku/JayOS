#include <system/mm.h>
#include <string.h>
#include <sys/types.h>
#include "../kernel/global.h"

#define MAX_SIZE 128*1024 //最大申请空间 128KB
#define PAGE_OFFSET 0xC0000000 // 3GB   内核入口地址为0xC0100000-0xc0150000 (320K)   0xc0150000开始的1MB给kmalloc
#define KM_START 0xC0150000
#define KM_END 0xC0250000

static unsigned short MCRNumber=1;
static struct ARDStruct MemChkBuf[12]={1};
unsigned long MemSize = 1;
static unsigned long base_heap_addr=1; //堆基址
static unsigned long max_heap_addr=1; //堆顶
static char mmap[131072]={1}; //一个占用512B, 1MB/512B=2048 2048/8=256

// extern struct PageDir *page_dir_ptr;

void load_memory_size()
{
    MemSize = 0;
    base_heap_addr = 0;
    max_heap_addr = 0;
    memcpy(MemChkBuf, (void *)(0x7e00 + 2), 256);
    MCRNumber = *(unsigned short *)(0x7e00);
    for (int i = 0; i < MCRNumber; i++)
    {
        if ( MemChkBuf[i].type==ARD_TYPE_FREE) {
            if (MemChkBuf[i].baseAddrLow+MemChkBuf[i].lengthLow>MemSize) {
                MemSize = MemChkBuf[i].baseAddrLow + MemChkBuf[i].lengthLow;
                max_heap_addr = MemChkBuf[i].baseAddrLow + MemChkBuf[i].lengthLow;
                // if (MemChkBuf[i].baseAddrLow>0x10000) {
                //     max_heap_addr = MemChkBuf[i].baseAddrLow + MemChkBuf[i].lengthLow;
                // }
            }
        }
    }
    base_heap_addr = max_heap_addr - 1024 * 1024; //1MB的内核空间
    memcpy(mmap, 0, 131072);
    // disp_int(mmap);
    // printk("mmap:%x\n", mmap);
}

void init_paging()
{
    // int page_count = (MemSize + 0x400000 - 1) / 0x400000;
    // page_dir_ptr = kzmalloc(1024*4);
    // disp_int(page_dir_ptr);
    // page_dir_ptr[0].entry = (void *)((int)kzmalloc(1024 * 4) | PG_P | PG_RWW | PG_USU);
    // page_dir_ptr[768].entry = (void *)((int)kzmalloc(1024 * 4) | PG_P | PG_RWW | PG_USU);
    struct PageDir *page_dir_ptr = (void *)(0x1000);
    page_dir_ptr[0].entry = (void*)(0x2000| PG_P | PG_RWW | PG_USU);
    page_dir_ptr[1].entry = (void*)(0x3000| PG_P | PG_RWW | PG_USU);
    page_dir_ptr[768].entry = (void*)(0x3000| PG_P | PG_RWW | PG_USU);
    page_dir_ptr[769].entry = (void*)(0x3000| PG_P | PG_RWW | PG_USU);
    struct PageTable *pt = (struct PageTable *)0x2000;
    int a = 0;
    for (int i = 0; i < 1024*2; i++)
    {
        pt->entry = a | PG_P | PG_RWW | PG_USU;
        pt++;
        a += 4096;
    }
}

void *kmalloc(int size)
{
    if (size>MAX_SIZE) {
        printk("kmalloc size not greater than 128KB\n");
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
    if (base_heap_addr + start_p>max_heap_addr) {
        printk("kmalloc full\n");
        return NULL;
    }
    // printk("%x\n", KM_START+start_p);
    return (void *)(KM_START+start_p);
    // return (void *)(base_heap_addr + start_p);
}

void kfree(void *ptr, int size)
{
    // int offset = (int)ptr - base_heap_addr;
    int offset = (int)ptr - KM_START;
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