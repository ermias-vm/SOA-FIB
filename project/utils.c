/**
 * @file utils.c
 * @brief Utility functions and helper routines for ZeOS.
 *
 * This file contains general-purpose utility functions including
 * memory operations, copying routines, and system helper functions.
 */

#include <io.h>
#include <mm_address.h>
#include <types.h>
#include <utils.h>

void copy_data(void *start, void *dest, int size) {
    DWord *p = start, *q = dest;
    Byte *p1, *q1;
    while (size > 4) {
        *q++ = *p++;
        size -= 4;
    }
    p1 = (Byte *)p;
    q1 = (Byte *)q;
    while (size > 0) {
        *q1++ = *p1++;
        size--;
    }
}

int copy_from_user(void *start, void *dest, int size) {
    DWord *p = start, *q = dest;
    Byte *p1, *q1;
    while (size > 4) {
        *q++ = *p++;
        size -= 4;
    }
    p1 = (Byte *)p;
    q1 = (Byte *)q;
    while (size > 0) {
        *q1++ = *p1++;
        size--;
    }
    return 0;
}

int copy_to_user(void *start, void *dest, int size) {
    DWord *p = start, *q = dest;
    Byte *p1, *q1;
    while (size > 4) {
        *q++ = *p++;
        size -= 4;
    }
    p1 = (Byte *)p;
    q1 = (Byte *)q;
    while (size > 0) {
        *q1++ = *p1++;
        size--;
    }
    return 0;
}

int access_ok(int type, const void *addr, unsigned long size) {
    unsigned long addr_ini, addr_fin;

    addr_ini = (((unsigned long)addr) >> 12);
    addr_fin = ((((unsigned long)addr) + size) >> 12);
    if (addr_fin < addr_ini) return 0; // This looks like an overflow ... deny access

    unsigned long max_user_page = TOTAL_PAGES;

    switch (type) {
    case VERIFY_WRITE:
        if ((addr_ini >= USER_FIRST_PAGE) && (addr_fin < max_user_page)) return 1;
        break;
    default:
        if ((addr_ini >= USER_FIRST_PAGE) && (addr_fin < max_user_page)) return 1;
    }
    return 0;
}

unsigned long get_ticks(void) {
    unsigned long eax;
    unsigned long edx;
    unsigned long long ticks;

    rdtsc(eax, edx);

    ticks = ((unsigned long long)edx << 32) + eax;
    do_div(ticks, CYCLESPERTICK);

    return ticks;
}

void wait_ticks(int ticks_to_wait) {
    unsigned long start_ticks = get_ticks();
    while (get_ticks() - start_ticks < (unsigned long)ticks_to_wait) {
        __asm__ __volatile__("nop");
    }
}

void itoa_hex(unsigned int num, char *buffer) {
    const char hex_chars[] = "0123456789ABCDEF";
    int i = 7;

    buffer[0] = '0';
    buffer[1] = 'x';

    for (i = 9; i >= 2; i--) {
        buffer[i] = hex_chars[num & 0xF];
        num >>= 4;
    }

    buffer[10] = '\0';
}

void utoa(unsigned int a, char *b) {
    int i, i1;
    char c;

    if (a == 0) {
        b[0] = '0';
        b[1] = 0;
        return;
    }

    i = 0;
    while (a > 0) {
        b[i] = (a % 10) + '0';
        a = a / 10;
        i++;
    }

    for (i1 = 0; i1 < i / 2; i1++) {
        c = b[i1];
        b[i1] = b[i - i1 - 1];
        b[i - i1 - 1] = c;
    }
    b[i] = 0;
}

void print_splash_screen(void) {
    clear_screen();
    printk("\n");
    // clang-format off
    //ZEOS
    printk_color("                  ###########    ########  #######    ######       \n", MAKE_COLOR(BLACK, LIGHT_CYAN));
    printk_color("                         ##     ##        ##     ##  ##    ##      \n", MAKE_COLOR(BLACK, CYAN));
    printk_color("                       ##      ##        ##     ##  ##             \n", MAKE_COLOR(BLACK, LIGHT_BLUE));
    printk_color("                     ##       ######    ##     ##   ######         \n", MAKE_COLOR(BLACK, BLUE));
    printk_color("                   ##        ##        ##     ##        ##         \n", MAKE_COLOR(BLACK, LIGHT_BLUE));
    printk_color("                 ##         ##        ##     ##  ##    ##          \n", MAKE_COLOR(BLACK, CYAN));
    printk_color("               ##########  ########   #######    ######            \n", MAKE_COLOR(BLACK, LIGHT_CYAN));
    printk("\n\n");
    // SOA-FIB
    printk_color("               ####    #####     ###        #######  ##  ######      \n", MAKE_COLOR(BLACK, YELLOW));
    printk_color("              ##  ##  ##   ##   ## ##       ##       ##  ##   ##     \n", MAKE_COLOR(BLACK, LIGHT_RED));
    printk_color("              ##      ##   ##  ##   ##      ##       ##  ##   ##     \n", MAKE_COLOR(BLACK, LIGHT_MAGENTA));
    printk_color("               ####   ##   ##  #######      #####    ##  ######      \n", MAKE_COLOR(BLACK, MAGENTA));
    printk_color("                  ##  ##   ##  ##   ##      ##       ##  ##   ##     \n", MAKE_COLOR(BLACK, LIGHT_MAGENTA));
    printk_color("              ##  ##  ##   ##  ##   ##      ##       ##  ##   ##     \n", MAKE_COLOR(BLACK, LIGHT_RED));
    printk_color("               ####    #####   ##   ##      ##       ##  ######      \n\n", MAKE_COLOR(BLACK, YELLOW));
    
    // 2025-2026
    printk_color("                        =========================\n", MAKE_COLOR(BLACK, DARK_GRAY));
    printk_color("                            2 0 2 5", MAKE_COLOR(BLACK, LIGHT_GREEN));
    printk_color(" - ", MAKE_COLOR(BLACK, WHITE));
    printk_color("2 0 2 6\n", MAKE_COLOR(BLACK, LIGHT_GREEN));
    printk_color("                        =========================\n\n", MAKE_COLOR(BLACK, DARK_GRAY));
    printk_color("                        Booting ZeOs by Baka Baka\n", WARNING_COLOR);
    // clang-format on
    wait_ticks(2500);
}
