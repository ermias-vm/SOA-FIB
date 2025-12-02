/**
 * @file io.c
 * @brief Input/Output operations and screen management for ZeOS.
 *
 * This file implements low-level I/O operations including port access,
 * VGA text mode screen management, character printing, and cursor control
 * for 80x25 character resolution display.
 */

#include <io.h>
#include <libc.h>
#include <types.h>
#include <utils.h>

/* Current cursor column position (0-79) */
Byte x = 0;

/* Current cursor row position (starts at row 19) */
Byte y = 19;

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

void scroll_screen(void) {
    Word *screen = (Word *)VIDEO_MEMORY_BASE;
    for (int i = 0; i < (NUM_ROWS - 1) * NUM_COLUMNS; i++) {
        screen[i] = screen[i + NUM_COLUMNS];
    }
    for (int i = (NUM_ROWS - 1) * NUM_COLUMNS; i < NUM_ROWS * NUM_COLUMNS; i++) {
        screen[i] = (Word)' ' | DEFAULT_COLOR;
    }
    y = NUM_ROWS - 1;
}

void handle_newline(void) {
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

void clear_screen(void) {
    Word *screen = (Word *)VIDEO_MEMORY_BASE;
    for (int i = 0; i < NUM_ROWS * NUM_COLUMNS; i++) {
        screen[i] = (Word)' ' | DEFAULT_COLOR;
    }
    x = y = 0;
}

void print_string_xy(Byte px, Byte py, const char *str, Word color) {
    Word *screen = (Word *)VIDEO_MEMORY_BASE;
    int pos = py * NUM_COLUMNS + px;

    for (int i = 0; str[i] != '\0' && (px + i) < NUM_COLUMNS; i++) {
        screen[pos + i] = (Word)(str[i] & 0x00FF) | color;
    }
}

void printk_color_fmt(Word color, char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    char buffer[32];
    char *str;
    int num;
    unsigned int unum;

    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%') {
            printc(fmt[i], color);
            continue;
        }

        i++; // Skip '%'
        switch (fmt[i]) {
        case 'd': // Integer
            num = __builtin_va_arg(args, int);
            if (num < 0) {
                printc('-', color);
                unum = -(unsigned int)num;
            } else {
                unum = (unsigned int)num;
            }
            utoa(unum, buffer);
            printk_color(buffer, color);
            break;
        case 'x': // Hexadecimal
            num = __builtin_va_arg(args, int);
            itoa_hex(num, buffer);
            printk_color(buffer, color);
            break;
        case 's': // String
            str = __builtin_va_arg(args, char *);
            printk_color(str, color);
            break;
        case 'c':                              // Character
            num = __builtin_va_arg(args, int); // char is promoted to int
            printc((char)num, color);
            break;
        case '%': // Escaped %
            printc('%', color);
            break;
        default: // Unknown format, print as is
            printc('%', color);
            printc(fmt[i], color);
            break;
        }
    }

    __builtin_va_end(args);
}
