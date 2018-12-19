#include <sys/types.h>
#include <system/mm.h>
#include <system/page.h>
#include <string.h>
#include <../kernel/global.h>

unsigned int page_start = PAGE_OFFSET+1024*1024 + 512 * 1024;
unsigned int  page_table_count = 1;
unsigned int kernel_page_count = 1;
struct Page *mem_map = (void*)1;
struct PageDir *swapper_pg_dir=(void*)(PAGE_OFFSET+0x1000);

inline struct PageDir *create_dir()
{
    // return kzmalloc(sizeof(struct PageDir));
    return (struct PageDir*)(get_free_page()+PAGE_OFFSET);
}

inline unsigned int create_table(unsigned int attr)
{
    // int a=get_free_page();
    // printk("%x %x %x\n", a, a+PAGE_OFFSET, (a+PAGE_OFFSET)|attr);
    // return ((int)kzmalloc(sizeof(struct PageTable)) - PAGE_OFFSET) | attr;
    return ((get_free_page())|attr);
}

//清除并释放pd下所有pt
inline void clearDir(struct PageDir *pd)
{
    for (int i = 0; i < 1024; i++) {
        if (pd->entry[i]!= 0) {
            free_page((int)get_pt_entry_p_addr(pd->entry[i]));
            // kfree(get_entry_address(pd->entry[i]), sizeof(struct PageTable));
            pd->entry[i] = 0;
        }
    }
}

//清除并释放pd
inline void removeDir(struct PageDir *pd)
{
    clearDir(pd);
    free_page(__pa(pd));
}

void copy_page(struct PageDir *pd, struct PageDir **res)
{
    int i = 0;
    int j = 0;
    for (i = 0; i < 768; i++)
    {
        if (pd->entry[i]!= 0) {
            (*res)->entry[i] = create_table(PG_P | PG_RWW | PG_USU);
            // printk("%x %d ", (*res)->entry[i], i);while(1){}
            for (j = 0; j < 1024; j++)
            {
                if (get_pt_entry_v_addr(pd->entry[i])->entry[j]!= 0) {
                    get_pt_entry_v_addr((*res)->entry[i])->entry[j] = get_pt_entry_v_addr(pd->entry[i])->entry[j] & 0xfffff000 | PG_P | PG_RWW | PG_USU;
                    //((struct PageTable*)((int)pd->entry[i] & 0xfffff000))
                    // printk(":%x:", get_entry_address(pd->entry[i])->entry[j]);
                    // (*res)->entry[i]->entry[j] = (unsigned int)kzmalloc(1024*4);
                    // memcpy((void*)((*res)->entry[i]->entry[j]), (void*)pd->entry[i]->entry[j], 1024*4);
                }
                else
                {
                    get_pt_entry_v_addr((*res)->entry[i])->entry[j] = 0;
                }
            }
        }
        else
        {
            (*res)->entry[i] = 0;
        }
    }
    for (i = 768; i < 1024; i++)
    {
        (*res)->entry[i]= swapper_pg_dir->entry[i];
        // for (j = 0; j < 1024; j++) {
        //     get_pt_entry_v_addr((*res)->entry[i])->entry[j] = get_pt_entry_v_addr(swapper_pg_dir->entry[i])->entry[j];
        // }
    }
}

void init_process_page(struct PageDir **res)
{
    *res = create_dir();

    // struct PageTable *pt = (struct PageTable *)kzmalloc(page_table_count * 1024 * 4);
    // unsigned int a = 0;
    // int k = 768;
    // pt += 1024 * (page_table_count - kernel_page_count);
    // copy_page(swapper_pg_dir, res);
    for (int i = 0; i < 1024; i++)
    {
        (*res)->entry[i]= swapper_pg_dir->entry[i];
        // (*res)->entry[i]=kzmalloc(sizeof(struct PageTable));
        for (int j = 0; j < 1024; j++) {
            // (*res)->entry[i]->entry[j] = swapper_pg_dir->entry[i]->entry[j];
            get_pt_entry_v_addr((*res)->entry[i])->entry[j] = get_pt_entry_v_addr(swapper_pg_dir->entry[i])->entry[j];
        }
        // if (i>967) {

        // }
        // memcpy((*res)->entry[i]->entry, swapper_pg_dir->entry[i]->entry, 1024 * 4);
    }
    // for (int i = page_table_count - kernel_page_count; i < page_table_count; i++)
    // {
    //     struct PageTable *pt = create_table();
    //     (*res)->entry[k] = (void *)((unsigned int)pt | PG_P | PG_RWW | PG_USS);
    //     for (int j = 0; j < 1024; j++)
    //     {
    //         pt->entry[j] = a | PG_P | PG_RWW | PG_USS;
    //         a += 4096;
    //     }
    //     k++;
    //     // pt += 1024;
    //     }
}

unsigned int get_free_page()
{
    for (int i = 0; i < page_table_count * 1024;i++) {
        if ((mem_map[i].pyhsics_addr & 1) == 0)
        {
            mem_map[i].pyhsics_addr |= 1;
            return mem_map[i].pyhsics_addr & 0xfffff000;
        }
    }
    printk("Not more page.\n");
    return -1;
}

void free_page(unsigned int pyhsics_addr)
{
    pyhsics_addr >>= 12;
    mem_map[pyhsics_addr].pyhsics_addr &= 0xfffff000;
}