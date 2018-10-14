#include "kernel.h"
#include "global.h"
#include "interrupt.h"
#include "keyboard.h"

// unsigned char gdt_ptr[6];
// DESCRIPTOR gdt[GDT_SIZE];
unsigned short SelectorKernelCs;
unsigned short SelectorKernelDs;
unsigned short SelectorVideo;
unsigned short SelectorUserCs;
unsigned short SelectorUserDs;
unsigned short SelectorTss;
unsigned short testcallS;
unsigned short ss3;
TSS tss;

static void init_idt();
static void init_gdt();

void calltest();

static void moveGdt()
{
    // MemCpy(&gdt,
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
    init_keyboard();
    // DispStr("----Init idt success----\n\n");
    // DispStr("----JayOS----\n\n");
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

    init_idt_desc(idt, INT_VECTOR_IRQ0 + 1, DA_386IGate, hwint02, PRIVILEGE_KRNL);
}

DESCRIPTOR create_descriptor(unsigned int base, unsigned int limit, unsigned short attr)
{
    DESCRIPTOR desc;

    desc.limit_low	= limit & 0x0FFFF;		/* 段界限 1		(2 字节) */
	desc.base_low	= base & 0x0FFFF;		/* 段基址 1		(2 字节) */
	desc.base_mid	= (base >> 16) & 0x0FF;		/* 段基址 2		(1 字节) */
	desc.attr1		= attr & 0xFF;		/* 属性 1 */
	desc.limit_high_attr2= ((limit >> 16) & 0x0F) |
				  ((attr >> 8) & 0xF0);	/* 段界限 2 + 属性 2 */
	desc.base_high	= (base >> 24) & 0x0FF;		/* 段基址 3		(1 字节) */
    return desc;
}

unsigned short insert_descriptor(DESCRIPTOR *gdt, unsigned int index, DESCRIPTOR desc, unsigned short attr)
{
    gdt[index] = desc;
    unsigned short selector = (index * 0x8);
    selector |= attr;
    return selector;
}

GATE create_gate(unsigned short selector, unsigned int limit, unsigned char dcount, unsigned short attr)
{
	GATE gate;
	gate.offset_low	= limit & 0xFFFF;
	gate.selector	= selector;
	gate.dcount		= dcount;
	gate.attr		= attr;
	gate.offset_high	= (limit >> 16) & 0xFFFF;
    return gate;
}

DESCRIPTOR gate_to_descriptor(GATE gate)
{
    DESCRIPTOR desc;
    desc.limit_low = gate.offset_low;
    desc.base_low = gate.selector;
    desc.base_mid = gate.dcount;
    desc.attr1 = gate.attr;
    desc.limit_high_attr2 = gate.offset_high & 0xff;
    desc.base_high = (gate.offset_high >> 8) & 0x0FF;
    return desc;
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
    DESCRIPTOR tss_desc = create_descriptor((unsigned int)&tss, sizeof(TSS) - 1, DA_386TSS);
    SelectorTss=insert_descriptor(gdt, 6, tss_desc, PRIVILEGE_KRNL);
	
    GATE call_test = create_gate(SelectorKernelCs, (unsigned int)&calltest, 0, DA_386CGate | DA_DPL3);
    testcallS=insert_descriptor(gdt, 7, gate_to_descriptor(call_test), PRIVILEGE_KRNL);

    // DESCRIPTOR s3 = create_descriptor(0, TOP_OF_KERNEL_STACK, DA_DRWA|DA_32|DA_DPL0);
	// ss3=insert_descriptor(gdt, 8, s3, PRIVILEGE_KRNL);
}

void calltest()
{
    DispStr("i'm calltest\n");
    char output[2] = {'\0', '\0'};
    int key;
    while (1)
    {
        key = keyboard_read();
        if (!(key & FLAG_EXT)) {
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
    }
}

// void keyboard_handler(int irq)
// {
//     unsigned char scan_code = in_byte(0x60);
//     disp_int(scan_code);
// }