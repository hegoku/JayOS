#include <sys/types.h>
#include <system/mm.h>
#include <system/page.h>
#include <string.h>
#include <../kernel/global.h>
#include <../kernel/kernel.h>

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
                    // int new_page = get_free_page();
                    // get_pt_entry_v_addr((*res)->entry[i])->entry[j] = new_page | PG_P | PG_RWW | PG_USU;
                    // memcpy(__va(new_page), __va(get_pt_entry_v_addr(pd->entry[i])->entry[j] & 0xfffff000), 1024 * 4);
                    get_pt_entry_v_addr((*res)->entry[i])->entry[j] = get_pt_entry_v_addr(pd->entry[i])->entry[j] & ~PG_RWW;
                    get_pt_entry_v_addr(pd->entry[i])->entry[j] &= ~PG_RWW;
                    mem_map[(get_pt_entry_v_addr(pd->entry[i])->entry[j] & 0xfffff000) >> 12].count++;
                    mem_map[(get_pt_entry_v_addr(pd->entry[i])->entry[j] & 0xfffff000) >> 12].pyhsics_addr |= MP_COW;
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
    invalidate();
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

//返回一个未使用的页的物理地址, 即物理页地址
PG_P_ADDR get_free_page()
{
    for (int i = 0; i < page_table_count * 1024;i++) {
        if ((mem_map[i].pyhsics_addr & MP_USE) == 0)
        {
            mem_map[i].pyhsics_addr &= 0xfffff000;
            mem_map[i].pyhsics_addr |= MP_USE;
            mem_map[i].count = 0;
            return mem_map[i].pyhsics_addr & 0xfffff000;
        }
    }
    printk("Not more page.\n");
    return -1;
}

//将物理页释放
void free_page(PG_P_ADDR pyhsics_addr)
{
    pyhsics_addr >>= 12;
    mem_map[pyhsics_addr].pyhsics_addr &= 0xfffff000;
    // mem_map[pyhsics_addr].count--;
}

// void do_wp_page(unsigned int error_code, unsigned int address)
// {
//     int index1 = address >> 22;
//     int index2 = address >> 12 & 0x03FF;
//     unsigned int old_page, new_page;
//     old_page = 0xfffff000 & get_pt_entry_v_addr(current_process->page_dir->entry[index1])->entry[index2];
//     new_page = get_free_page();
//     get_pt_entry_v_addr(current_process->page_dir->entry[index1])->entry[index2] = new_page | PG_P | PG_RWW | PG_USU;
//     printk("do_wap_page pid(%d): %x %x %x %x eip:%x esp:%x\n", current_process->pid, address, old_page, new_page, old_page >> 12, current_process->regs.eip, current_process->regs.esp);
//     // while (1)
//     // {
//     // }
//     invalidate();
//     // memset((void *)__va(old_page), 2, 1024);
//     memcpy((void *)__va(new_page), (void *)__va(old_page), 1024*4);
//     mem_map[new_page >> 12].count++;
//     mem_map[old_page >> 12].count--;
//     if (mem_map[old_page >> 12].count==0) {
//         free_page(old_page);
//     }
//     if (current_process->pid==2) {
//         while(1){}
//     }
//     // __asm__("cld ; rep ; movsl" ::"S"(__va(old_page)), "D"(__va(new_page)), "c"(1024)
//     //         : "cx", "di", "si");
// }

void do_wp_page(unsigned int error_code, unsigned int address)
{
    int index1 = address >> 22;
    int index2 = address >> 12 & 0x03FF;
    unsigned int old_page, new_page;
    old_page = 0xfffff000 & get_pt_entry_v_addr(current_process->page_dir->entry[index1])->entry[index2];
    if (mem_map[old_page >> 12].count==1) {
        get_pt_entry_v_addr(current_process->page_dir->entry[index1])->entry[index2] = old_page | PG_P | PG_RWW | PG_USU;
        mem_map[old_page >> 12].pyhsics_addr &= 0xfffff000;
        mem_map[old_page >> 12].pyhsics_addr |= MP_USE;
        new_page = old_page;
    }
    else
    {
        new_page = get_free_page();
        get_pt_entry_v_addr(current_process->page_dir->entry[index1])->entry[index2] = new_page | PG_P | PG_RWW | PG_USU;
        memcpy((void *)__va(new_page), (void *)__va(old_page), 1024 * 4);
        mem_map[new_page >> 12].count++;
        mem_map[old_page >> 12].count--;
    }

    printk("do_wap_page pid(%d): addr:%x old:%x new:%x %x eip:%x esp:%x\n", current_process->pid, address, old_page, new_page, old_page >> 12, current_process->regs.eip, current_process->regs.esp);

    invalidate();

    if (current_process->pid==2) {
        // while(1){}
    }
}

void clear_page_tables(struct PageDir *pd)
{
    for (int i = 0; i < 768; i++) {
        if (pd->entry[i]!= 0) {
            free_page((int)get_pt_entry_p_addr(pd->entry[i]));
            // kfree(get_entry_address(pd->entry[i]), sizeof(struct PageTable));
            // pd->entry[i] = 0;
        }
    }
    invalidate();
}


void do_no_page(unsigned int error_code, unsigned int address)
{
    int index1 = address >> 22;
    int index2 = address >> 12 & 0x03FF;
    unsigned int new_page;

    new_page = get_free_page();
    current_process->page_dir->entry[index1] = create_table(PG_P | PG_RWW | PG_USU);
    get_pt_entry_v_addr(current_process->page_dir->entry[index1])->entry[index2] = new_page | PG_P | PG_RWW | PG_USU;
    mem_map[new_page >> 12].count++;

    printk("do_no_page pid(%d): addr:%x new:%x eip:%x esp:%x\n", current_process->pid, address, new_page, current_process->regs.eip, current_process->regs.esp);

    invalidate();

    // if (current_process->pid==2) {
        // while(1){}
    // }
}