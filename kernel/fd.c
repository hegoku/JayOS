#include "global.h"
#include "fd.h"

static unsigned short  current_dev = 1;

void floppy_init()
{
    current_dev = 0;
    // out_byte(FD_DOR, 0x18); //重启软驱
    out_byte(FD_DOR, 0x1c); //重启软驱
    out_byte(FD_CCR, 0); //设置速度,1.44

    output_byte(FD_SPECIFY);
    output_byte(0xCF); /* 马达步进速度、磁头卸载时间=32ms */
    output_byte(6); /* Head load time =6ms, DMA */
}

void floppy_handler(int irq)
{
	disp_int(irq);
}

// void floppy_motor_on()
// {
//     out_byte(FD_DOR, 0x18);
// }

static void output_byte(unsigned char byte)
{
    int counter;
    unsigned char status;

    for(counter = 0 ; counter < 10000 ; counter++) {
        status = in_byte(FD_STATUS) & (FD_STATUS_READY | FD_STATUS_DIR);
        if (status == FD_STATUS_READY) {
            out_byte(FD_DATA, byte);
            return;
        }
    }

    DispStr("FDC error\n");
}

void FloppyReadSector(unsigned short  sectNo, unsigned char *buf)
{

    unsigned short  head, track, block, sector, seek_track;

    // if (0 == buf)
    // {
    //     DispStr("FloppyReadSector para error.\n");
    //     return;
    // }



    if (sectNo >= (floppy_type.head * floppy_type.track * floppy_type.sect))
    {

        // printf("FloppyReadSector sectNo error: %x.\r", sectNo);
        DispStr("FloppyReadSector sectNo error: %x.\n");
        return;
    }

    /* 计算参数 */
    sector = sectNo % floppy_type.sect + 1;
    block = sectNo / floppy_type.sect;
    track = block / floppy_type.head;
    head = block % floppy_type.head;
    seek_track = track << floppy_type.stretch;

    /* 软盘重新校正 */
    output_byte(FD_RECALIBRATE);
    output_byte(current_dev);

    /* 寻找磁道 */
    output_byte(FD_SEEK);
    output_byte(current_dev);
    output_byte(seek_track);

    /* 设置DMA，准备传送数据 */
    SetDMA(buf, FD_READ);

    /* 发送读扇区命令 */
    output_byte(FD_READ); /* command */
    output_byte(current_dev); /* driver no. */
    output_byte(track); /* track no. */
    output_byte(head); /* head */
    output_byte(sector); /* start sector */
    output_byte(2); /* sector size = 512 */
    output_byte(floppy_type.sect); /* Max sector */
    output_byte(floppy_type.gap); /* sector gap */
    output_byte(0xFF); /* sector size (0xff when n!=0 ?) */
}

#define DMA_READ 0x46

#define DMA_WRITE 0x4A



#define immoutb_p(val,port) asm("outb %0,%1\n\tjmp 1f\n1:\tjmp 1f\n1:"::"a" ((char) (val)),"i" (port))



void SetDMA(unsigned char *buf, unsigned char cmd)

{

    long addr = (long)buf;

    asm("cli");

    /* mask DMA 2 */

    immoutb_p(4|2,10);

    /* output command byte. I don't know why, but everyone (minix, */

    /* sanches & canton) output this twice, first to 12 then to 11 */

    asm("outb %%al,$12\n\tjmp 1f\n1:\tjmp 1f\n1:\t"

    "outb %%al,$11\n\tjmp 1f\n1:\tjmp 1f\n1:"::

    "a" ((char) ((cmd == FD_READ)?DMA_READ:DMA_WRITE)));

    /* 8 low bits of addr */

    immoutb_p(addr,4);

    addr >>= 8;

    /* bits 8-15 of addr */

    immoutb_p(addr,4);

    addr >>= 8;

    /* bits 16-19 of addr */

    immoutb_p(addr,0x81);

    /* low 8 bits of count-1 (1024-1=0x3ff) */

    immoutb_p(0xff,5);

    /* high 8 bits of count-1 */

    immoutb_p(3,5);

    /* activate DMA 2 */

    immoutb_p(0|2,10);
    disp_int(addr);

    asm("sti");
}