#include "kernel.h"
#include "asm_global.h"
#include "interrupt.h"

// unsigned char gdt_ptr[6];
// DESCRIPTOR gdt[GDT_SIZE];
unsigned short SelectorTss;

static void init_idt();
static void init_gdt();

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
    DispStr(
    "-----Move Gdt success-------\n\n"
    
    );
    init_gdt();
    init_idt();
    DispStr("----Init idt success----\n\n");
    DispStr("----JayOS----\n\n");
}

void init_idt()
{
	// 全部初始化成中断门(没有陷阱门)
	init_idt_desc(idt, INT_VECTOR_DIVIDE,	DA_386IGate,
		      divide_error,		PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_DEBUG,		DA_386IGate,
		      single_step_exception,	PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_NMI,		DA_386IGate,
		      nmi,			PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_BREAKPOINT,	DA_386IGate,
		      breakpoint_exception,	PRIVILEGE_USER);

	init_idt_desc(idt, INT_VECTOR_OVERFLOW,	DA_386IGate,
		      overflow,			PRIVILEGE_USER);

	init_idt_desc(idt, INT_VECTOR_BOUNDS,	DA_386IGate,
		      bounds_check,		PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_INVAL_OP,	DA_386IGate,
		      inval_opcode,		PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_COPROC_NOT,	DA_386IGate,
		      copr_not_available,	PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_DOUBLE_FAULT,	DA_386IGate,
		      double_fault,		PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_COPROC_SEG,	DA_386IGate,
		      copr_seg_overrun,		PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_INVAL_TSS,	DA_386IGate,
		      inval_tss,		PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_SEG_NOT,	DA_386IGate,
		      segment_not_present,	PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_STACK_FAULT,	DA_386IGate,
		      stack_exception,		PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_PROTECTION,	DA_386IGate,
		      general_protection,	PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_PAGE_FAULT,	DA_386IGate,
		      page_fault,		PRIVILEGE_KRNL);

	init_idt_desc(idt, INT_VECTOR_COPROC_ERR,	DA_386IGate,
		      copr_error,		PRIVILEGE_KRNL);
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
    unsigned short selector = (index * 0x8) << 3;
    selector |= attr;
    return selector;
}

static void init_gdt()
{
    DESCRIPTOR tss_desc=create_descriptor(0, sizeof(TSS)-1, DA_386TSS);
    SelectorTss=insert_descriptor(gdt, 6, tss_desc, PRIVILEGE_KRNL);
}
