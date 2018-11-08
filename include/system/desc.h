#ifndef _SYSTEM_DESC_H_
#define _SYSTEM_DESC_H_

#define GDT_SIZE 128
#define IDT_SIZE 256

/* 描述符类型值说明 */
#define	DA_32			0x4000	/* 32 位段				*/
#define	DA_LIMIT_4K		0x8000	/* 段界限粒度为 4K 字节			*/
#define	DA_DPL0			0x00	/* DPL = 0				*/
#define	DA_DPL1			0x20	/* DPL = 1				*/
#define	DA_DPL2			0x40	/* DPL = 2				*/
#define	DA_DPL3			0x60	/* DPL = 3				*/
/* 存储段描述符类型值说明 */
#define	DA_DR			0x90	/* 存在的只读数据段类型值		*/
#define	DA_DRW			0x92	/* 存在的可读写数据段属性值		*/
#define	DA_DRWA			0x93	/* 存在的已访问可读写数据段类型值	*/
#define	DA_C			0x98	/* 存在的只执行代码段属性值		*/
#define	DA_CR			0x9A	/* 存在的可执行可读代码段属性值		*/
#define	DA_CCO			0x9C	/* 存在的只执行一致代码段属性值		*/
#define	DA_CCOR			0x9E	/* 存在的可执行可读一致代码段属性值	*/
/* 系统段描述符类型值说明 */
#define	DA_LDT			0x82	/* 局部描述符表段类型值			*/
#define	DA_TaskGate		0x85	/* 任务门类型值				*/
#define	DA_386TSS		0x89	/* 可用 386 任务状态段类型值		*/
#define	DA_386CGate		0x8C	/* 386 调用门类型值			*/
#define	DA_386IGate		0x8E	/* 386 中断门类型值			*/
#define	DA_386TGate		0x8F	/* 386 陷阱门类型值			*/

/* 权限 */
#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3

/* 选择子类型值说明 */
/* 其中, SA_ : Selector Attribute */
#define	SA_RPL_MASK	0xFFFC
#define	SA_RPL0		0
#define	SA_RPL1		1
#define	SA_RPL2		2
#define	SA_RPL3		3

#define	SA_TI_MASK	0xFFFB
#define	SA_TIG		0
#define	SA_TIL		4

typedef struct s_selector
{
    unsigned short value;
} SELECTOR;

/* 存储段描述符/系统段描述符 */
typedef struct s_decriptor
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_mid;
    unsigned char attr1;
    unsigned char limit_high_attr2;
    unsigned char base_high;
}DESCRIPTOR;

/* 门描述符 */
typedef struct s_gate
{
	unsigned short offset_low;	/* Offset Low */
	unsigned short selector;	/* Selector */
	unsigned char dcount;		/* 该字段只在调用门描述符中有效。如果在利用
				   调用门调用子程序时引起特权级的转换和堆栈
				   的改变，需要将外层堆栈中的参数复制到内层
				   堆栈。该双字计数字段就是用于说明这种情况
				   发生时，要复制的双字参数的数量。*/
	unsigned char attr;		/* P(1) DPL(2) DT(1) TYPE(4) */
	unsigned short offset_high;	/* Offset High */
}GATE;

typedef struct s_tss {
	unsigned int	backlink;
	unsigned int	esp0;		/* stack pointer to use during interrupt */
	unsigned int	ss0;		/*   "   segment  "  "    "        "     */
	unsigned int	esp1;
	unsigned int	ss1;
	unsigned int	esp2;
	unsigned int	ss2;
	unsigned int	cr3;
	unsigned int	eip;
	unsigned int	flags;
	unsigned int	eax;
	unsigned int	ecx;
	unsigned int	edx;
	unsigned int	ebx;
	unsigned int	esp;
	unsigned int	ebp;
	unsigned int	esi;
	unsigned int	edi;
	unsigned int	es;
	unsigned int	cs;
	unsigned int	ss;
	unsigned int	ds;
	unsigned int	fs;
	unsigned int	gs;
	unsigned int	ldt;
	unsigned short	trap;
	unsigned short	iobase;	/* I/O位图基址大于或等于TSS段界限，就表示没有I/O许可位图 */
    unsigned char iomap;
    /*u8	iomap[2];*/
}TSS;

DESCRIPTOR create_descriptor(unsigned int base, unsigned int limit, unsigned short attr);
unsigned short insert_descriptor(DESCRIPTOR *gdt, unsigned int index, DESCRIPTOR desc, unsigned short attr);
GATE create_gate(unsigned short selector, unsigned int limit, unsigned char dcount, unsigned short attr);
DESCRIPTOR gate_to_descriptor(GATE gate);

#endif