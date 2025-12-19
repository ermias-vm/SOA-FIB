/**
 * @file devices.c
 * @brief Device driver implementations for ZeOS.
 *
 * This file contains device-specific functions for system I/O
 * operations, particularly console output device management.
 */

#include <io.h>
#include <list.h>
#include <utils.h>

int sys_write_console(char *buffer, int size) {
    int i;

    for (i = 0; i < size; i++) {
        printc(buffer[i], DEFAULT_COLOR);
    }

    return size;
}

int sys_write_debug(char *buffer, int size) {
    int i;

    /* Write only to Bochs debug port (0xe9) - terminal only, not game screen */
    for (i = 0; i < size; i++) {
        __asm__ __volatile__("movb %0, %%al; outb $0xe9" ::"a"(buffer[i]));
    }

    return size;
}
