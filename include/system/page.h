#ifndef __SYSTE_PAGE_H
#define __SYSTE_PAGE_H

#define PAGE_OFFSET 0xC0000000 // 3GB   内核入口地址为0xC0100000-0xc0150000 (320K)   0xc0150000开始的1MB给kmalloc

#define PDINDEX(virtualaddr) ((unsigned long)virtualaddr >> 22) //获取虚拟地址的目录页下标
#define PTINDEX(virtualaddr) ((unsigned long)virtualaddr >> 12 & 0x03FF) //获取虚拟地址的页下标

/*----------------------------------------------------------------------------
; 分页机制使用的常量说明
;----------------------------------------------------------------------------
*/
#define PG_P		1	// 页存在属性位
#define PG_RWR		0	// R/W 属性位值, 读/执行
#define PG_RWW		2	// R/W 属性位值, 读/写/执行
#define PG_USS		0	// U/S 属性位值, 系统级
#define PG_USU		4	// U/S 属性位值, 用户级

struct PageDir{
    unsigned int entry[1024];
};

struct PageTable{
    unsigned int entry[1024];
};

struct Page{
    unsigned int pyhsics_addr;
};

extern unsigned int page_start;
extern unsigned int  page_table_count;
extern unsigned int kernel_page_count;
extern struct Page *mem_map;
extern struct PageDir *swapper_pg_dir;

#define __pa(v_addr) ((int)(v_addr)-PAGE_OFFSET)
#define __va(p_addr) ((int)(p_addr)+PAGE_OFFSET)
#define get_pt_entry_p_addr(entry) ((struct PageTable*)((int)(entry) & 0xfffff000))
#define get_pt_entry_v_addr(entry) ((struct PageTable*)(__va(get_pt_entry_p_addr(entry))))

#define invalidate() \
__asm__ __volatile__("movl %%cr3,%%eax\n\tmovl %%eax,%%cr3": : :"ax")

#define load_cr3(pgdir) \
    asm volatile("movl %0,%%cr3": :"r" (__pa(pgdir)))

struct PageDir *create_dir();
unsigned int create_table(unsigned int attr);
void clearDir(struct PageDir *pd);
void removeDir(struct PageDir *pd);
void copy_page(struct PageDir *pd, struct PageDir **res);
void init_process_page(struct PageDir **res);
unsigned int get_free_page();
void free_page(unsigned int pyhsics_addr);
#endif