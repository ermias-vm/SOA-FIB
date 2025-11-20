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
