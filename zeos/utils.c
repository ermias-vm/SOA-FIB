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
/* Copy from user space to kernel space, returns 0 if ok and -1 if
 * error*/
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
/* Copy from kernel space to user space, returns 0 if ok and -1 if
 * error*/
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

/* access_ok: Checks if a user space pointer is valid
 * @type:  Type of access: %VERIFY_READ or %VERIFY_WRITE. Note that
 *         %VERIFY_WRITE is a superset of %VERIFY_READ: if it is safe
 *         to write to a block, it is always safe to read from it
 * @addr:  User space pointer to start of block to check
 * @size:  Size of block to check
 * Returns true (nonzero) if the memory block may be valid,
 *         false (zero) if it is definitely invalid
 */
int access_ok(int type, const void *addr, unsigned long size) {
    unsigned long addr_ini, addr_fin;

    addr_ini = (((unsigned long)addr) >> 12);
    addr_fin = ((((unsigned long)addr) + size) >> 12);
    if (addr_fin < addr_ini) return 0; // This looks like an overflow ... deny access

    switch (type) {
    case VERIFY_WRITE:
        /* Should suppose no support for automodifyable code */
        if ((addr_ini >= USER_FIRST_PAGE) && (addr_fin <= USER_FIRST_PAGE + NUM_PAG_DATA)) return 1;
        /* fallthrough */
    default:
        if ((addr_ini >= USER_FIRST_PAGE) &&
            (addr_fin <= (USER_FIRST_PAGE + NUM_PAG_CODE + NUM_PAG_DATA)))
            return 1;
    }
    return 0;
}

#define CYCLESPERTICK 109000

/*
 * do_div() is NOT a C function. It wants to return
 * two values (the quotient and the remainder), but
 * since that doesn't work very well in C, what it
 * does is:
 *
 * - modifies the 64-bit dividend _in_place_
 * - returns the 32-bit remainder
 *
 * This ends up being the most efficient "calling
 * convention" on x86.
 */
#define do_div(n, base)                                                                            \
    ({                                                                                             \
        unsigned long __upper, __low, __high, __mod, __base;                                       \
        __base = (base);                                                                           \
        asm("" : "=a"(__low), "=d"(__high) : "A"(n));                                              \
        __upper = __high;                                                                          \
        if (__high) {                                                                              \
            __upper = __high % (__base);                                                           \
            __high = __high / (__base);                                                            \
        }                                                                                          \
        asm("divl %2" : "=a"(__low), "=d"(__mod) : "rm"(__base), "0"(__low), "1"(__upper));        \
        asm("" : "=A"(n) : "a"(__low), "d"(__high));                                               \
        __mod;                                                                                     \
    })

#define rdtsc(low, high) __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high))

unsigned long get_ticks(void) {
    unsigned long eax;
    unsigned long edx;
    unsigned long long ticks;

    rdtsc(eax, edx);

    ticks = ((unsigned long long)edx << 32) + eax;
    do_div(ticks, CYCLESPERTICK);

    return ticks;
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

void wait_ticks(int ticks_to_wait) {
    unsigned long start_ticks = get_ticks();
    while (get_ticks() - start_ticks < (unsigned long)ticks_to_wait) {
        __asm__ __volatile__("nop");
    }
}

void print_splash_screen(void) {
    printk("\n\n");

    // clang-format off
    //ZEOS
    printk_color("             ###########    ########  #######    ######       \n", MAKE_COLOR(BLACK, LIGHT_CYAN));
    printk_color("                    ##     ##        ##     ##  ##    ##      \n", MAKE_COLOR(BLACK, CYAN));
    printk_color("                  ##      ##        ##     ##  ##             \n", MAKE_COLOR(BLACK, LIGHT_BLUE));
    printk_color("                ##       ######    ##     ##   ######         \n", MAKE_COLOR(BLACK, BLUE));
    printk_color("              ##        ##        ##     ##        ##         \n", MAKE_COLOR(BLACK, LIGHT_BLUE));
    printk_color("            ##         ##        ##     ##  ##    ##          \n", MAKE_COLOR(BLACK, CYAN));
    printk_color("          ##########  ########   #######    ######            \n", MAKE_COLOR(BLACK, LIGHT_CYAN));
    printk_color("\n\n", DEFAULT_COLOR);
    // SOA-FIB
    printk_color("        ####    #####     ###        #######  ##  ######      \n", MAKE_COLOR(BLACK, YELLOW));
    printk_color("       ##  ##  ##   ##   ## ##       ##       ##  ##   ##     \n", MAKE_COLOR(BLACK, LIGHT_RED));
    printk_color("       ##      ##   ##  ##   ##      ##       ##  ##   ##     \n", MAKE_COLOR(BLACK, LIGHT_MAGENTA));
    printk_color("        ####   ##   ##  #######      #####    ##  ######      \n", MAKE_COLOR(BLACK, MAGENTA));
    printk_color("           ##  ##   ##  ##   ##      ##       ##  ##   ##     \n", MAKE_COLOR(BLACK, LIGHT_MAGENTA));
    printk_color("       ##  ##  ##   ##  ##   ##      ##       ##  ##   ##     \n", MAKE_COLOR(BLACK, LIGHT_RED));
    printk_color("        ####    #####   ##   ##      ##       ##  ######      \n", MAKE_COLOR(BLACK, YELLOW));
    
    printk("\n"); 
    
    // 2025-2026
    printk_color("                   =========================\n\n", MAKE_COLOR(BLACK, DARK_GRAY));
    printk_color("                       2 0 2 5", MAKE_COLOR(BLACK, LIGHT_GREEN));
    printk_color(" - ", MAKE_COLOR(BLACK, WHITE));
    printk_color("2 0 2 6\n\n", MAKE_COLOR(BLACK, LIGHT_GREEN));
    printk_color("                   =========================\n", MAKE_COLOR(BLACK, DARK_GRAY));
    // clang-format on
    printk("\n");
    wait_ticks(1500);
}
