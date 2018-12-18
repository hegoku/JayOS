#include <system/mm.h>
#include <string.h>
#include <sys/types.h>
#include <system/page.h>
#include "../kernel/global.h"

#define MAX_SIZE 128*1024 //最大申请空间 128KB
// #define KM_START 0xC0150000
// #define KM_END 0xC0250000

static unsigned short MCRNumber=1;
static struct ARDStruct MemChkBuf[12]={1};
unsigned long MemSize = 1;
static unsigned long base_heap_addr=1; //堆基址
static unsigned long max_heap_addr=1; //堆顶
static char mmap[131072]={1}; //一个占用512B, 1MB/512B=2048 2048/8=256
static unsigned int kernel_heap_start = 1;
static unsigned int kernel_heap_end = 1;

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
    page_table_count = (MemSize + 0x400000 - 1) / 0x400000;
    struct PageDir *page_dir_ptr = (void *)(1024*1024 + 512 * 1024);
    page_start = (unsigned int)page_dir_ptr + 1024 * 4;
    unsigned int page_end = (page_start + page_table_count * 1024 * 4) - 1;
    kernel_heap_start = page_end + 1;
    kernel_heap_end = kernel_heap_start + 1024 * 1024-1;
    kernel_page_count = (kernel_heap_end+1 + 0x1000 - 1) / 0x1000;
    printk("MemSize: %x\n", MemSize);
    printk("page_table_count: %x\n", page_table_count);
    printk("pdir: %x\n", page_dir_ptr);
    printk("page_start: %x\n", page_start);
    printk("page_end: %x\n", page_end);
    printk("kernel_heap_start: %x\n", kernel_heap_start);
    printk("kernel_heap_end: %x\n", kernel_heap_end);
    printk("kernel_page_count: %x\n", kernel_page_count);
    // page_dir_ptr = kzmalloc(1024*4);
    // disp_int(page_dir_ptr);
    // page_dir_ptr[0].entry = (void *)((int)kzmalloc(1024 * 4) | PG_P | PG_RWW | PG_USU);
    // page_dir_ptr[768].entry = (void *)((int)kzmalloc(1024 * 4) | PG_P | PG_RWW | PG_USU);
    // struct PageDir *page_dir_ptr = (void *)(0x1000);

    // page_dir_ptr->entry[0] = (void*)(0x2000| PG_P | PG_RWW | PG_USU);
    // page_dir_ptr->entry[1] = (void*)(0x3000| PG_P | PG_RWW | PG_USU);
    // page_dir_ptr->entry[768] = (void*)(0x2000| PG_P | PG_RWW | PG_USU);
    // page_dir_ptr->entry[769] = (void*)(0x3000| PG_P | PG_RWW | PG_USU);
    // struct PageTable *pt = (struct PageTable *)0x2000;
    struct PageTable *pt = (struct PageTable *)page_start;
    int a = 0;
    for (int j = 0; j < page_table_count;j++) {
        for (int i = 0; i < 1024; i++)
        {
            pt->entry[i] = a | PG_P | PG_RWW | PG_USS;
            a += 4096;
        }
        pt+=1024;
    }

    page_dir_ptr->entry[0] = (void *)(page_start | PG_P | PG_RWW | PG_USS);
    page_dir_ptr->entry[1] = (void*)(page_start+0x1000| PG_P | PG_RWW | PG_USS);
    page_dir_ptr->entry[768] = (void*)(page_start| PG_P | PG_RWW | PG_USS);
    page_dir_ptr->entry[769] = (void*)(page_start+0x1000| PG_P | PG_RWW | PG_USS);
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
    if (kernel_heap_start + start_p>kernel_heap_end) {
        printk("kmalloc full\n");
        return NULL;
    }
    // printk("%x\n", KM_START+start_p);
    return (void *)(PAGE_OFFSET+kernel_heap_start+start_p);
    // return (void *)(base_heap_addr + start_p);
}

void kfree(void *ptr, int size)
{
    // int offset = (int)ptr - base_heap_addr;
    int offset = (int)ptr - kernel_heap_start-PAGE_OFFSET;
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

