/**
 * @file io.h
 * @brief Input/Output interface definitions for ZeOS.
 *
 * This header defines I/O functions, screen management interfaces,
 * and console operations for the ZeOS kernel.
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>

/**************/
/** Colors  **/
/**************/

// Color attribute format (upper 8 bits): bits 8-11 text color, bits 12-15 background color
#define MAKE_COLOR(background, text) (((background & 0xF) << 12) | ((text & 0xF) << 8))

#define BLACK 0x0
#define BLUE 0x1
#define GREEN 0x2
#define CYAN 0x3
#define RED 0x4
#define MAGENTA 0x5
#define BROWN 0x6
#define LIGHT_GRAY 0x7
#define DARK_GRAY 0x8
#define LIGHT_BLUE 0x9
#define LIGHT_GREEN 0xA
#define LIGHT_CYAN 0xB
#define LIGHT_RED 0xC
#define LIGHT_MAGENTA 0xD
#define YELLOW 0xE
#define WHITE 0xF

#define DEFAULT_COLOR MAKE_COLOR(BLACK, GREEN)
#define ERROR_COLOR MAKE_COLOR(BLACK, LIGHT_RED)
#define WARNING_COLOR MAKE_COLOR(BLACK, YELLOW)
#define INFO_COLOR MAKE_COLOR(BLACK, LIGHT_BLUE)

/** Screen functions **/
/**********************/

/**
 * @brief Read byte from I/O port.
 *
 * This function reads a byte from the specified I/O port.
 * @param port I/O port number to read from.
 * @return Byte value read from the port.
 */
Byte inb(unsigned short port);

/**
 * @brief Print character with color.
 *
 * This function prints a single character to the screen at the current
 * cursor position with the specified color.
 * @param c Character to print.
 * @param color Color attribute for the character.
 */
void printc(char c, Word color);

/**
 * @brief Print character at specific position with color.
 *
 * This function prints a character at the specified screen coordinates
 * with the given color.
 * @param x X coordinate (column) on the screen.
 * @param y Y coordinate (row) on the screen.
 * @param c Character to print.
 * @param color Color attribute for the character.
 */
void printc_xy(Byte x, Byte y, char c, Word color);

/**
 * @brief Print string to screen.
 *
 * This function prints a null-terminated string to the screen at the
 * current cursor position using the default color.
 * @param string Null-terminated string to print.
 */
void printk(char *string);

/**
 * @brief Print string with color.
 *
 * This function prints a null-terminated string to the screen with
 * the specified color.
 * @param string Null-terminated string to print.
 * @param color Color attribute for the text.
 */
void printk_color(char *string, Word color);

#endif // __IO_H__
