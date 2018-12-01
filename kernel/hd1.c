#include <string.h>
#include <stdio.h>
#include "global.h"
#include <unistd.h>
#include <math.h>
#include "hd.h"
#include "kernel.h"
#include "process.h"
#include <system/dev.h>
#include <system/fs.h>

#define MAJOR_NR 3

static unsigned char hd_status;
static unsigned char hdbuf[SECTOR_SIZE * 2];
static struct hd_info hd_info[MAX_DRIVES];
static unsigned char hd_num;
static struct blk_request *tail;
static void(*hd_callback)();

static void do_hd_request();
static void get_part_table(int drive, int sect_nr, struct part_ent *entry, int entry_count);
static void partition(int drive);
static int waitfor (int mask, int val, int timeout);
static void	interrupt_wait ();
static void add_request(struct blk_request *r);
static void print_hdinfo(struct hd_info *hdi);
static void do_read();
static void do_write();
static int do_request(int mi_dev, int cmd, unsigned long sector, unsigned char* buf, unsigned long bytes);
static int dev_do_write(struct inode *, struct file_descriptor *fd, char *buf, int nbyte);
static int dev_do_read(struct inode *inode, struct file_descriptor *fd, char *buf, int nbyte);
struct file_operation hd_f_op = {
    0,
    dev_do_read,
    dev_do_write,
    // 0,
    // 0,
    0,
    // 0,
    0,
    0,
    0};

void init_hd()
{
    dev_table[MAJOR_NR].type = DEV_TYPE_CHR;
    // dev_table[MAJOR_NR].request_fn = do_request;
    dev_table[MAJOR_NR].current_request = NULL;
    dev_table[MAJOR_NR].f_op = &hd_f_op;
    tail = NULL;
}

void hd_setup()
{
    int i;
    hd_num = 0;

    for (i = 0; i < (sizeof(hd_info) / sizeof(hd_info[0])); i++) {
		memset(&hd_info[i], 0, sizeof(hd_info[0]));
        hd_info[0].open_cnt = 0;
        hd_info[i].channel = 0;
    }

    /* Get the number of drives from the BIOS data area */
	hd_num = *(unsigned char*)(0x475);
    printk("hd1 num: %d\n", hd_num);

    
    for (i = 0; i < hd_num; i++)
    {
        hd_info[i].channel = 0;
        hd_info[i].is_master = 1;
        partition(i);
        print_hdinfo(&hd_info[i]);
    }
}

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

static void interrupt_wait(PROCESS *p)
{
    while (p->status == 1){
        // printk("1 %d name:%s mem:%x\n", p->status, p->p_name, p);
    }
}

void hd_handler(int irq)
{
    if (dev_table[MAJOR_NR].current_request==NULL) {
        printk("no hd request\n");
        return;
    }
    printk("hd iqr %x\n", hd_callback);
    hd_callback();
    printk("hd iqr end\n");
}

void do_hd_request()
{
    struct blk_request *current = dev_table[MAJOR_NR].current_request;
    struct hd_cmd cmd;
    
    cmd.device  = MAKE_DEVICE_REG(1, current->dev, (current->sector >> 24) & 0xF);
    cmd.lba_low = current->sector & 0xFF;
    cmd.lba_mid = (current->sector >> 8) & 0xFF;
    cmd.lba_high = (current->sector >> 16) & 0xFF;
    cmd.count = current->nr_sectors;
    // cmd.count = 1;
    cmd.features = 0;
    if (current->cmd == 0) //0=read 1=write
    {
        cmd.command = ATA_READ;
        hd_callback = do_read;
    } else if (current->cmd==1) {
        cmd.command = ATA_WRITE;
        hd_callback = do_write;
    } else {
        printk("unknow hd-command\n");
        return;
    }
    current->waiting->status = 1;
    hd_cmd_out(&cmd);
    if (current->cmd==1) {
        do_write();
    }
}

static void do_read()
{
    struct blk_request *current = dev_table[MAJOR_NR].current_request;
    unsigned int bytes_left = (int)fmin(SECTOR_SIZE, current->bytes);

    while (current->nr_sectors--) {
        memset(hdbuf, 0, SECTOR_SIZE);
        port_read(REG_DATA, hdbuf, SECTOR_SIZE); // 将数据从数据寄存器口读到请求结构缓冲区。
        memcpy(current->buffer, hdbuf, bytes_left);
        current->buffer += bytes_left; // 调整缓冲区指针，指向新的空区。
        current->sector++; // 起始扇区号加1
        current->bytes -= bytes_left;
    }

    // memset(hdbuf, 0, SECTOR_SIZE);
    // port_read(REG_DATA, hdbuf, SECTOR_SIZE); // 将数据从数据寄存器口读到请求结构缓冲区。
    // printk("%d", hdbuf[512]);
    // memcpy(current->buffer, hdbuf, bytes_left);
    // current->buffer += bytes_left; // 调整缓冲区指针，指向新的空区。
    // current->sector++; // 起始扇区号加1
    // current->bytes -= bytes_left;
    // if (--current->nr_sectors)
    // {
    //     hd_callback = do_read;
    //     return;
    // }

    current->waiting->status = 0;
    // printk("end read\n");
    dev_table[MAJOR_NR].current_request = current->next;
    // do_hd_request ();
}

static void do_write()
{
    struct blk_request *current = dev_table[MAJOR_NR].current_request;
    unsigned int bytes_left = (int)fmin(SECTOR_SIZE, current->bytes);
    
    if (current->nr_sectors--)
    {
        if (!waitfor(STATUS_DRQ, STATUS_DRQ, HD_TIMEOUT))
        {
            printk("hd writing error\n");
            return;
        }
        memset(hdbuf, 0, SECTOR_SIZE);
        memcpy(hdbuf, current->buffer, bytes_left);
        current->buffer += bytes_left;	// 调整缓冲区指针，指向新的空区。
        current->sector++; // 起始扇区号加1
        current->bytes -= bytes_left;
        hd_callback = do_write;
        port_write(REG_DATA, hdbuf, SECTOR_SIZE); // 将数据从数据寄存器口读到请求结构缓冲区。
        return;
    }

    current->waiting->status = 0;
    dev_table[MAJOR_NR].current_request = current->next;
}

static void get_part_table(int drive, int sect_nr, struct part_ent * entry, int entry_count)
{
    struct blk_request a;
    char buf[SECTOR_SIZE*2];
    hd_rw(drive, 0, buf, sect_nr, SECTOR_SIZE);
    memcpy(entry, buf + PARTITION_TABLE_OFFSET, sizeof(struct part_ent) * entry_count);
}

static void add_request(struct blk_request *r)
{
    r->bh = r->buffer;
    r->next = NULL;
    r->waiting = current_process;
    if (dev_table[MAJOR_NR].current_request == NULL)
    {
        dev_table[MAJOR_NR].current_request = r;
        tail = r;
    } else {
        tail->next = r;
    }
    tail = r;
}

int hd_rw(int drive, int cmd, unsigned char* buf, unsigned long sector, unsigned long bytes)
{
    struct blk_request r;
    r.buffer = buf;
    r.cmd = cmd;
    if (hd_info[drive].is_master == 1)
    {
        r.dev = 0;
    } else {
        r.dev = 1;
    }
    r.bytes = bytes;
    r.nr_sectors = (bytes + SECTOR_SIZE - 1) / SECTOR_SIZE;
    r.sector = sector;
    add_request(&r);
    do_hd_request();
    interrupt_wait(r.waiting);
    return bytes;
}

int do_request(int mi_dev, int cmd, unsigned long sector, unsigned char* buf, unsigned long bytes)
{
    int drive = GET_DRIVER_INDEX_BYMINOR(mi_dev);
    return hd_rw(drive, cmd, buf, sector, bytes);
}

static void partition(int drive)
{
	int i;
    struct hd_info * hdi = &hd_info[drive];

	struct part_ent part_tbl[NR_PART_PER_DRIVE];
    get_part_table(drive, 0, part_tbl, NR_PART_PER_DRIVE);

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

            int ext_start_sect = hdi->part[dev_nr].base;
            int s = ext_start_sect;

            for (int j = 0; j < NR_SUB_PER_PART-NR_PART_PER_DRIVE; j++) {
                struct part_ent part_tbl_logic; //逻辑扇区最多12个
                int dev_nr_logic = NR_PART_PER_DRIVE+ 1 + j;

                get_part_table(drive, s, &part_tbl_logic, 1);

                hdi->part[dev_nr_logic].sys_id = part_tbl_logic.sys_id;
                hdi->part[dev_nr_logic].boot_ind = part_tbl_logic.boot_ind;
                hdi->part[dev_nr_logic].style = P_LOGICAL;
                hdi->part[dev_nr_logic].base = part_tbl_logic.start_sect;
                hdi->part[dev_nr_logic].size = part_tbl_logic.nr_sects;

                s = ext_start_sect + part_tbl_logic.start_sect;

                /* no more logical partitions
                in this extended partition */
                if (part_tbl_logic.sys_id == NO_PART)
                    break;
            }
        }
    }
}

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
}

int dev_do_write(struct inode *inode, struct file_descriptor *fd, char *buf, int nbyte)
{
    unsigned long start_block = fd->pos / SECTOR_SIZE;
    unsigned long end_block = (fd->pos + nbyte) / SECTOR_SIZE;
    int len = (end_block - start_block + 1) * SECTOR_SIZE;
    char rbuf[len];
    hd_rw(MINOR(inode->dev_num), 0, rbuf, start_block, len);
    memcpy(&rbuf[fd->pos - start_block * SECTOR_SIZE], buf, nbyte);
    hd_rw(MINOR(inode->dev_num), 1, rbuf, start_block, len);
    fd->pos += nbyte;
    return nbyte;
}

int dev_do_read(struct inode *inode, struct file_descriptor *fd, char *buf, int nbyte)
{
    unsigned long start_block = fd->pos / SECTOR_SIZE;
    unsigned long end_block = (fd->pos + nbyte) / SECTOR_SIZE;
    int len = (end_block - start_block + 1) * SECTOR_SIZE;
    char rbuf[len];
    hd_rw(MINOR(inode->dev_num), 0, rbuf, start_block, len);
    memcpy(buf, &rbuf[fd->pos - start_block * SECTOR_SIZE], nbyte);
    fd->pos += nbyte;
    return nbyte;
}