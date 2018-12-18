#include "global.h"
#include "interrupt.h"
#include "keyboard.h"
#include "fd.h"
#include "hd.h"
#include "tty.h"
#include "process.h"
#include "system/desc.h"
#include "kernel.h"
#include "unistd.h"
#include "stdio.h"
#include <system/system_call.h>
#include <string.h>
#include <fcntl.h>
#include <system/rootfs.h>
#include "../fs/ext2/ext2.h"
#include "../fs/fat/fat.h"
#include "system/mm.h"

int ticks = 1;
TSS tss={
    .esp0 = TOP_OF_KERNEL_STACK,
	.ss0 = GDT_SEL_KERNEL_DATA
};
// DESCRIPTOR ldt[2] = {
//     {
//         1,
//     },
//     {
//         1
//     }
// };
extern irq_handler irq_table[];
PROCESS process_table[PROC_NUMBER];
PROCESS *current_process=(void*)1;

static void init_idt();
static void init_gdt();
void kernel_main();

void clock_handler(int irq);

void init();
void calltest();
void TestA();
void delay();
void milli_delay(int mill_sec);
void task_tty();

extern void restart();

void cstart()
{
    load_memory_size();
    init_gdt();
    init_idt();
    init_paging();
    // hd_open(0);
}

void init_idt()
{
    init_8259A();
    enable_irq(CASCADE_IRQ); //开启从片

    // 全部初始化成中断门(没有陷阱门)
	init_idt_desc(idt, INT_VECTOR_DIVIDE, DA_386IGate, divide_error, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_DEBUG, DA_386IGate, single_step_exception, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_NMI, DA_386IGate, nmi, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_BREAKPOINT, DA_386IGate, breakpoint_exception, PRIVILEGE_USER);

	init_idt_desc(idt, INT_VECTOR_OVERFLOW,	DA_386IGate, overflow, PRIVILEGE_USER);

	init_idt_desc(idt, INT_VECTOR_BOUNDS, DA_386IGate, bounds_check, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_INVAL_OP,	DA_386IGate, inval_opcode, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_COPROC_NOT, DA_386IGate, copr_not_available, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_DOUBLE_FAULT,	DA_386IGate, double_fault, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_COPROC_SEG, DA_386IGate, copr_seg_overrun, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_INVAL_TSS, DA_386IGate, inval_tss, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_SEG_NOT, DA_386IGate, segment_not_present, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_STACK_FAULT, DA_386IGate, stack_exception, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_PROTECTION, DA_386IGate, general_protection, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_PAGE_FAULT, DA_386IGate, page_fault, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_COPROC_ERR, DA_386IGate, copr_error, PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_SYS_CALL, DA_386IGate, sys_call, PRIVILEGE_USER);

    init_idt_desc(idt, INT_VECTOR_IRQ0 + 0, DA_386IGate, hwint00, PRIVILEGE_KRNL); //时钟
    init_idt_desc(idt, INT_VECTOR_IRQ0 + 1, DA_386IGate, hwint01, PRIVILEGE_KRNL); //键盘
    init_idt_desc(idt, INT_VECTOR_IRQ0 + 6, DA_386IGate, hwint06, PRIVILEGE_KRNL); //软盘
    init_idt_desc(idt, INT_VECTOR_IRQ8 + 6, DA_386IGate, hwint14, PRIVILEGE_KRNL); //硬盘
}

static void init_gdt()
{
	DESCRIPTOR gdt_0 = create_descriptor(0, 0, 0);
	insert_descriptor(gdt, 0, gdt_0, PRIVILEGE_KRNL);

	DESCRIPTOR kernel_cs = create_descriptor(0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL0);
    insert_descriptor(gdt, 1, kernel_cs, PRIVILEGE_KRNL);

    DESCRIPTOR kernel_ds = create_descriptor(0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL0);
	insert_descriptor(gdt, 2, kernel_ds, PRIVILEGE_KRNL);

	DESCRIPTOR video = create_descriptor(0xB8000, 0xBFFFF, DA_DRW | DA_32 | DA_DPL3);
	insert_descriptor(gdt, 3, video, PRIVILEGE_USER);

	DESCRIPTOR user_cs = create_descriptor(0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3);
	insert_descriptor(gdt, 4, user_cs, PRIVILEGE_USER);

	DESCRIPTOR user_ds = create_descriptor(0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3);
	insert_descriptor(gdt, 5, user_ds, PRIVILEGE_USER);

    DESCRIPTOR tss_desc = create_descriptor((unsigned int)&tss, sizeof(TSS) - 1, DA_386TSS);
    insert_descriptor(gdt, 6, tss_desc, PRIVILEGE_KRNL);

    // ldt[0]=create_descriptor(0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3);
    // ldt[1]=create_descriptor(0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3);
    // DESCRIPTOR ldt=create_descriptor((unsigned int)&ldt, 2*sizeof(DESCRIPTOR)-1, DA_LDT);
	// insert_descriptor(gdt, 7, ldt, PRIVILEGE_KRNL);
	
    // GATE call_test = create_gate(SelectorKernelCs, (unsigned int)&calltest, 0, DA_386CGate | DA_DPL3);
    // testcallS=insert_descriptor(gdt, 7, gate_to_descriptor(call_test), PRIVILEGE_KRNL);

    // DESCRIPTOR s3 = create_descriptor(0, TOP_OF_KERNEL_STACK, DA_DRWA|DA_32|DA_DPL0);
	// ss3=insert_descriptor(gdt, 8, s3, PRIVILEGE_KRNL);
}

void kernel_main()
{
    ticks = 0;
    current_process = NULL;
    // init_system_call(sys_call_table);

    for (int i = 0; i < PROC_NUMBER;i++) {
        process_table[i].pid = i;
        create_process(gdt, &process_table[i], 0);
    }

    sprintf(process_table[0].name, "init");
    process_table[0].is_free = 1;
    process_table[0].regs.eip = (unsigned int)init;

    // sprintf(process_table[1].name, "tty");
    // process_table[1].is_free = 1;
    // process_table[1].regs.eip = (unsigned int)task_tty;

    // process_table[1].pid = 1;
    // process_table[1].name[0]='B';
    // process_table[1].name[1]='\0';
    // process_table[1].regs.eip = (unsigned int)calltest;
    // // create_process(gdt, &process_table[1], (unsigned int)calltest);

    // process_table[2].pid = 2;
    // process_table[2].name[0] = 'A';
    // process_table[2].name[1] = '\0';
    // process_table[2].regs.eip = (unsigned int)TestA;
    // create_process(gdt, &process_table[2], (unsigned int)TestA);

    is_in_ring0 = 0;
    current_process = process_table;

    init_rootfs();
    init_fat12();
    mount_root();

    init_tty();

    irq_table[CLOCK_IRQ] = clock_handler;
    enable_irq(CLOCK_IRQ);

    out_byte(0x43, 0x34);
    out_byte(0x40, (unsigned char)(1193182L / 100));
    out_byte(0x40, (unsigned char)((1193182L / 100)>>8));

    keyboard_init();
    irq_table[KEYBOARD_IRQ] = keyboard_handler;
    enable_irq(KEYBOARD_IRQ);

    init_hd();
    irq_table[AT_WINI_IRQ] = hd_handler;
    enable_irq(AT_WINI_IRQ);
    hd_setup();

    // struct dir_entry *tmp = current_process->root;
    // for (int i = 0; i < PROC_NUMBER; i++)
    // {
    //     process_table[i].pwd = tmp;
    //     process_table[i].root = tmp;
    // }

    printk("MemSize:%d\n", MemSize);
    char *a = kmalloc(1);
    *a = 'B';
    printk("%x %c\n", a, *a);
    kfree(a, 1);

    char *b = kmalloc(7);
    memcpy(b, "ABCDEF", 7);
    printk("%x %s\n", b, b);

    char *d = kmalloc(3);
    memset(d, 0, 3);
    memcpy(d, "BB", 3);
    printk("%x %s\n", d, d);

    char *c = kmalloc(10);
    memcpy(c, " 1234789.", 10);
    printk("%x %s\n", c, c);
    kfree(d, 3);

    a = kmalloc(1);
    *a = 'Z';
    printk("%x %c\n", a, *a);
    printk("%x %s\n", b, b);

    // for (int i = 0; i < TTY_NUM; i++) {
    //     struct inode *a=get_inode();
    //     a->dev_num = MKDEV(4, i);
    //     a->mode = FILE_MODE_CHR;
    //     a->f_op = dev_table[4].f_op;
    // }

    // struct inode *a=get_inode();
    // a->dev_num = MKDEV(3, 0);
    // a->mode = FILE_MODE_BLK;
    // a->f_op = dev_table[3].f_op;
    // printk("%x\n", init);while(1){}
    while(1){}
    restart();
}

void init()
{
    // mount("/dev/hd1", "/root", "fat12");

    mount("/dev/hd1", "/root", "fat12");
    (void)open("/dev/tty0", O_RDWR);
    (void) dup(0);
    (void) dup(0);
    printf("parent is running, %d\n", getpid());
    int i, pid;
    pid = fork();
    if (pid!=0)
    {
        printf("parent is running, child pid: %d\n", pid);
        char a[513];
        // int ff = open("/root/d.txt", O_RDWR);
        // memset(a, 0, 513);
        while(1){
            // delay(1);
            // lseek(ff, 5, SEEK_SET);
            // int aaa = read(ff, a, 2);
            // printf("(pid:%d) read_len:%d\ncontent:%s\n===\n", 0, aaa, a);
        }
    } else {
        pid = getpid();
        execve("/root/HELLO1" ,NULL, NULL);
        // while(1){}
        // while(1){
        //     printf("child is running, pid: 1\n", pid);
        //     delay(1);
        // }
        printf("child is running, pid: %d\n", pid);
        char a[513];
        int ff = open("/root/d.txt", O_RDWR);
        memset(a, 0, 513);
        printf("%d\n", ff);
        int aaa=read(ff, a, 2);
        printf("(pid:%d) read_len:%d\ncontent:%s\n===\n", pid, aaa, a);
        close(ff);

        ff = open("/root/b/c.txt", O_RDWR);
        memset(a, 0, 513);
        aaa=read(ff, a, 2);
        printf("%d\n%s\n", aaa, a);
        while(1){
            aaa=read(ff, a, 2);
        printf("(pid:%d) read_len:%d\ncontent:%s\n===\n", pid, aaa, a);
        delay(1);
            // int ff = open("/root/d.txt", O_RDWR);
            // char *a[512];
            // memset(a, 0, 513);
            // int aaa=read(ff, a, 2);
            // printf("%d\n%s\n", aaa, a);
            // close(ff);
        }
    }

    while(1) {
        int i;
        pid_t child;
        child=wait(&i);
        // printf("Child (%d) exited with status: %d\n", child, i);
    }
}

void TestA()
{
    unsigned int i = 0;
    // hd_setup();
    // init_fat12();
    // mount("/dev/hd1", "/root", "fat12");
    // init_ext2();
    // mount("/dev/hd1", "/root", "ext2");
    int stdin = open("/dev/tty0", O_RDWR);
    int stdout=dup(0);
    int errout=dup(0);
    // int stdout = open("/dev/tty0", O_RDWR);
    // int errout = open("/dev/tty0", O_RDWR);
    char a[513];
    // // hd_rw(0, 1, a, 0, sizeof(a));
    // memset(a, 4, 513);
    int hd = open("/dev/hd0", O_RDWR);
    // // write(hd, a, sizeof(a));
    // // memset(a, 5, 513);
    // // write(hd, a, sizeof(a));
    // // memset(a, 0, 513);
    lseek(hd, 512, SEEK_SET);
    read(hd, a, 3);
    printf("%d %d %d\n", a[0], a[1], a[2]);

    int ff = open("/root/d.txt", O_RDWR);
    memset(a, 0, 513);
    int aaa=read(ff, a, 2);
    printf("%d\n%s\n", aaa, a);
    close(ff);

    ff = open("/root/b/c.txt", O_RDWR);
    memset(a, 0, 513);
    aaa=read(ff, a, 2);
    printf("%d\n%s\n", aaa, a);
    // hd_open(0);
    // hd_rw(0, 1, "abc132", 0, sizeof("abc132"));
    while (1)
    {
        // out_byte(INT_M_CTLMASK,0xF0);
        // printf("1111111111111111111111111111111");
        // printf("%s.", "A31231\n");
        // char a = in_byte(INT_M_CTLMASK);
    //     char *buf;
    // int j=sprintf(buf, "%s%x.", "A", get_ticks());
    // tty_write(&tty, buf, j);
        // printf("%s%x.%d)%x:", "A", get_ticks(), is_in_ring0, &is_in_ring0);
        // printf("%x", disp_pos);
        // printf("%s%x.", "A", get_ticks());
        // printf("%s%x.", "A", i++);
        // DispStr("A");
        // disp_int(get_tcks());
        // DispStr(".");
        // hd_rw(0, 1, a, 0, sizeof(a));
        delay(1);
        // milli_delay(1000);
        // char a = in_byte(INT_M_CTLMASK);
        // // printf("%d", a);
        // if (a == 0xFF)
        // {
        //     printf("|");
        // } else {
        //     printf("+");
        // }
    }
}

void delay(int time)
{
    int i, j, k;
    for (int k = 0; k < time; k++)
    {
        for (int i = 0; i < 10;i++) {
            for (int j = 0; j < 10000; j++){}
        }
    }
}

void calltest()
{
    int stdin = open("/dev/tty0", O_RDWR);
    int stdout = open("/dev/tty0", O_RDWR);
    int errout = open("/dev/tty0", O_RDWR);
    // DispStr("i'm calltest\n");
    char *buf = "ABCD1234";
    unsigned int i = 0;
    // struct request r;
    // r.buffer = buf;
    // r.cmd = 1;
    // r.dev = 0;
    // r.nr_sectors = 1;
    // r.sector = 2;
    // make_request(&r);
    // hd_open(0);
    // hd_rw(0, 1, buf, 2, sizeof(buf));
    while (1)
    {
        // DispStr("B");
        // disp_int(i++);
        // DispStr(".");
        // printf("%c%x.", 'B', i++);

    // //     char *buf;
    // int j=sprintf(buf, "%c%x.", 'B', i++);
    // // tty_write(&tty, buf, j);
        delay(1);
        // hd_rw(0, 1, buf, 1, strlen(buf));
        // milli_delay(1000);
    }

    char output[2] = {'\0', '\0'};
    unsigned int key=0;

    // floppy_init();
    unsigned char *a="123456";
    // floppy_motor_on();
    // DispStr("\n");
    // disp_int((long)a);
    // FloppyReadSector(0, a);
    // DispStr("\n");
    // disp_int((long)a);

    while (1)
    {
        // printf("%c%x.", 'B', i++);
        // delay(1);
        // key=keyboard_read(&tty);
        
        if (!(key & FLAG_EXT))
        {
            //     tty_input(&tty, key);
            // tty_output(&tty);
            // output[0] = key & 0xff;
            output[0] = '*';
            // DispStr(output);
            // disp_int(key);
            printf("B");
            // tty_write(&tty, output, 1);
        }
        // else
        // {
        //     int raw_code=key&MASK_RAW;
        //     switch (raw_code) {
        //         case UP:
        //             if ((key&FLAG_SHIFT_L) || (key&FLAG_SHIFT_R)) {
        //                 scroll_screen(tty.console, SCR_UP);
        //             }
        //             break;
        //         case DOWN:
        //             if ((key&FLAG_SHIFT_L) || (key&FLAG_SHIFT_R)) {
        //                 scroll_screen(tty.console, SCR_DN);
        //             }
        //             break;
        //     }
        // }
        // tty_input(&tty, key);
    }
}

void clock_handler(int irq)
{
    // DispStr("#");
    // printf("#")
    char a[2] = {'#', '\0'};
    // char *buf;
    // int i=sprintf(buf, "#");
    // tty_write(0, a, 1);
    if (++ticks >= MAX_TICKS)
    {
        ticks = 0;
    }
    if (is_in_ring0 != 0)
    {
        // a[0]='1';
        // // i=sprintf(buf, "!");
        // tty_write(&tty, a, 1);
        return;
    }
    
    // disp_int((int)current_process);
    do {
        current_process++;
    
    // if (disp_pos>80*25) {
    //     disp_pos = 0;
    // }
        if (current_process >= process_table + PROC_NUMBER)
        {
            current_process = process_table;
        }
    } while (current_process->is_free != 1);
    // do {
    //     current_process++;
    //     if (current_process >= process_table + PROC_NUMBER)
    //     {
    //         current_process = process_table;
    //     }
    // } while (current_process->status != 0);
}

void milli_delay(int mill_sec)
{
    int t = get_ticks();
    while((get_ticks()-t)*1000/100){}
}

void task_tty()
{
    unsigned int key = 0;
    while (1)
    {
        keyboard_read(current_tty);
        tty_output(current_tty);
    }
}