#include <system/desc.h>

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