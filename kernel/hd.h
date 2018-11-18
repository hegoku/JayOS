#ifndef	_HD_H_
#define	_HD_H_

#include "process.h"
#include <system/dev.h>
/**
 * @struct part_ent
 * @brief  Partition Entry struct.
 *
 * <b>Master Boot Record (MBR):</b>
 *   Located at offset 0x1BE in the 1st sector of a disk. MBR contains
 *   four 16-byte partition entries. Should end with 55h & AAh.
 *
 * <b>partitions in MBR:</b>
 *   A PC hard disk can contain either as many as four primary partitions,
 *   or 1-3 primaries and a single extended partition. Each of these
 *   partitions are described by a 16-byte entry in the Partition Table
 *   which is located in the Master Boot Record.
 *
 * <b>extented partition:</b>
 *   It is essentially a link list with many tricks. See
 *   http://en.wikipedia.org/wiki/Extended_boot_record for details.
 */
struct part_ent {
	unsigned char boot_ind;		/**
				 * boot indicator
				 *   Bit 7 is the active partition flag,
				 *   bits 6-0 are zero (when not zero this
				 *   byte is also the drive number of the
				 *   drive to boot so the active partition
				 *   is always found on drive 80H, the first
				 *   hard disk).
				 */

	unsigned char start_head;		/**
				 * Starting Head
				 */

	unsigned char start_sector;	/**
				 * Starting Sector.
				 *   Only bits 0-5 are used. Bits 6-7 are
				 *   the upper two bits for the Starting
				 *   Cylinder field.
				 */

	unsigned char start_cyl;		/**
				 * Starting Cylinder.
				 *   This field contains the lower 8 bits
				 *   of the cylinder value. Starting cylinder
				 *   is thus a 10-bit number, with a maximum
				 *   value of 1023.
				 */

	unsigned char sys_id;		/**
				 * System ID
				 * e.g.
				 *   01: FAT12
				 *   81: MINIX
				 *   83: Linux
				 */

	unsigned char end_head;		/**
				 * Ending Head
				 */

	unsigned char end_sector;		/**
				 * Ending Sector.
				 *   Only bits 0-5 are used. Bits 6-7 are
				 *   the upper two bits for the Ending
				 *    Cylinder field.
				 */

	unsigned char end_cyl;		/**
				 * Ending Cylinder.
				 *   This field contains the lower 8 bits
				 *   of the cylinder value. Ending cylinder
				 *   is thus a 10-bit number, with a maximum
				 *   value of 1023.
				 */

	unsigned long start_sect;	/**
				 * starting sector counting from
				 * 0 / Relative Sector. / start in LBA
				 */

	unsigned long nr_sects;		/**
				 * nr of sectors in partition
				 */

} PARTITION_ENTRY;


/********************************************/
/* I/O Ports used by hard disk controllers. */
/********************************************/
/* slave disk not supported yet, all master registers below */

/* Command Block Registers */
/*	MACRO		PORT			DESCRIPTION			INPUT/OUTPUT	*/
/*	-----		----			-----------			------------	*/
#define REG_DATA	0x1F0		/*	Data				I/O		*/
#define REG_FEATURES	0x1F1		/*	Features			O		*/
#define REG_ERROR	REG_FEATURES	/*	Error				I		*/
					/* 	The contents of this register are valid only when the error bit
						(ERR) in the Status Register is set, except at drive power-up or at the
						completion of the drive's internal diagnostics, when the register
						contains a status code.
						When the error bit (ERR) is set, Error Register bits are interpreted as such:
						|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
						+-----+-----+-----+-----+-----+-----+-----+-----+
						| BRK | UNC |     | IDNF|     | ABRT|TKONF| AMNF|
						+-----+-----+-----+-----+-----+-----+-----+-----+
						   |     |     |     |     |     |     |     |
						   |     |     |     |     |     |     |     `--- 0. Data address mark not found after correct ID field found
						   |     |     |     |     |     |     `--------- 1. Track 0 not found during execution of Recalibrate command
						   |     |     |     |     |     `--------------- 2. Command aborted due to drive status error or invalid command
						   |     |     |     |     `--------------------- 3. Not used
						   |     |     |     `--------------------------- 4. Requested sector's ID field not found
						   |     |     `--------------------------------- 5. Not used
						   |     `--------------------------------------- 6. Uncorrectable data error encountered
						   `--------------------------------------------- 7. Bad block mark detected in the requested sector's ID field
					*/
#define REG_NSECTOR	0x1F2		/*	Sector Count			I/O		*/
#define REG_LBA_LOW	0x1F3		/*	Sector Number / LBA Bits 0-7	I/O		*/
#define REG_LBA_MID	0x1F4		/*	Cylinder Low / LBA Bits 8-15	I/O		*/
#define REG_LBA_HIGH	0x1F5		/*	Cylinder High / LBA Bits 16-23	I/O		*/
#define REG_DEVICE	0x1F6		/*	Drive | Head | LBA bits 24-27	I/O		*/
					/*	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
						+-----+-----+-----+-----+-----+-----+-----+-----+
						|  1  |  L  |  1  | DRV | HS3 | HS2 | HS1 | HS0 |
						+-----+-----+-----+-----+-----+-----+-----+-----+
						         |           |   \_____________________/
						         |           |              |
						         |           |              `------------ If L=0, Head Select.
						         |           |                                    These four bits select the head number.
						         |           |                                    HS0 is the least significant.
						         |           |                            If L=1, HS0 through HS3 contain bit 24-27 of the LBA.
						         |           `--------------------------- Drive. When DRV=0, drive 0 (master) is selected. 
						         |                                               When DRV=1, drive 1 (slave) is selected.
						         `--------------------------------------- LBA mode. This bit selects the mode of operation.
					 	                                                            When L=0, addressing is by 'CHS' mode.
					 	                                                            When L=1, addressing is by 'LBA' mode.
					*/
#define REG_STATUS	0x1F7		/*	Status				I		*/
					/* 	Any pending interrupt is cleared whenever this register is read.
						|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
						+-----+-----+-----+-----+-----+-----+-----+-----+
						| BSY | DRDY|DF/SE|  #  | DRQ |     |     | ERR |
						+-----+-----+-----+-----+-----+-----+-----+-----+
						   |     |     |     |     |     |     |     |
						   |     |     |     |     |     |     |     `--- 0. Error.(an error occurred)
						   |     |     |     |     |     |     `--------- 1. Obsolete.
						   |     |     |     |     |     `--------------- 2. Obsolete.
						   |     |     |     |     `--------------------- 3. Data Request. (ready to transfer data)
						   |     |     |     `--------------------------- 4. Command dependent. (formerly DSC bit)
						   |     |     `--------------------------------- 5. Device Fault / Stream Error.
						   |     `--------------------------------------- 6. Drive Ready.
						   `--------------------------------------------- 7. Busy. If BSY=1, no other bits in the register are valid.
					*/
#define	STATUS_BSY	0x80
#define	STATUS_DRDY	0x40
#define	STATUS_DFSE	0x20
#define	STATUS_DSC	0x10
#define	STATUS_DRQ	0x08
#define	STATUS_CORR	0x04
#define	STATUS_IDX	0x02
#define	STATUS_ERR	0x01

#define REG_CMD		REG_STATUS	/*	Command				O		*/
					/*
						+--------+---------------------------------+-----------------+
						| Command| Command Description             | Parameters Used |
						| Code   |                                 | PC SC SN CY DH  |
						+--------+---------------------------------+-----------------+
						| ECh  @ | Identify Drive                  |             D   |
						| 91h    | Initialize Drive Parameters     |    V        V   |
						| 20h    | Read Sectors With Retry         |    V  V  V  V   |
						| E8h  @ | Write Buffer                    |             D   |
						+--------+---------------------------------+-----------------+
					
						KEY FOR SYMBOLS IN THE TABLE:
						===========================================-----=========================================================================
						PC    Register 1F1: Write Precompensation	@     These commands are optional and may not be supported by some drives.
						SC    Register 1F2: Sector Count		D     Only DRIVE parameter is valid, HEAD parameter is ignored.
						SN    Register 1F3: Sector Number		D+    Both drives execute this command regardless of the DRIVE parameter.
						CY    Register 1F4+1F5: Cylinder low + high	V     Indicates that the register contains a valid paramterer.
						DH    Register 1F6: Drive / Head
					*/

/* Control Block Registers */
/*	MACRO		PORT			DESCRIPTION			INPUT/OUTPUT	*/
/*	-----		----			-----------			------------	*/
#define REG_DEV_CTRL	0x3F6		/*	Device Control			O		*/
					/*	|  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
						+-----+-----+-----+-----+-----+-----+-----+-----+
						| HOB |  -  |  -  |  -  |  -  |SRST |-IEN |  0  |
						+-----+-----+-----+-----+-----+-----+-----+-----+
						   |                             |     |
						   |                             |     `--------- Interrupt Enable.
						   |                             |                  - IEN=0, and the drive is selected,
						   |                             |                    drive interrupts to the host will be enabled.
						   |                             |                  - IEN=1, or the drive is not selected,
						   |                             |                    drive interrupts to the host will be disabled.
						   |                             `--------------- Software Reset.
						   |                                                - The drive is held reset when RST=1.
						   |                                                  Setting RST=0 re-enables the drive.
						   |                                                - The host must set RST=1 and wait for at least
						   |                                                  5 microsecondsbefore setting RST=0, to ensure
						   |                                                  that the drive recognizes the reset.
						   `--------------------------------------------- HOB (High Order Byte)
						                                                    - defined by 48-bit Address feature set.
					*/
#define REG_ALT_STATUS	REG_DEV_CTRL	/*	Alternate Status		I		*/
					/*	This register contains the same information as the Status Register.
						The only difference is that reading this register does not imply interrupt acknowledge or clear a pending interrupt.
					*/

#define REG_DRV_ADDR	0x3F7		/*	Drive Address			I		*/

/* Hard Drive */
#define SECTOR_SIZE		512
#define SECTOR_BITS		(SECTOR_SIZE * 8)
#define SECTOR_SIZE_SHIFT	9

#define	MAX_DRIVES 4
#define	NR_PART_PER_DRIVE	4 //主分区数
#define	NR_SUB_PER_PART		16 //1个硬盘支持的总分区数-1, 因为第0个表示整个硬盘
#define	NR_SUB_PER_DRIVE	(NR_SUB_PER_PART * NR_PART_PER_DRIVE)
#define	NR_PRIM_PER_DRIVE	(NR_PART_PER_DRIVE + 1)

/* device numbers of hard disk */
#define	MINOR_hd1a		0x10
#define	MINOR_hd2a		(MINOR_hd1a+NR_SUB_PER_PART)

#define	ROOT_DEV		MAKE_DEV(DEV_HD, MINOR_BOOT)

#define	P_PRIMARY	0
#define	P_EXTENDED	1
#define P_LOGICAL 2

#define ORANGES_PART	0x99	/* Orange'S partition */
#define NO_PART		0x00	/* unused entry */
#define FAT12_PART		0x01	/* FAT entry */
#define EXT_PART	0x05	/* extended partition */
#define LINUX_PART 0x83

#define TIMER_FREQ     1193182L/* clock frequency for timer in PC and AT */
#define HZ             100  /* clock freq (software settable on IBM-PC) */

/**
 * @def MAX_PRIM
 * Defines the max minor number of the primary partitions.
 * If there are 2 disks, prim_dev ranges in hd[0-9], this macro will
 * equals 9.
 */
#define	MAX_PRIM		(MAX_DRIVES * NR_PRIM_PER_DRIVE - 1)

#define	MAX_SUBPARTITIONS	(NR_SUB_PER_DRIVE * MAX_DRIVES)

struct hd_identify_info_wCapabilities{
    unsigned short  reserved1:8;
    unsigned short  DMA:1;                  // 1=支持DMA
    unsigned short  LBA:1;                  // 1=支持LBA
    unsigned short  DisIORDY:1;             // 1=可不使用IORDY
    unsigned short  IORDY:1;                // 1=支持IORDY
    unsigned short  SoftReset:1;            // 1=需要ATA软启动
    unsigned short  Overlap:1;              // 1=支持重叠操作
    unsigned short  Queue:1;                // 1=支持命令队列
    unsigned short  InlDMA:1;               // 1=支持交叉存取DMA
};

// 硬盘identify命令得到的原始参数
struct hd_identify_info_raw
{
    unsigned short  wGenConfig;                 // WORD 0: 基本信息字
    unsigned short  wNumCyls;                   // WORD 1: 柱面数
    unsigned short  wReserved2;                 // WORD 2: 保留
    unsigned short  wNumHeads;                  // WORD 3: 磁头数
    unsigned short  wReserved4;                 // WORD 4: 保留
    unsigned short  wReserved5;                 // WORD 5: 保留
    unsigned short  wNumSectorsPerTrack;        // WORD 6: 每磁道扇区数
    unsigned short  wVendorUnique[3];           // WORD 7-9: 厂家设定值
    unsigned char   sSerialNumber[20];          // WORD 10-19:序列号
    unsigned short  wBufferType;                // WORD 20: 缓冲类型
    unsigned short  wBufferSize;                // WORD 21: 缓冲大小
    unsigned short  wECCSize;                   // WORD 22: ECC校验大小
    unsigned char   sFirmwareRev[8];            // WORD 23-26: 固件版本
    unsigned char   sModelNumber[40];           // WORD 27-46: 内部型号
    unsigned short  wMoreVendorUnique;          // WORD 47: 厂家设定值
    unsigned short  wReserved48;                // WORD 48: 保留
    struct hd_identify_info_wCapabilities wCapabilities;                    // WORD 49: 一般能力
    unsigned short  wReserved1;                 // WORD 50: 保留
    unsigned short  wPIOTiming;                 // WORD 51: PIO时序
    unsigned short  wDMATiming;                 // WORD 52: DMA时序
    struct {
        unsigned short  CHSNumber:1;            // 1=WORD 54-58有效
        unsigned short  CycleNumber:1;          // 1=WORD 64-70有效
        unsigned short  UnltraDMA:1;            // 1=WORD 88有效
        unsigned short  reserved:13;
    } wFieldValidity;                   // WORD 53: 后续字段有效性标志
    unsigned short  wNumCurCyls;                // WORD 54: CHS可寻址的柱面数
    unsigned short  wNumCurHeads;               // WORD 55: CHS可寻址的磁头数
    unsigned short  wNumCurSectorsPerTrack;     // WORD 56: CHS可寻址每磁道扇区数
    unsigned short  wCurSectorsLow;             // WORD 57: CHS可寻址的扇区数低位字
    unsigned short  wCurSectorsHigh;            // WORD 58: CHS可寻址的扇区数高位字
    struct {
        unsigned short  CurNumber:8;            // 当前一次性可读写扇区数
        unsigned short  Multi:1;                // 1=已选择多扇区读写
        unsigned short  reserved1:7;
    } wMultSectorStuff;                 // WORD 59: 多扇区读写设定
    unsigned int  dwTotalSectors;              // WORD 60-61: LBA可寻址的扇区数
    unsigned short  wSingleWordDMA;             // WORD 62: 单字节DMA支持能力
    struct {
        unsigned short  Mode0:1;                // 1=支持模式0 (4.17Mb/s)
        unsigned short  Mode1:1;                // 1=支持模式1 (13.3Mb/s)
        unsigned short  Mode2:1;                // 1=支持模式2 (16.7Mb/s)
        unsigned short  Reserved1:5;
        unsigned short  Mode0Sel:1;             // 1=已选择模式0
        unsigned short  Mode1Sel:1;             // 1=已选择模式1
        unsigned short  Mode2Sel:1;             // 1=已选择模式2
        unsigned short  Reserved2:5;
    } wMultiWordDMA;                    // WORD 63: 多字节DMA支持能力
    struct {
        unsigned short  AdvPOIModes:8;          // 支持高级POI模式数
        unsigned short  reserved:8;
    } wPIOCapacity;                     // WORD 64: 高级PIO支持能力
    unsigned short  wMinMultiWordDMACycle;      // WORD 65: 多字节DMA传输周期的最小值
    unsigned short  wRecMultiWordDMACycle;      // WORD 66: 多字节DMA传输周期的建议值
    unsigned short  wMinPIONoFlowCycle;         // WORD 67: 无流控制时PIO传输周期的最小值
    unsigned short  wMinPOIFlowCycle;           // WORD 68: 有流控制时PIO传输周期的最小值
    unsigned short  wReserved69[11];            // WORD 69-79: 保留
    struct {
        unsigned short  Reserved1:1;
        unsigned short  ATA1:1;                 // 1=支持ATA-1
        unsigned short  ATA2:1;                 // 1=支持ATA-2
        unsigned short  ATA3:1;                 // 1=支持ATA-3
        unsigned short  ATA4:1;                 // 1=支持ATA/ATAPI-4
        unsigned short  ATA5:1;                 // 1=支持ATA/ATAPI-5
        unsigned short  ATA6:1;                 // 1=支持ATA/ATAPI-6
        unsigned short  ATA7:1;                 // 1=支持ATA/ATAPI-7
        unsigned short  ATA8:1;                 // 1=支持ATA/ATAPI-8
        unsigned short  ATA9:1;                 // 1=支持ATA/ATAPI-9
        unsigned short  ATA10:1;                // 1=支持ATA/ATAPI-10
        unsigned short  ATA11:1;                // 1=支持ATA/ATAPI-11
        unsigned short  ATA12:1;                // 1=支持ATA/ATAPI-12
        unsigned short  ATA13:1;                // 1=支持ATA/ATAPI-13
        unsigned short  ATA14:1;                // 1=支持ATA/ATAPI-14
        unsigned short  Reserved2:1;
    } wMajorVersion;                    // WORD 80: 主版本
    unsigned short  wMinorVersion;              // WORD 81: 副版本
    unsigned short wReserved82[6]; // WORD 82-87: 保留
    struct {
        unsigned short  Mode0:1;                // 1=支持模式0 (16.7Mb/s)
        unsigned short  Mode1:1;                // 1=支持模式1 (25Mb/s)
        unsigned short  Mode2:1;                // 1=支持模式2 (33Mb/s)
        unsigned short  Mode3:1;                // 1=支持模式3 (44Mb/s)
        unsigned short  Mode4:1;                // 1=支持模式4 (66Mb/s)
        unsigned short  Mode5:1;                // 1=支持模式5 (100Mb/s)
        unsigned short  Mode6:1;                // 1=支持模式6 (133Mb/s)
        unsigned short  Mode7:1;                // 1=支持模式7 (166Mb/s) ???
        unsigned short  Mode0Sel:1;             // 1=已选择模式0
        unsigned short  Mode1Sel:1;             // 1=已选择模式1
        unsigned short  Mode2Sel:1;             // 1=已选择模式2
        unsigned short  Mode3Sel:1;             // 1=已选择模式3
        unsigned short  Mode4Sel:1;             // 1=已选择模式4
        unsigned short  Mode5Sel:1;             // 1=已选择模式5
        unsigned short  Mode6Sel:1;             // 1=已选择模式6
        unsigned short  Mode7Sel:1;             // 1=已选择模式7
    } wUltraDMA;                        // WORD 88:  Ultra DMA支持能力
    unsigned short    wReserved89[167];         // WORD 89-255
};

//硬盘identify命令原始参数处理后的格式，比如字符串的处理
struct hd_identify_info
{
    unsigned short  wGenConfig;                 // WORD 0: 基本信息字
    unsigned short  wNumCyls;                   // WORD 1: 柱面数
    unsigned short  wReserved2;                 // WORD 2: 保留
    unsigned short  wNumHeads;                  // WORD 3: 磁头数
    unsigned short  wReserved4;                 // WORD 4: 保留
    unsigned short  wReserved5;                 // WORD 5: 保留
    unsigned short  wNumSectorsPerTrack;        // WORD 6: 每磁道扇区数
    unsigned short  wVendorUnique[3];           // WORD 7-9: 厂家设定值
    unsigned char   sSerialNumber[20+1];          // WORD 10-19:序列号， 这里多1字节是为了存放字符串结束符\0
    unsigned short  wBufferType;                // WORD 20: 缓冲类型
    unsigned short  wBufferSize;                // WORD 21: 缓冲大小
    unsigned short  wECCSize;                   // WORD 22: ECC校验大小
    unsigned char   sFirmwareRev[8+1];            // WORD 23-26: 固件版本， 这里多1字节是为了存放字符串结束符\0
    unsigned char   sModelNumber[40+1];           // WORD 27-46: 内部型号， 这里多1字节是为了存放字符串结束符\0
    unsigned short  wMoreVendorUnique;          // WORD 47: 厂家设定值
    unsigned short  wReserved48;                // WORD 48: 保留
    struct hd_identify_info_wCapabilities wCapabilities;                    // WORD 49: 一般能力
    unsigned short  wReserved1;                 // WORD 50: 保留
    unsigned short  wPIOTiming;                 // WORD 51: PIO时序
    unsigned short  wDMATiming;                 // WORD 52: DMA时序
    struct {
        unsigned short  CHSNumber:1;            // 1=WORD 54-58有效
        unsigned short  CycleNumber:1;          // 1=WORD 64-70有效
        unsigned short  UnltraDMA:1;            // 1=WORD 88有效
        unsigned short  reserved:13;
    } wFieldValidity;                   // WORD 53: 后续字段有效性标志
    unsigned short  wNumCurCyls;                // WORD 54: CHS可寻址的柱面数
    unsigned short  wNumCurHeads;               // WORD 55: CHS可寻址的磁头数
    unsigned short  wNumCurSectorsPerTrack;     // WORD 56: CHS可寻址每磁道扇区数
    unsigned short  wCurSectorsLow;             // WORD 57: CHS可寻址的扇区数低位字
    unsigned short  wCurSectorsHigh;            // WORD 58: CHS可寻址的扇区数高位字
    struct {
        unsigned short  CurNumber:8;            // 当前一次性可读写扇区数
        unsigned short  Multi:1;                // 1=已选择多扇区读写
        unsigned short  reserved1:7;
    } wMultSectorStuff;                 // WORD 59: 多扇区读写设定
    unsigned int  dwTotalSectors;              // WORD 60-61: LBA可寻址的扇区数
    unsigned short  wSingleWordDMA;             // WORD 62: 单字节DMA支持能力
    struct {
        unsigned short  Mode0:1;                // 1=支持模式0 (4.17Mb/s)
        unsigned short  Mode1:1;                // 1=支持模式1 (13.3Mb/s)
        unsigned short  Mode2:1;                // 1=支持模式2 (16.7Mb/s)
        unsigned short  Reserved1:5;
        unsigned short  Mode0Sel:1;             // 1=已选择模式0
        unsigned short  Mode1Sel:1;             // 1=已选择模式1
        unsigned short  Mode2Sel:1;             // 1=已选择模式2
        unsigned short  Reserved2:5;
    } wMultiWordDMA;                    // WORD 63: 多字节DMA支持能力
    struct {
        unsigned short  AdvPOIModes:8;          // 支持高级POI模式数
        unsigned short  reserved:8;
    } wPIOCapacity;                     // WORD 64: 高级PIO支持能力
    unsigned short  wMinMultiWordDMACycle;      // WORD 65: 多字节DMA传输周期的最小值
    unsigned short  wRecMultiWordDMACycle;      // WORD 66: 多字节DMA传输周期的建议值
    unsigned short  wMinPIONoFlowCycle;         // WORD 67: 无流控制时PIO传输周期的最小值
    unsigned short  wMinPOIFlowCycle;           // WORD 68: 有流控制时PIO传输周期的最小值
    unsigned short  wReserved69[11];            // WORD 69-79: 保留
    struct {
        unsigned short  Reserved1:1;
        unsigned short  ATA1:1;                 // 1=支持ATA-1
        unsigned short  ATA2:1;                 // 1=支持ATA-2
        unsigned short  ATA3:1;                 // 1=支持ATA-3
        unsigned short  ATA4:1;                 // 1=支持ATA/ATAPI-4
        unsigned short  ATA5:1;                 // 1=支持ATA/ATAPI-5
        unsigned short  ATA6:1;                 // 1=支持ATA/ATAPI-6
        unsigned short  ATA7:1;                 // 1=支持ATA/ATAPI-7
        unsigned short  ATA8:1;                 // 1=支持ATA/ATAPI-8
        unsigned short  ATA9:1;                 // 1=支持ATA/ATAPI-9
        unsigned short  ATA10:1;                // 1=支持ATA/ATAPI-10
        unsigned short  ATA11:1;                // 1=支持ATA/ATAPI-11
        unsigned short  ATA12:1;                // 1=支持ATA/ATAPI-12
        unsigned short  ATA13:1;                // 1=支持ATA/ATAPI-13
        unsigned short  ATA14:1;                // 1=支持ATA/ATAPI-14
        unsigned short  Reserved2:1;
    } wMajorVersion;                    // WORD 80: 主版本
    unsigned short  wMinorVersion;              // WORD 81: 副版本
    unsigned short wReserved82[6]; // WORD 82-87: 保留
    struct {
        unsigned short  Mode0:1;                // 1=支持模式0 (16.7Mb/s)
        unsigned short  Mode1:1;                // 1=支持模式1 (25Mb/s)
        unsigned short  Mode2:1;                // 1=支持模式2 (33Mb/s)
        unsigned short  Mode3:1;                // 1=支持模式3 (44Mb/s)
        unsigned short  Mode4:1;                // 1=支持模式4 (66Mb/s)
        unsigned short  Mode5:1;                // 1=支持模式5 (100Mb/s)
        unsigned short  Mode6:1;                // 1=支持模式6 (133Mb/s)
        unsigned short  Mode7:1;                // 1=支持模式7 (166Mb/s) ???
        unsigned short  Mode0Sel:1;             // 1=已选择模式0
        unsigned short  Mode1Sel:1;             // 1=已选择模式1
        unsigned short  Mode2Sel:1;             // 1=已选择模式2
        unsigned short  Mode3Sel:1;             // 1=已选择模式3
        unsigned short  Mode4Sel:1;             // 1=已选择模式4
        unsigned short  Mode5Sel:1;             // 1=已选择模式5
        unsigned short  Mode6Sel:1;             // 1=已选择模式6
        unsigned short  Mode7Sel:1;             // 1=已选择模式7
    } wUltraDMA;                        // WORD 88:  Ultra DMA支持能力
    unsigned short    wReserved89[167];         // WORD 89-255
};

struct hd_cmd {
	unsigned char features;
	unsigned char count;
	unsigned char lba_low;
	unsigned char lba_mid;
	unsigned char lba_high;
	unsigned char device;
	unsigned char command;
};

struct part_info {
    unsigned char style;
    unsigned char boot_ind;
    unsigned char sys_id;
    unsigned int extended_part_dev; //父扩展分区次设备号
    unsigned long base; /* # of start sector (NOT byte offset, but SECTOR) */
    unsigned long size;	/* how many sectors in this partition */
};

/* main drive struct, one entry per drive */
struct hd_info
{
	int open_cnt;
    unsigned char channel; //IDE通道id
    unsigned char is_master; //IDE通道的master还是slave
    // struct part_info	primary[NR_PRIM_PER_DRIVE];
    // struct part_info	logical[NR_SUB_PER_DRIVE];
    // struct part_ent	primary[NR_PRIM_PER_DRIVE];
    // struct part_ent	logical[NR_SUB_PER_DRIVE];
    struct part_info part[NR_SUB_PER_PART];
};


/***************/
/* DEFINITIONS */
/***************/
#define	HD_TIMEOUT		10000	/* in millisec */
#define	PARTITION_TABLE_OFFSET	0x1BE
#define ATA_IDENTIFY		0xEC
#define ATA_READ		0x20
#define ATA_WRITE		0x30
/* for DEVICE register. */
#define	MAKE_DEVICE_REG(lba,drv,lba_highest) (((lba) << 6) |		\
					      ((drv) << 4) |		\
					      (lba_highest & 0xF) | 0xA0)

void init_hd ();
void hd_open (int drive);
void hd_identify (int drive);
void hd_handler(int irq);
int hd_rw(int drive, int cmd, unsigned char *buf, unsigned long sector, unsigned long bytes);
void hd_setup();
void hd_w(int drive, unsigned char *buf, unsigned long sector, unsigned long bytes);

#define	DRV_OF_DEV(dev) (dev <= MAX_PRIM ? \
			 dev / NR_PRIM_PER_DRIVE : \
			 (dev - MINOR_hd1a) / NR_SUB_PER_DRIVE)

#define GET_DRIVER_INDEX_BYMINOR(mi) ((unsigned int) (mi/NR_SUB_PER_PART)) //通过次设备号算出属于第几块硬盘的下标



#endif
