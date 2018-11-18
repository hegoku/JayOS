/*************************************************************************//**
 *****************************************************************************
 * @file   hd.c
 * @brief  Hard disk (winchester) driver.
 * The `device nr' in this file means minor device nr.
 * @author Forrest Y. Yu
 * @date   2005~2008
 *****************************************************************************
 *****************************************************************************/

#include <string.h>
#include <stdio.h>
#include "global.h"
#include <unistd.h>
#include <math.h>
#include "hd.h"
#include "kernel.h"
#include "process.h"

/* blk_dev_struct 块设备结构是：(kernel/blk_drv/blk.h,23)
* do_request-address //对应主设备号的请求处理程序指针。
* current-request // 该设备的下一个请求。
*/
// 该数组使用主设备号作为索引（下标）。
struct blk_dev_struct blk_dev[7] = {
	{NULL, NULL},		/* no_dev */// 0 - 无设备。
	{NULL, NULL},		/* dev mem */// 1 - 内存。
	{NULL, NULL},		/* dev fd */// 2 - 软驱设备。
	{NULL, NULL},		/* dev hd */// 3 - 硬盘设备。
	{NULL, NULL},		/* dev ttyx */// 4 - ttyx 设备。
	{NULL, NULL},		/* dev tty */// 5 - tty 设备。
	{NULL, NULL}		/* dev lp */// 6 - lp 打印机设备。
};

#define BIOS_HD 0x7E000+0x80 //loader.asm里存放硬盘信息的起始地址

static unsigned char hd_status;
static unsigned char hdbuf[SECTOR_SIZE * 2];
static struct hd_info hd_info[MAX_DRIVES];
static unsigned char hd_num;

static void do_hd_request();
static struct request *tail;
static void(*hd_callback)();
static void do_hd_read();
static void do_hd_write();
static void add_request(struct request *r);

static void	print_identify_info	(struct hd_identify_info* hdinfo);
struct hd_identify_info format_hd_info(struct hd_identify_info_raw *raw);
static void	hd_cmd_out (struct hd_cmd* cmd);
static void	get_part_table (int drive, int sect_nr, struct part_ent * entry);
static void	partition (int device, int style);
static void	print_hdinfo (struct hd_info * hdi);
static int	waitfor	(int mask, int val, int timeout);
static void	interrupt_wait ();

/*****************************************************************************
 *                                init_hd
 *****************************************************************************/
/**
 * <Ring 1> Check hard drive, set IRQ handler, enable IRQ and initialize data
 *          structures.
 *****************************************************************************/
void init_hd()
{
    blk_dev[3].request_fn = do_hd_request;
    blk_dev[3].current_request = NULL;
    tail = NULL;
    int i;
    hd_num = 0;

    for (i = 0; i < (sizeof(hd_info) / sizeof(hd_info[0])); i++) {
		memset(&hd_info[i], 0, sizeof(hd_info[0]));
        hd_info[0].open_cnt = 0;
        hd_info[i].channel = 0;
    }

    /* Get the number of drives from the BIOS data area */
	hd_num = *(unsigned char*)(0x475);
    // int bios_hd = BIOS_HD;
    // for (i = 0; i < MAX_DRIVES; i++)
    // {
    //     unsigned short cyl = *(unsigned short *)bios_hd; // 柱面数。
    //     unsigned short ctl = *(unsigned short *)(8+bios_hd); // 柱面数。
    //     // setup.s 程序在取BIOS 中的硬盘参数表信息时，如果只有1 个硬盘，就会将对应第2 个硬盘的
    //     // 16 字节全部清零。因此这里只要判断第i 个硬盘柱面数是否为0 就可以知道有没有第i 个硬盘了。  
    //     if (cyl) {
    //         printk("hd num: %x\n", ctl);

    //         // hd_identify(i);
    //         hd_info[i].channel = 0;
    //         hd_info[i].is_master = 1;
    //         hd_num++;
    //     }
    //     bios_hd += 16; // 每个硬盘的参数表长16 字节，这里BIOS 指向下一个表。
    // }
    printk("hd1 num: %d\n", hd_num);

    
    for (i = 0; i < hd_num; i++)
    {
        hd_info[i].channel = 0;
        hd_info[i].is_master = 1;
        partition(hd_info[drive], P_PRIMARY);
    }
}

/*****************************************************************************
 *                                hd_open
 *****************************************************************************/
/**
 * <Ring 1> This routine handles DEV_OPEN message. It identify the drive
 * of the given device and read the partition table of the drive if it
 * has not been read.
 * 
 * @param device The device to be opened. 次设备号
 *****************************************************************************/
void hd_open(int drive)
{
    // int index = GET_DRIVER_INDEX_BYMINOR(mi_dev);
    // int drive = DRV_OF_DEV(mi_dev);

    hd_identify(drive);

	if (hd_info[drive].open_cnt++ == 0) {
		// partition(drive * (NR_PART_PER_DRIVE + 1), P_PRIMARY);
		partition(drive, P_PRIMARY);
		print_hdinfo(&hd_info[drive]);
	}
}

/*****************************************************************************
 *                                get_part_table
 *****************************************************************************/
/**
 * <Ring 1> Get a partition table of a drive.
 * 
 * @param drive   Drive nr (0 for the 1st disk, 1 for the 2nd, ...)n
 * @param sect_nr The sector at which the partition table is located.
 * @param entry   Ptr to part_ent struct.
 *****************************************************************************/
static void get_part_table(int drive, int sect_nr, struct part_ent * entry)
{
    struct request a;
    char buf[SECTOR_SIZE*2];
    // hd_rw(drive, 0, buf, sect_nr, SECTOR_SIZE);
    // a.buffer = buf;
    // a.cmd = 0;
    // if (hd_info[drive].is_master==1) {
    //     a.dev = 0;
    // } else {
    //     a.dev = 1;
    // }
    // a.nr_sectors = 1;
    // a.sector = sect_nr;
    // make_request(&a);
    // if (blk_dev[3].current_request == NULL)
    // {
    //     blk_dev[3].current_request = &a;
    //     tail = &a;
    // } else {
    //     tail->next = &a;
    // }
    // tail = &a;

    // do_hd_request();
    // interrupt_wait(a.waiting);
    // struct hd_cmd cmd;
    // cmd.features	= 0;
    // cmd.count	= 1;
    // cmd.lba_low	= sect_nr & 0xFF;
    // cmd.lba_mid	= (sect_nr >>  8) & 0xFF;
    // cmd.lba_high	= (sect_nr >> 16) & 0xFF;
    // cmd.device	= MAKE_DEVICE_REG(1, /* LBA mode*/
    // 				  drive,
    // 				  (sect_nr >> 24) & 0xF);
    // cmd.command	= ATA_READ;
    // hd_callback = hd_read;
    // a.waiting->status = 1;
    // hd_cmd_out(&cmd);
    // interrupt_wait(a.waiting);

    // port_read(REG_DATA, hdbuf, SECTOR_SIZE);
    memcpy(entry,
           buf + PARTITION_TABLE_OFFSET,
           sizeof(struct part_ent) * NR_PART_PER_DRIVE);
}

/*****************************************************************************
 *                                partition
 *****************************************************************************/
/**
 * 通过任意次设备号获取其所属硬盘的主分区信息或 扩展分区的逻辑分区信息
 * <Ring 1> This routine is called when a device is opened. It reads the
 * partition table(s) and fills the hd_info struct.
 * 
 * @param device Device nr. 次设备号
 * @param style  P_PRIMARY or P_EXTENDED.
 *****************************************************************************/
static void partition(int drive, int style)
{
	int i;
    // int index = GET_DRIVER_INDEX_BYMINOR(mi_dev); //第几块硬盘
    // int drive = DRV_OF_DEV(device);
    struct hd_info * hdi = &hd_info[drive];

	if (style == P_PRIMARY) {
        struct part_ent part_tbl[NR_PART_PER_DRIVE];
        get_part_table(drive, 0, part_tbl);

        int nr_prim_parts = 0;
		for (i = 0; i < NR_PART_PER_DRIVE; i++) { /* 0~3 */
			if (part_tbl[i].sys_id == NO_PART) 
				continue;

            nr_prim_parts++;
            int dev_nr = i + 1;		  /* 1~4 */
            hdi->part[dev_nr].sys_id = part_tbl[i].sys_id;
            hdi->part[dev_nr].boot_ind = part_tbl[i].boot_ind;
            hdi->part[dev_nr].style = P_PRIMARY;
            hdi->part[dev_nr].base = part_tbl[i].start_sect;
            hdi->part[dev_nr].size = part_tbl[i].nr_sects;

			if (part_tbl[i].sys_id == EXT_PART) { /* extended */
                int base_dev=drive *NR_SUB_PER_PART; //该硬盘的第0个设备的设备号
                hdi->part[dev_nr].style = P_EXTENDED;
                // partition(base_dev + dev_nr, P_EXTENDED);

                struct part_ent part_tbl_logic[NR_SUB_PER_PART-NR_PART_PER_DRIVE]; //逻辑扇区最多12个
                // int j = mi_dev % NR_PRIM_PER_DRIVE; /* 1~4 */
                // int j = mi_dev - drive * 2;
                int ext_start_sect = hdi->part[dev_nr].base;
                int s = ext_start_sect;
                // int nr_1st_sub = 5; /* 0/16/32/48 */

                for (int j = 0; j < NR_SUB_PER_PART-NR_PART_PER_DRIVE; j++) {
                    int dev_nr_logic = NR_PART_PER_DRIVE+ 1 + j;

                    get_part_table(drive, s, part_tbl);

                    // hdi->logical[dev_nr] = part_tbl[0];
                    // hdi->logical[dev_nr].base = s + part_tbl[0].start_sect;
                    // hdi->logical[dev_nr].size = part_tbl[0].nr_sects;
                    hdi->part[dev_nr_logic].sys_id = part_tbl_logic[j].sys_id;
                    hdi->part[dev_nr_logic].boot_ind = part_tbl_logic[j].boot_ind;
                    hdi->part[dev_nr_logic].style = P_LOGICAL;
                    hdi->part[dev_nr_logic].base = part_tbl_logic[j].start_sect;
                    hdi->part[dev_nr_logic].size = part_tbl_logic[j].nr_sects;

                    s = ext_start_sect + part_tbl_logic[1].start_sect;

                    /* no more logical partitions
                    in this extended partition */
                    if (part_tbl_logic[1].sys_id == NO_PART)
                        break;
                }
            }
				
		}
	} else if (style == P_EXTENDED) {
        
	}
	else {
	}
}

/*****************************************************************************
 *                                print_hdinfo
 *****************************************************************************/
/**
 * <Ring 1> Print disk info.
 * 
 * @param hdi  Ptr to struct hd_info.
 *****************************************************************************/
static void print_hdinfo(struct hd_info * hdi)
{
	int i;
	for (i = 0; i < NR_PART_PER_DRIVE + 1; i++) {
        printk("%sPART_%d: BI 0x%x, base %d(0x%x), size %d(0x%x), sid 0x%x\n",
               i == 0 ? " " : (i<=4 ? "     " : "         "),
               i,
               hdi->part[i].boot_ind,
               hdi->part[i].base,
               hdi->part[i].base,
               hdi->part[i].size,
               hdi->part[i].size,
               hdi->part[i].sys_id);
    }
    // for (i = 0; i < NR_SUB_PER_DRIVE; i++) {
	// 	if (hdi->logical[i].nr_sects == 0)
	// 		continue;
	// 	printk("         "
	// 	       "%d: BI 0x%x, base %d(0x%x), size %d(0x%x), sid 0x%x\n",
	// 	       i,
    //            hdi->logical[i].boot_ind,
	// 	       hdi->logical[i].start_sect,
	// 	       hdi->logical[i].start_sect,
	// 	       hdi->logical[i].nr_sects,
	// 	       hdi->logical[i].nr_sects,
    //            hdi->logical[i].sys_id);
	// }
}

/*****************************************************************************
 *                                hd_identify
 *****************************************************************************/
/**
 * <Ring 1> Get the disk information.
 * 
 * @param drive  Drive Nr.
 *****************************************************************************/
void hd_identify(int drive)
{
    struct request a;
    struct hd_identify_info_raw info_raw;
    a.buffer = (char *)&info_raw;
    a.cmd = 0;
    if (hd_info[drive].is_master==1) {
        a.dev = 0;
    } else {
        a.dev = 1;
    }
    a.nr_sectors = 1;
    a.sector = 1;
    a.bytes = SECTOR_SIZE;
    make_request(&a);

    struct hd_cmd cmd;
    cmd.device  = MAKE_DEVICE_REG(0, a.dev, 0);
	cmd.command = ATA_IDENTIFY;
    hd_callback = do_hd_read;
    a.waiting->status = 1;
    hd_cmd_out(&cmd);
    interrupt_wait(a.waiting);
    struct hd_identify_info info = format_hd_info((struct hd_identify_info_raw *)a.bh);
    print_identify_info(&info);
	// interrupt_wait();
	// port_read(REG_DATA, hdbuf, SECTOR_SIZE);

	// print_identify_info((unsigned  short*)hdbuf);

	// unsigned short* hdinfo = (unsigned  short*)hdbuf;

	hd_info[drive].part[0].base = 0;
	/* Total Nr of User Addressable Sectors */
	hd_info[drive].part[0].size = info.dwTotalSectors;
}

/*****************************************************************************
 *                            print_identify_info
 *****************************************************************************/
/**
 * <Ring 1> Print the hdinfo retrieved via ATA_IDENTIFY command.
 * 
 * @param hdinfo  The buffer read from the disk i/o port.
 *****************************************************************************/
// static void print_identify_info(unsigned short* hdinfo)
static void print_identify_info(struct hd_identify_info* hdinfo)
{
	int i, k;
	char s[64];

	struct iden_info_ascii {
		int idx;
		int len;
		char * desc;
	} iinfo[] = {{10, 20, "HD SN"}, /* Serial number in ASCII */
		     {27, 40, "HD Model"} /* Model number in ASCII */ };

	// for (k = 0; k < sizeof(iinfo)/sizeof(iinfo[0]); k++) {
	// 	char * p = (char*)&hdinfo[iinfo[k].idx];
	// 	for (i = 0; i < iinfo[k].len/2; i++) {
	// 		s[i*2+1] = *p++;
	// 		s[i*2] = *p++;
	// 	}
	// 	s[i*2] = 0;
	// 	printf("%s: %s\n", iinfo[k].desc, s);
	// }
    // char * p = hdinfo->sSerialNumber;
    // for (i = 0; i < 10/2; i++) {
    //     s[i*2+1] = *p++;
    //     s[i*2] = *p++;
    // }
    // s[i*2] = 0;

    // char a[21];
    // for (int i = 0; i < 20;i++) {
    //     a[i] = hdinfo->sSerialNumber[i];
    // }
    // a[20] = '\0';
    printf("HD SN: %s\n", hdinfo->sSerialNumber);
    printf("HD Model: %s\n", hdinfo->sModelNumber);
	// int capabilities = hdinfo[49];
	printf("LBA supported: %s\n",
	       hdinfo->wCapabilities.LBA ? "Yes" : "No");

	int cmd_set_supported = hdinfo->wReserved82[1];
	printf("LBA48 supported: %s\n",
	       (cmd_set_supported & 0x0400) ? "Yes" : "No");

	int sectors = hdinfo->dwTotalSectors * 512 / 1000000;
    printf("HD size: %dMB\n", sectors);
}

/*****************************************************************************
 *                                hd_cmd_out
 *****************************************************************************/
/**
 * <Ring 1> Output a command to HD controller.
 * 
 * @param cmd  The command struct ptr.
 *****************************************************************************/
static void hd_cmd_out(struct hd_cmd* cmd)
{
	/**
	 * For all commands, the host must first check if BSY=1,
	 * and should proceed no further unless and until BSY=0
	 */
    if (!waitfor(STATUS_BSY, 0, HD_TIMEOUT))
    {
        printf("hd error.\n");
        return;
    }

    /* Activate the Interrupt Enable (nIEN) bit */
	out_byte(REG_DEV_CTRL, 0);
    /* Load required parameters in the Command Block Registers */
    out_byte(REG_FEATURES, cmd->features);
	out_byte(REG_NSECTOR,  cmd->count);
	out_byte(REG_LBA_LOW,  cmd->lba_low);
	out_byte(REG_LBA_MID,  cmd->lba_mid);
	out_byte(REG_LBA_HIGH, cmd->lba_high);
	out_byte(REG_DEVICE,   cmd->device);
	/* Write the command code to the Command Register */
	out_byte(REG_CMD,     cmd->command);
}

/*****************************************************************************
 *                                interrupt_wait
 *****************************************************************************/
/**
 * <Ring 1> Wait until a disk interrupt occurs.
 * 
 *****************************************************************************/
static void interrupt_wait(PROCESS *p)
{
    while (p->status == 1){
        // printk("1 %d name:%s mem:%x\n", p->status, p->p_name, p);
    }
}

/*****************************************************************************
 *                                waitfor
 *****************************************************************************/
/**
 * <Ring 1> Wait for a certain status.
 * 
 * @param mask    Status mask.
 * @param val     Required status.
 * @param timeout Timeout in milliseconds.
 * 
 * @return One if sucess, zero if timeout.
 *****************************************************************************/
static int waitfor(int mask, int val, int timeout)
{
	int t = get_ticks();

    while (((get_ticks() - t) * 1000 / 100) < timeout) {
        if ((in_byte(REG_STATUS) & mask) == val)
        {
            return 1;
        }
    }

	return 0;
}

// <Ring 0>
void hd_handler(int irq)
{
    if (blk_dev[3].current_request==NULL) {
        printk("no hd request\n");
        return;
    }
    printk("hd iqr\n");
    hd_callback();
    printk("hd iqr end\n");
    return;
    if (blk_dev[3].current_request->cmd == 0)
    { //0=read 1=write
        // hd_status = in_byte(REG_STATUS);
        // struct hd_identify_info_raw info_raw;
        port_read(REG_DATA, hdbuf, SECTOR_SIZE);
        // port_read(REG_DATA, &info_raw, 256);
        memcpy(blk_dev[3].current_request->buffer, hdbuf, SECTOR_SIZE);
        // blk_dev[3].current_request->buffer = &info_raw;
        printk("status: %x, %x\n", hd_status, hdbuf);
    }
    else if (blk_dev[3].current_request->cmd == 0)
    {
    } else {
        printf("unknown hd-command in hd_handler\n");
        return;
    }

    printk("2 %d name:%s mem:%x\n", blk_dev[3].current_request->waiting->status, blk_dev[3].current_request->waiting->p_name, blk_dev[3].current_request->waiting);
    blk_dev[3].current_request->waiting->status = 0;
    printk("%d mem_n:\n", blk_dev[3].current_request->waiting->status, blk_dev[3].current_request->next);
    blk_dev[3].current_request = blk_dev[3].current_request->next;

    // hd_status = in_byte(REG_STATUS);
    // struct hd_identify_info_raw info_raw;
    // // port_read(REG_DATA, hdbuf, SECTOR_SIZE);
    // port_read(REG_DATA, &info_raw, 256);
    // struct hd_identify_info info=format_hd_info(&info_raw);
    // print_identify_info(&info);
}

struct hd_identify_info format_hd_info(struct hd_identify_info_raw *raw)
{
    struct hd_identify_info info;
    info.wGenConfig = raw->wGenConfig;
    info.wNumCyls = raw->wNumCyls;
    info.wReserved2 = raw->wReserved2;
    info.wNumHeads = raw->wNumHeads;
    info.wReserved4 = raw->wReserved4;
    info.wReserved5 = raw->wReserved5;
    info.wNumSectorsPerTrack = raw->wNumSectorsPerTrack;
    memcpy(info.wVendorUnique ,raw->wVendorUnique, sizeof(raw->wVendorUnique));

    char * p = raw->sSerialNumber;
    int i;
    for (i=0; i < 10 / 2; i++)
    {
        info.sSerialNumber[i*2+1] = *p++;
        info.sSerialNumber[i*2] = *p++;
    }
    info.sSerialNumber[i*2] = '\0';

    info.wBufferType = raw->wBufferType;
    info.wBufferSize = raw->wBufferSize;
    info.wECCSize = raw->wECCSize;
    // info.sFirmwareRev = raw->sFirmwareRev;

    p = raw->sModelNumber;
    for (i = 0; i < 20 / 2; i++)
    {
        info.sModelNumber[i*2+1] = *p++;
        info.sModelNumber[i*2] = *p++;
    }
    info.sModelNumber[i*2] = '\0';

    info.wMoreVendorUnique = raw->wMoreVendorUnique;
    info.wReserved48 = raw->wReserved48;
    info.wCapabilities = raw->wCapabilities;
    info.wReserved1 = raw->wReserved1;
    info.wPIOTiming = raw->wPIOTiming;
    info.wDMATiming = raw->wDMATiming;
    memcpy(&info.wFieldValidity ,&raw->wFieldValidity, sizeof(raw->wFieldValidity));
    // info.wFieldValidity = raw->wFieldValidity;
    info.wNumCurCyls = raw->wNumCurCyls;
    info.wNumCurHeads = raw->wNumCurHeads;
    info.wNumCurSectorsPerTrack = raw->wNumCurSectorsPerTrack;
    info.wCurSectorsLow = raw->wCurSectorsLow;
    info.wCurSectorsHigh = raw->wCurSectorsHigh;
    // info.wMultSectorStuff = raw->wMultSectorStuff;
    info.dwTotalSectors = raw->dwTotalSectors;
    info.wSingleWordDMA = raw->wSingleWordDMA;
    // info.wMultiWordDMA = raw->wMultiWordDMA;
    // info.wPIOCapacity = raw->wPIOCapacity;
    info.wMinMultiWordDMACycle = raw->wMinMultiWordDMACycle;
    info.wRecMultiWordDMACycle = raw->wRecMultiWordDMACycle;
    info.wMinPIONoFlowCycle = raw->wMinPIONoFlowCycle;
    info.wMinPOIFlowCycle = raw->wMinPOIFlowCycle;
    memcpy(info.wReserved69 ,raw->wReserved69, sizeof(raw->wReserved69));
    // info.wMajorVersion = raw->wMajorVersion;
    info.wMinorVersion = raw->wMinorVersion;
    memcpy(info.wReserved82 ,raw->wReserved82, sizeof(raw->wReserved82));
    // info.wUltraDMA = raw->wUltraDMA;
    memcpy(info.wReserved89 ,raw->wReserved89, sizeof(raw->wReserved89));

    return info;
}

static void do_hd_request()
{
    struct request *current = blk_dev[3].current_request;
    struct hd_cmd cmd;
    
    cmd.device  = MAKE_DEVICE_REG(1, current->dev, (current->sector >> 24) & 0xF);
    cmd.lba_low = current->sector & 0xFF;
    cmd.lba_mid = (current->sector >> 8) & 0xFF;
    cmd.lba_high = (current->sector >> 16) & 0xFF;
    // cmd.count = current->nr_sectors;
    cmd.count = 1;
    cmd.features = 0;
    if (current->cmd == 0) //0=read 1=write
    {
        cmd.command = ATA_READ;
        hd_callback = do_hd_read;
    } else if (current->cmd==1) {
        cmd.command = ATA_WRITE;
        hd_callback = do_hd_write;
    } else {
        printk("unknow hd-command\n");
        return;
    }
    current->waiting->status = 1;
    hd_cmd_out(&cmd);
    if (current->cmd==1) {
        do_hd_write();
    }
}

static void do_hd_read()
{
    // printk("hd_read\n");
    struct request *current = blk_dev[3].current_request;
    // port_read(REG_DATA, hdbuf, SECTOR_SIZE);
    unsigned int bytes_left = fmin(SECTOR_SIZE, current->bytes);
    port_read(REG_DATA, current->buffer, bytes_left); // 将数据从数据寄存器口读到请求结构缓冲区。
    // printk("ibe:%x\n", current->bh+PARTITION_TABLE_OFFSET);
    // memcpy(blk_dev[3].current_request->buffer, hdbuf, SECTOR_SIZE);
    current->buffer += SECTOR_SIZE;	// 调整缓冲区指针，指向新的空区。
    current->sector++; // 起始扇区号加1
    current->bytes -= bytes_left;
    if (--current->nr_sectors)
    {				// 如果所需读出的扇区数还没有读完，则
        // printk("rn: %d\n", current->nr_sectors);
        hd_callback = do_hd_read; // 再次置硬盘调用C 函数指针为read_intr()
        do_hd_request ();
        // return;			// 因为硬盘中断处理程序每次调用do_hd 时
	}				// 都会将该函数指针置空。参见system_call.s
    current->waiting->status = 0;
    // printk("end read\n");
    blk_dev[3].current_request = current->next;
    // do_hd_request ();
}

static void do_hd_write()
{
    if (!waitfor(STATUS_DRQ, STATUS_DRQ, HD_TIMEOUT))
    {
        printk("hd writing error\n");
        return;
    }
    // printk("hd_read\n");
    struct request *current = blk_dev[3].current_request;
    // port_read(REG_DATA, hdbuf, SECTOR_SIZE);
    unsigned int bytes_left = fmin(SECTOR_SIZE, current->bytes);
    port_write (REG_DATA, current->buffer, bytes_left);	// 将数据从数据寄存器口读到请求结构缓冲区。
    // printk("ibe:%x\n", current->bh+PARTITION_TABLE_OFFSET);
    // memcpy(blk_dev[3].current_request->buffer, hdbuf, SECTOR_SIZE);
    current->buffer += SECTOR_SIZE;	// 调整缓冲区指针，指向新的空区。
    current->sector++; // 起始扇区号加1，
    current->bytes -= bytes_left;
    if (--current->nr_sectors)
	{				// 如果所需读出的扇区数还没有读完，则
        // printk("rn: %d\n", current->nr_sectors);
        hd_callback = do_hd_write; // 再次置硬盘调用C 函数指针为read_intr()
        do_hd_request ();
        // return;			// 因为硬盘中断处理程序每次调用do_hd 时
	}				// 都会将该函数指针置空。参见system_call.s
    current->waiting->status = 0;
    // printk("end read\n");
    blk_dev[3].current_request = current->next;
    // do_hd_request ();
}

void make_request(struct request *r)
{
    r->waiting = current_process;
    add_request(r);
}

static void add_request(struct request *r)
{
    r->bh = r->buffer;
    r->next = NULL;
    if (blk_dev[3].current_request == NULL)
    {
        blk_dev[3].current_request = r;
        tail = r;
    } else {
        tail->next = r;
    }
    tail = r;
}



void hd_rw(int drive, int cmd, unsigned char* buf, unsigned long sector, unsigned long bytes)
{
    struct request r;
    r.buffer = buf;
    r.cmd = cmd;
    if (hd_info[drive].is_master==1) {
        r.dev = 0;
    } else {
        r.dev = 1;
    }
    r.bytes = bytes;
    r.nr_sectors = (bytes + SECTOR_SIZE - 1) / SECTOR_SIZE;
    r.sector = sector;
    r.waiting = current_process;
    add_request(&r);
    do_hd_request();
    interrupt_wait(r.waiting);
}