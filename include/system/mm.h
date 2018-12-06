#ifndef __SYSTEM_MM_H
#define __SYSTEM_MM_H

#define ARD_TYPE_FREE 1
#define ADR_TYPE_RESERVED 2
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

extern unsigned long MemSize;

void load_memory_size();
void *kmalloc(int size);
void kfree(void *ptr, int size);

#endif