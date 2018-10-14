#ifndef	_TTY_H_
#define	_TTY_H_

#define DEFAULT_CHAR_COLOR 0x07
#define TTY_NUM 3
#define TTY_IN_BYTES 256

/* VGA */
#define	CRTC_ADDR_REG	0x3D4	/* CRT Controller Registers - Addr Register */
#define	CRTC_DATA_REG	0x3D5	/* CRT Controller Registers - Data Register */
#define	START_ADDR_H	0xC	/* reg index of video mem start addr (MSB) */
#define	START_ADDR_L	0xD	/* reg index of video mem start addr (LSB) */
#define	CURSOR_H	0xE	/* reg index of cursor position (MSB) */
#define	CURSOR_L	0xF	/* reg index of cursor position (LSB) */
#define	V_MEM_BASE	0xB8000	/* base of color video memory */
#define	V_MEM_SIZE	0x8000	/* 32K: B8000H -> BFFFFH */

typedef struct s_console
{
    unsigned int current_start_addr;
    unsigned int original_addr;
    unsigned int v_mem_limit;
    unsigned int cursor;
} CONSOLE;

typedef struct s_tty
{
    unsigned char id;
    unsigned int in_buff[TTY_IN_BYTES];
    unsigned int* inbuf_head;
    unsigned int* inbuf_tail;
    int inbuf_count;

    CONSOLE *console;
} TTY;

TTY tty_create(unsigned char id);
void tty_input(TTY* tty, int content);
void tty_output(TTY* tty);
static void console_out_char(CONSOLE *console, char ch);
static void console_set_cursor(unsigned int position);

#endif