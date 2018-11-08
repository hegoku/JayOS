#include "global.h"
#include "interrupt.h"
#include "keyboard.h"
#include "fd.h"
// #include "hd.h"
#include "tty.h"
#include "process.h"
#include "system/desc.h"
#include "kernel.h"

// unsigned char gdt_ptr[6];
// DESCRIPTOR gdt[GDT_SIZE];

unsigned short testcallS;
unsigned short ss3;
TSS tss;

static void init_idt();
static void init_gdt();
void kernel_main();
int is_in_int;
void clock_handler(int irq);

TTY tty;

PROCESS process_table[PROC_NUMBER];
PROCESS *p_proc_ready;

void calltest();
void TestA();
void delay();

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
}

void init_idt()
{
    init_8259A();

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

	DESCRIPTOR video = create_descriptor(0xB8000, 0xfffff, DA_DRW | DA_32 | DA_DPL3);
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
    is_in_int = 0;
    p_proc_ready = process_table;
    restart();
}

void TestA()
{
    int i = 0;
    while (1)
    {
        DispStr("A");
        disp_int(i++);
        DispStr(".");
        delay(1);
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
    DispStr("i'm calltest\n");
    int i = 0;
    while (1)
    {
        DispStr("B");
        disp_int(i++);
        DispStr(".");
        delay(1);
    }

    char output[2] = {'\0', '\0'};
    int key;

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
        key = keyboard_read();
        if (!(key & FLAG_EXT)) {
        //     tty_input(&tty, key);
        // tty_output(&tty);
            output[0] = key & 0xff;
            DispStr(output);
        } else if(key==ENTER) {
            DispStr("\n");
        } else {
            int raw_code=key&MASK_RAW;
            switch (raw_code) {
                case UP:
                    if ((key&FLAG_SHIFT_L) || (key&FLAG_SHIFT_R)) {
                        asm("cli");
                        out_byte(0x3d4, 0xc);
                        out_byte(0x3d5, ((80*15)>>8)&0xff);
                        out_byte(0x3d4, 0xd);
                        out_byte(0x3d5, (80*15)&0xff);
                        asm("sti");
                    }
                    break;
                case DOWN:
                    if ((key&FLAG_SHIFT_L) || (key&FLAG_SHIFT_R)) {
                        asm("cli");
                        out_byte(0x3d4, 0xc);
                        out_byte(0x3d5, 0&0xff);
                        out_byte(0x3d4, 0xd);
                        out_byte(0x3d5, 0&0xff);
                        asm("sti");
                    }
                    break;
            }
        }
        // tty_input(&tty, key);
    }
}

void clock_handler(int irq)
{
    DispStr("#");
    p_proc_ready++;
    if (p_proc_ready>=process_table+PROC_NUMBER) {
        p_proc_ready = process_table;
    }
}
