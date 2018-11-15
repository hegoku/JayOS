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

static void do_hd_request();
static struct request *tail;
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
    /* Get the number of drives from the BIOS data area */
	unsigned char * hd_num = (unsigned char*)(0x475);
    printk("hd num: %d\n", *hd_num);

    for (i = 0; i < (sizeof(hd_info) / sizeof(hd_info[0])); i++) {
		memset(&hd_info[i], 0, sizeof(hd_info[0]));
    }
	hd_info[0].open_cnt = 0;
}

/*****************************************************************************
 *                                hd_open
 *****************************************************************************/
/**
 * <Ring 1> This routine handles DEV_OPEN message. It identify the drive
 * of the given device and read the partition table of the drive if it
 * has not been read.
 * 
 * @param device The device to be opened.
 *****************************************************************************/
void hd_open(int device)
{
	int drive = DRV_OF_DEV(device);

	hd_identify(drive);

	if (hd_info[drive].open_cnt++ == 0) {
		partition(drive * (NR_PART_PER_DRIVE + 1), P_PRIMARY);
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
	struct hd_cmd cmd;
	cmd.features	= 0;
	cmd.count	= 1;
	cmd.lba_low	= sect_nr & 0xFF;
	cmd.lba_mid	= (sect_nr >>  8) & 0xFF;
	cmd.lba_high	= (sect_nr >> 16) & 0xFF;
	cmd.device	= MAKE_DEVICE_REG(1, /* LBA mode*/
					  drive,
					  (sect_nr >> 24) & 0xF);
	cmd.command	= ATA_READ;
	hd_cmd_out(&cmd);
	interrupt_wait();

	port_read(REG_DATA, hdbuf, SECTOR_SIZE);
	memcpy(entry,
	       hdbuf + PARTITION_TABLE_OFFSET,
	       sizeof(struct part_ent) * NR_PART_PER_DRIVE);
}

/*****************************************************************************
 *                                partition
 *****************************************************************************/
/**
 * <Ring 1> This routine is called when a device is opened. It reads the
 * partition table(s) and fills the hd_info struct.
 * 
 * @param device Device nr.
 * @param style  P_PRIMARY or P_EXTENDED.
 *****************************************************************************/
static void partition(int device, int style)
{
	int i;
	int drive = DRV_OF_DEV(device);
	struct hd_info * hdi = &hd_info[drive];

	struct part_ent part_tbl[NR_SUB_PER_DRIVE];

	if (style == P_PRIMARY) {
		get_part_table(drive, drive, part_tbl);

		int nr_prim_parts = 0;
		for (i = 0; i < NR_PART_PER_DRIVE; i++) { /* 0~3 */
			if (part_tbl[i].sys_id == NO_PART) 
				continue;

			nr_prim_parts++;
			int dev_nr = i + 1;		  /* 1~4 */
			hdi->primary[dev_nr].base = part_tbl[i].start_sect;
			hdi->primary[dev_nr].size = part_tbl[i].nr_sects;

			if (part_tbl[i].sys_id == EXT_PART) /* extended */
				partition(device + dev_nr, P_EXTENDED);
		}
	}
	else if (style == P_EXTENDED) {
		int j = device % NR_PRIM_PER_DRIVE; /* 1~4 */
		int ext_start_sect = hdi->primary[j].base;
		int s = ext_start_sect;
		int nr_1st_sub = (j - 1) * NR_SUB_PER_PART; /* 0/16/32/48 */

		for (i = 0; i < NR_SUB_PER_PART; i++) {
			int dev_nr = nr_1st_sub + i;/* 0~15/16~31/32~47/48~63 */

			get_part_table(drive, s, part_tbl);

			hdi->logical[dev_nr].base = s + part_tbl[0].start_sect;
			hdi->logical[dev_nr].size = part_tbl[0].nr_sects;

			s = ext_start_sect + part_tbl[1].start_sect;

			/* no more logical partitions
			   in this extended partition */
			if (part_tbl[1].sys_id == NO_PART)
				break;
		}
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
		printk("%sPART_%d: base %d(0x%x), size %d(0x%x) (in sector)\n",
		       i == 0 ? " " : "     ",
		       i,
		       hdi->primary[i].base,
		       hdi->primary[i].base,
		       hdi->primary[i].size,
		       hdi->primary[i].size);
	}
	for (i = 0; i < NR_SUB_PER_DRIVE; i++) {
		if (hdi->logical[i].size == 0)
			continue;
		printk("         "
		       "%d: base %d(0x%x), size %d(0x%x) (in sector)\n",
		       i,
		       hdi->logical[i].base,
		       hdi->logical[i].base,
		       hdi->logical[i].size,
		       hdi->logical[i].size);
	}
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
    a.dev = 3;
    a.next = NULL;
    a.waiting = p_proc_ready;
    
    if (blk_dev[3].current_request==NULL) {
        blk_dev[3].current_request = &a;
        tail = &a;
    } else {
        tail = &a;
    }
    tail = &a;
    
    struct hd_cmd cmd;
    cmd.device  = MAKE_DEVICE_REG(0, drive, 0);
	cmd.command = ATA_IDENTIFY;
	hd_cmd_out(&cmd);
    interrupt_wait(a.waiting);
    struct hd_identify_info info = format_hd_info((struct hd_identify_info_raw*)a.buffer);
    print_identify_info(&info);
	// interrupt_wait();
	// port_read(REG_DATA, hdbuf, SECTOR_SIZE);

	// print_identify_info((unsigned  short*)hdbuf);

	// unsigned short* hdinfo = (unsigned  short*)hdbuf;

	// hd_info[drive].primary[0].base = 0;
	// /* Total Nr of User Addressable Sectors */
	// hd_info[drive].primary[0].size = ((int)hdinfo[61] << 16) + hdinfo[60];
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
    p->status = 0;
    while (p->status == 1){}
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
        return;
    }
    if (blk_dev[3].current_request->cmd==0) { //0=read 1=write
        hd_status = in_byte(REG_STATUS);
        struct hd_identify_info_raw info_raw;
        port_read(REG_DATA, hdbuf, SECTOR_SIZE);
        // port_read(REG_DATA, &info_raw, 256);
        memcpy(blk_dev[3].current_request->buffer, hdbuf, 256);
        blk_dev[3].current_request->waiting->status = 0;
        // blk_dev[3].current_request->buffer = &info_raw;
    } else if (blk_dev[3].current_request->cmd==0) {

    } else {
        printf("unknown hd-command in hd_handler\n");
        return;
    }

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


void hd_send_request()
{

}

static void do_hd_request()
{
    struct request *current = blk_dev[3].current_request;
    struct hd_cmd cmd;
    int bytes_left = current->nr_sectors*SECTOR_SIZE;
    cmd.device  = MAKE_DEVICE_REG(0, current->dev, 0);
    cmd.lba_low = current->nr_sectors & 0xFF;
    cmd.lba_mid = (current->nr_sectors >> 8) & 0xFF;
    cmd.lba_high = (current->nr_sectors >> 16) & 0xFF;
    cmd.count = bytes_left;
    cmd.features = 0;
    if (current->cmd == 0) //0=read 1=write
    {
        cmd.command = ATA_READ;
    } else if (current->cmd==1) {
        cmd.command = ATA_WRITE;
    } else {
        printk("unknow hd-command\n");
        return;
    }
    hd_cmd_out(&cmd);
	void * la = current->buffer;

	while (bytes_left) {
		int bytes = fmin((double)SECTOR_SIZE, (double)bytes_left);
		if (current->cmd == 0) {
            interrupt_wait(current->waiting);
            port_read(REG_DATA, hdbuf, SECTOR_SIZE);
            memcpy(la, hdbuf, bytes);
        }
		else {
			if (!waitfor(STATUS_DRQ, STATUS_DRQ, HD_TIMEOUT)) {
                printk("hd writing error.\n");
                return;
            }
            port_write(REG_DATA, la, bytes);
			interrupt_wait(current->waiting);
		}
		bytes_left -= SECTOR_SIZE;
		la += SECTOR_SIZE;
    }
}

void hd_read(int drive, unsigned long pos)
{
    int dev = DRV_OF_DEV(drive);


}