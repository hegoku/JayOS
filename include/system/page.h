#ifndef __SYSTE_PAGE_H
#define __SYSTEM_PAGE_H

#define PDINDEX(virtualaddr) ((unsigned long)virtualaddr >> 22) //获取虚拟地址的目录页下标
#define PTINDEX(virtualaddr) ((unsigned long)virtualaddr >> 12 & 0x03FF) //获取虚拟地址的页下标

struct Page_Dir{
    struct Page_table *entry[1024];
};

struct Page_table{
    unsigned int entry[1024];
};

inline struct Page_Dir *create_dir();
inline struct Page_table *create_table();
inline void clearDir(struct Page_Dir *pd);
inline void removeDir(struct Page_Dir *pd);
#endif