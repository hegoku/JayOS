#include "tty.h"
#include <system/dev.h>
#include <system/fs.h>
#include <system/schedule.h>
#include <system/page.h>
#include "global.h"
#include "kernel.h"
#include "keyboard.h"

#define MAJOR_NR 4

TTY tty_table[TTY_NUM];
TTY *current_tty;

static int do_request();
static int do_read(struct file_descriptor *fd, char *buf, int nbyte);
static int do_write(struct file_descriptor *fd, char *buf, int nbyte);
struct file_operation tty_f_op = {
    0,
    do_read,
    do_write,
    // 0,
    // 0,
    0,
    // 0,
    0,
    0,
    0
};

void init_tty()
{
    for (int i=0; i < TTY_NUM;i++) {
        tty_table[i]=tty_create(i);
    }
    current_tty = &tty_table[0];
    dev_table[MAJOR_NR].name = "tty";
    dev_table[MAJOR_NR].type = DEV_TYPE_CHR;
    // dev_table[MAJOR_NR].request_fn = do_request;
    
    dev_table[MAJOR_NR].f_op = &tty_f_op;
    install_dev(MAJOR_NR, "tty", DEV_TYPE_CHR, &tty_f_op);
}

TTY tty_create(unsigned char id)
{
    TTY tty;
    tty.id = id;
    tty.inbuf_count = 0;
    tty.inbuf_head = tty.inbuf_tail = tty.in_buff;

    CONSOLE console;
    int v_mem_size = V_MEM_SIZE >> 1;

    int con_v_mem_size = v_mem_size / TTY_NUM;
    console.original_addr = id*con_v_mem_size;
    console.v_mem_limit = con_v_mem_size;
    console.current_start_addr = console.original_addr;
    console.cursor = console.current_start_addr;
    tty.console = console;

    return tty;
}

void tty_input(TTY* tty, int content)
{
    if (!(content & FLAG_EXT) || content==ENTER || content==BACKSPACE) {
        char key = '\0';
        switch (content)
        {
        case ENTER:
            key = '\n';
            break;
        case BACKSPACE:
            key = '\b';
            break;
        default:
            key = content & 0xFF;
            break;
        }
        // console_out_char(&(tty->console), key& 0xff);
        if (tty->inbuf_count < TTY_IN_BYTES)
        {
            *(tty->inbuf_head) = key;
            tty->inbuf_head++;
            if (tty->inbuf_head == tty->in_buff + TTY_IN_BYTES)
            {
                tty->inbuf_head = tty->in_buff;
            }
            tty->inbuf_count++;
        }
    }
    
}

void tty_output(TTY* tty)
{
    if (tty->inbuf_count) {
        // char output[2] = {'\0', '\0'};
        char ch = *(tty->inbuf_tail);
        // output[0] = ch&0xff;
        tty->inbuf_tail++;
        if (tty->inbuf_tail==tty->in_buff+TTY_IN_BYTES) {
            tty->inbuf_tail = tty->in_buff;
        }
        tty->inbuf_count--;
        // DispStr(output);
        console_out_char(&(tty->console), ch);
    }
    // if (!(key & FLAG_EXT)) {
    // //     tty_input(&tty, key& 0xff);
    // // tty_output(&tty);
    //     output[0] = key & 0xff;
    //     console_out_char(tty->console, ch);
    // } else if(key==ENTER) {
    //     console_out_char(tty->console, '\n');
    // }
}

int do_request(unsigned mi_dev, int cmd, char * buf,int len)
{
    if (cmd==DEV_WRITE) {
        return tty_write(mi_dev, buf, len);
    }
    else if (cmd == DEV_READ)
    {
    }
}

int do_write(struct file_descriptor *fd, char *buf, int nbyte)
{
    int a = tty_write(MINOR(fd->inode->dev_num), (char *)buf, nbyte);
    fd->pos = tty_table[MINOR(fd->inode->dev_num)].console.cursor;
    return a;
}

int do_read(struct file_descriptor *fd, char *buf, int nbyte)
{
    nbyte = 0;
    buf[0] = '\0';
    TTY *tty = &tty_table[MINOR(fd->inode->dev_num)];
repeat:
    while (tty->inbuf_count)
    {
        char ch = *(tty->inbuf_tail);
        tty->inbuf_tail++;
        if (tty->inbuf_tail==tty->in_buff+TTY_IN_BYTES) {
            tty->inbuf_tail = tty->in_buff;
        }
        tty->inbuf_count--;
        // DispStr(output);
        switch (ch)
        {
        case '\n':
            console_out_char(&(tty->console), ch);
            return nbyte;
            break;
        case '\b':
            if (nbyte>0) {
                buf[--nbyte] = '\0';
                console_out_char(&(tty->console), ch);
            }
            break;
        default:
            buf[nbyte++] = ch;
            buf[nbyte] = '\0';
            console_out_char(&(tty->console), ch);
            break;
        }
    }
    // sys_alarm(1);
    // schedule();
    goto repeat;
    return -1;
}

unsigned int tty_write(int mi_dev, char* buf, int len)
{
    TTY *tty = &tty_table[mi_dev];
    char *p = buf;
    int i = len;
    // return len;
    // DispStr(buf);
    // if ((disp_pos/2) >= 80*25)
    // {
    //     disp_pos = 0;
    // }
    // return i;
    while (i)
    {
        console_out_char(&(tty->console), *p++);
        i--;
    }
    return len;
}

static void console_out_char(CONSOLE* console, char ch)
{
    unsigned char *p_vmem = (unsigned char *)(__va(V_MEM_BASE) + console->cursor * 2);
    switch (ch) {
        case '\n':
            if (console->cursor < console->original_addr +
                console->v_mem_limit - SCREEN_WIDTH) {
                console->cursor = console->original_addr + SCREEN_WIDTH * 
                    ((console->cursor - console->original_addr) /
                    SCREEN_WIDTH + 1);
            }
            break;
        case '\b':
            if (console->cursor > console->original_addr) {
                console->cursor--;
                *(p_vmem-2) = ' ';
                *(p_vmem-1) = DEFAULT_CHAR_COLOR;
            }
            break;
        default:
            if (console->cursor < console->original_addr + console->v_mem_limit - 1) {
                *p_vmem++ = ch;
                *p_vmem++ = DEFAULT_CHAR_COLOR;
                console->cursor++;
            }
            break;
    }

    while (console->cursor >= console->current_start_addr + SCREEN_SIZE)
    {
        // console->cursor = 0;
        scroll_screen(console, SCR_DN);
    }

    flush(console);
}

static void flush(CONSOLE *console)
{
    console_set_cursor(console->cursor);
    set_console_start_addr(console->current_start_addr);
}

static void console_set_cursor(unsigned int position)
{
    asm("cli");
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
    asm("sti");
}

static void set_console_start_addr(unsigned int addr)
{
    asm("cli");
    out_byte(CRTC_ADDR_REG, START_ADDR_H);
    out_byte(CRTC_DATA_REG, (addr>>8)&0xff);
    out_byte(CRTC_ADDR_REG, START_ADDR_L);
    out_byte(CRTC_DATA_REG, addr&0xff);
    asm("sti");
}

void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}else if (direction == SCR_DN) {
        if (p_con->current_start_addr + SCREEN_SIZE < p_con->original_addr + p_con->v_mem_limit)
        {
            p_con->current_start_addr += SCREEN_WIDTH;
        }
	}

	set_console_start_addr(p_con->current_start_addr);
	console_set_cursor(p_con->cursor);
}