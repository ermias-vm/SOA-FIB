/*
 * io.c -
 */

#include <io.h>
#include <types.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS 25
#define VIDEO_MEMORY_BASE 0xb8000

Byte x = 0;
Byte y = 19;

/* Read a byte from 'port' */
Byte inb(unsigned short port) {
    Byte v;

    __asm__ __volatile__("inb %w1,%0" : "=a"(v) : "Nd"(port));
    return v;
}

void write_char_to_screen(Byte x, Byte y, char c, Word color) {
    Word ch = (Word)(c & 0x00FF) | color;
    Word *screen = (Word *)VIDEO_MEMORY_BASE;
    screen[(y * NUM_COLUMNS + x)] = ch;
}

// TODO: refactor this function to make it more efficient
void scroll_screen() {
    Word *screen = (Word *)VIDEO_MEMORY_BASE;
    for (int i = 0; i < (NUM_ROWS - 1) * NUM_COLUMNS; i++) {
        screen[i] = screen[i + NUM_COLUMNS];
    }
    for (int i = (NUM_ROWS - 1) * NUM_COLUMNS; i < NUM_ROWS * NUM_COLUMNS; i++) {
        screen[i] = (Word)' ' | DEFAULT_COLOR;
    }
    y = NUM_ROWS - 1;
}

void handle_newline() {
    x = 0;
    if (++y >= NUM_ROWS) scroll_screen();
}

void printc(char c, Word color) {
    __asm__ __volatile__(
        "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */

    if (c == '\n')
        handle_newline();
    else {
        write_char_to_screen(x, y, c, color);
        if (++x >= NUM_COLUMNS) handle_newline();
    }
}

void printc_xy(Byte mx, Byte my, char c, Word color) {
    Byte cx, cy;
    cx = x;
    cy = y;
    x = mx;
    y = my;
    printc(c, color);
    x = cx;
    y = cy;
}

void printk(char *string) {
    for (int i = 0; string[i]; i++) {
        printc(string[i], DEFAULT_COLOR);
    }
}

void printk_color(char *string, Word color) {
    for (int i = 0; string[i]; i++) {
        printc(string[i], color);
    }
}
