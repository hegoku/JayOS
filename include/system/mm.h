#ifndef __SYSTEM_MM_H
#define __SYSTEM_MM_H

#define ARD_TYPE_FREE 1
#define ADR_TYPE_RESERVED 2

/*----------------------------------------------------------------------------
; 分页机制使用的常量说明
;----------------------------------------------------------------------------
*/
#define PG_P		1	// 页存在属性位
#define PG_RWR		0	// R/W 属性位值, 读/执行
#define PG_RWW		2	// R/W 属性位值, 读/写/执行
#define PG_USS		0	// U/S 属性位值, 系统级
#define PG_USU		4	// U/S 属性位值, 用户级

/*Type 3: ACPI reclaimable memory
Type 4: ACPI NVS memory
Type 5: Area containing bad memory*/
struct ARDStruct{
    unsigned int baseAddrLow;
    unsigned int baseAddrHigh;
    unsigned int lengthLow;
    unsigned int lengthHigh;
    unsigned int type;
}__attribute__ ((packed));

struct PageDir{
    struct PageTable *entry;
} __attribute__((packed));

struct PageTable{
    unsigned long entry;
} __attribute__((packed));

extern unsigned long MemSize;

void load_memory_size();
void init_paging();
void *kmalloc(int size);
void kfree(void *ptr, int size);
void *kzmalloc(int size);

#endif