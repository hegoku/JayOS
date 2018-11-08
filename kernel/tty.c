#include "tty.h"
#include "global.h"
#include "keyboard.h"

TTY tty_create(unsigned char id)
{
    TTY tty;
    tty.id = id;
    tty.inbuf_count = 0;
    tty.inbuf_head = tty.inbuf_tail = tty.in_buff;

    CONSOLE *console;
    int v_mem_size = V_MEM_SIZE >> 1;

    int con_v_mem_size = v_mem_size / TTY_NUM;
    console->original_addr = id*con_v_mem_size;
    console->v_mem_limit = con_v_mem_size;
    console->current_start_addr = console->original_addr;
    console->cursor = console->current_start_addr;
    tty.console = console;

    return tty;
}

void tty_input(TTY* tty, int content)
{
    if (!(content & FLAG_EXT)) {
        console_out_char(tty->console, content& 0xff);
    }
    // if (tty->inbuf_count<TTY_IN_BYTES) {
    //     *(tty->inbuf_head) = content;
    //     tty->inbuf_head++;
    //     if (tty->inbuf_head==tty->in_buff+TTY_IN_BYTES) {
    //         tty->inbuf_head = tty->in_buff;
    //     }
    //     tty->inbuf_count++;
    // }
}

void tty_output(TTY* tty)
{
    if (tty->inbuf_count) {
        char output[2] = {'\0', '\0'};
        char ch = *(tty->inbuf_tail);
        output[0] = ch&0xff;
        tty->inbuf_tail++;
        if (tty->inbuf_tail==tty->in_buff+TTY_IN_BYTES) {
            tty->inbuf_tail = tty->in_buff;
        }
        tty->inbuf_count--;
        // DispStr(output);
        console_out_char(tty->console, ch);
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

static void console_out_char(CONSOLE* console, char ch)
{
    unsigned char *p_vmem = (unsigned char *)(V_MEM_BASE+console->cursor*2);

    *p_vmem++ = ch;
    *p_vmem++ = DEFAULT_CHAR_COLOR;
    console->cursor++;
    console_set_cursor(console->cursor);
}

static void console_set_cursor(unsigned int position)
{
    asm("cli");
    out_byte(CRTC_ADDR_REG, 0xc);
    out_byte(CRTC_DATA_REG, ((position/2)>>8)&0xff);
    out_byte(CRTC_ADDR_REG, 0xd);
    out_byte(CRTC_DATA_REG, position&0xff);
    asm("sti");
}