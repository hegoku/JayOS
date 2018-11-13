#include "global.h"
#include "interrupt.h"
#include "keyboard.h"
#include "fd.h"
// #include "hd.h"
#include "tty.h"
#include "process.h"
#include "system/desc.h"
#include "kernel.h"
#include "unistd.h"
#include "stdio.h"

// unsigned char gdt_ptr[6];
// DESCRIPTOR gdt[GDT_SIZE];

unsigned short testcallS;
unsigned short ss3;

TSS tss;
irq_handler irq_table[IRQ_NUMBER];
sys_call_handler sys_call_table[SYS_CALL_NUMBER];
unsigned short SelectorKernelCs;
unsigned short SelectorKernelDs;
unsigned short SelectorVideo;
unsigned short SelectorUserCs;
unsigned short SelectorUserDs;
unsigned short SelectorTss;

static void init_idt();
static void init_gdt();
void kernel_main();

void clock_handler(int irq);
int sys_get_ticks();
int get_ticks();
ssize_t sys_write(int fildes, const void *buf, unsigned int nbyte);

TTY tty;

PROCESS process_table[PROC_NUMBER];
PROCESS *p_proc_ready;

void calltest();
void TestA();
void delay();
void milli_delay(int mill_sec);
void task_tty();

void restart();

static void moveGdt()
{
    // memcpy(&gdt,
    //     (void*)(*((unsigned int*) (&gdt_ptr[2]))),
    //     *((unsigned short*) (&gdt_ptr[0])) +1
    // );

    // unsigned short *p_gdt_limit = (unsigned short *)(&gdt_ptr[0]);
    // unsigned int *p_gdt_base = (unsigned int *)(&gdt_ptr[2]);
    // *p_gdt_limit = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
    // *p_gdt_base = (unsigned int)&gdt;

    // DispStr("-----Move Gdt success-------\n"
    // "----JayOS----\n"
    // );
}

void cstart()
{
    // DispStr("-----Move Gdt success-------\n\n");
    init_gdt();
    init_idt();
    keyboard_init();
    // init_hd();
    // hd_open(0);

    tty = tty_create(0);
    // DispStr("----Init idt success----\n\n");
    // DispStr("----JayOS----\n\n");

    process_table[0].pid = 0;
    create_process(gdt, &process_table[0], (unsigned int)TestA);
    process_table[0].regs.esp = TOP_OF_USER_STACK;

    process_table[1].pid = 1;
    create_process(gdt, &process_table[1], (unsigned int)calltest);
    process_table[1].regs.esp = TOP_OF_USER_STACK-0x400;

    process_table[2].pid = 2;
    create_process(gdt, &process_table[2], (unsigned int)task_tty);
    process_table[2].regs.esp = TOP_OF_USER_STACK-0x400*2;
}

void init_idt()
{
    init_8259A();

    for (int i; i < IRQ_NUMBER; i++) {
        irq_table[i] = spurious_irq;
    }

    sys_call_table[0] = sys_get_ticks;
    sys_call_table[1] = sys_write;

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
    init_idt_desc(idt, AT_WINI_IRQ, DA_386IGate, hwint14, PRIVILEGE_KRNL); //硬盘
}

static void init_gdt()
{
	DESCRIPTOR gdt_0 = create_descriptor(0, 0, 0);
	insert_descriptor(gdt, 0, gdt_0, PRIVILEGE_KRNL);

	DESCRIPTOR kernel_cs = create_descriptor(0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL0);
	SelectorKernelCs=insert_descriptor(gdt, 1, kernel_cs, PRIVILEGE_KRNL);

	DESCRIPTOR kernel_ds = create_descriptor(0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL0);
	SelectorKernelDs=insert_descriptor(gdt, 2, kernel_ds, PRIVILEGE_KRNL);

	DESCRIPTOR video = create_descriptor(0xB8000, 0xBFFFF, DA_DRW | DA_32 | DA_DPL3);
	SelectorVideo=insert_descriptor(gdt, 3, video, PRIVILEGE_USER);

	DESCRIPTOR user_cs = create_descriptor(0, 0xfffff, DA_CR | DA_32 | DA_LIMIT_4K | DA_DPL3);
	SelectorUserCs=insert_descriptor(gdt, 4, user_cs, PRIVILEGE_USER);

	DESCRIPTOR user_ds = create_descriptor(0, 0xfffff, DA_DRW | DA_32 | DA_LIMIT_4K | DA_DPL3);
	SelectorUserDs=insert_descriptor(gdt, 5, user_ds, PRIVILEGE_USER);

	tss.esp0 = TOP_OF_KERNEL_STACK;
	tss.ss0 = SelectorKernelDs;
    // disp_int((unsigned int)&tss);while(1){}
    DESCRIPTOR tss_desc = create_descriptor((unsigned int)&tss, sizeof(TSS) - 1, DA_386TSS);
    SelectorTss=insert_descriptor(gdt, 6, tss_desc, PRIVILEGE_KRNL);
	
    // GATE call_test = create_gate(SelectorKernelCs, (unsigned int)&calltest, 0, DA_386CGate | DA_DPL3);
    // testcallS=insert_descriptor(gdt, 7, gate_to_descriptor(call_test), PRIVILEGE_KRNL);

    // DESCRIPTOR s3 = create_descriptor(0, TOP_OF_KERNEL_STACK, DA_DRWA|DA_32|DA_DPL0);
	// ss3=insert_descriptor(gdt, 8, s3, PRIVILEGE_KRNL);
}

void kernel_main()
{
    // DispStr("i'm kernel_main\n");
    ticks = 0;
    irq_table[CLOCK_IRQ] = clock_handler;
    enable_irq(CLOCK_IRQ);

    out_byte(0x43, 0x34);
    out_byte(0x40, (unsigned char)(1193182L / 100));
    out_byte(0x40, (unsigned char)((1193182L / 100)>>8));

    irq_table[KEYBOARD_IRQ] = keyboard_handler;
    enable_irq(KEYBOARD_IRQ);

    is_in_int = 0;
    p_proc_ready = process_table;
    restart();
}

void TestA()
{
    unsigned int i = 0;
    while (1)
    {
        // out_byte(INT_M_CTLMASK,0xF0);
        // printf("1111111111111111111111111111111");
        // printf("%s.", "A31231\n");
        // char a = in_byte(INT_M_CTLMASK);
    //     char *buf;
    // int j=sprintf(buf, "%s%x.", "A", get_ticks());
    // tty_write(&tty, buf, j);
        // printf("%s%x.%d)%x:", "A", get_ticks(), is_in_int, &is_in_int);
        // printf("%x", disp_pos);
        // printf("%s%x.", "A", get_ticks());
        printf("%s%x.", "A", i++);
        // DispStr("A");
        // disp_int(get_tcks());
        // DispStr(".");
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
    // DispStr("i'm calltest\n");
    unsigned int i = 0;
    while (1)
    {
        // DispStr("B");
        // disp_int(i++);
        // DispStr(".");
        printf("%c%x.", 'B', i++);

    // //     char *buf;
    // int j=sprintf(buf, "%c%x.", 'B', i++);
    // // tty_write(&tty, buf, j);
        delay(1);
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
    // tty_write(&tty, a, 1);
    if (++ticks>=MAX_TICKS) {
        ticks = 0;
    }
    if (is_in_int!=0) {
        // a[0]='1';
        // // i=sprintf(buf, "!");
        // tty_write(&tty, a, 1);
        return;
    }
    
    // disp_int((int)p_proc_ready);
    p_proc_ready++;
    // disp_int(p_proc_ready);
    // if (disp_pos>80*25) {
    //     disp_pos = 0;
    // }
    if (p_proc_ready >= process_table + PROC_NUMBER)
    {
        p_proc_ready = process_table;
    }
}

int sys_get_ticks()
{
    // DispStr("\nAAAAA:");
    // disp_int(a);
    // DispStr("   BBBBB:");
    // disp_int(b);
    // DispStr("\n");
    return ticks;
}

int get_ticks()
{
    return sys_call_0_param(0);
}

ssize_t sys_write(int fildes, const void *buf, unsigned int nbyte)
{
    return tty_write(&tty, (char *)buf, nbyte);
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
        keyboard_read(&tty);
        tty_output(&tty);
    }
}