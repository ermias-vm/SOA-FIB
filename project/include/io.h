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

/** Color attribute format (upper 8 bits): bits 8-11 text color, bits 12-15 background color */
#define MAKE_COLOR(background, text) (((background & 0xF) << 12) | ((text & 0xF) << 8))

/** VGA color codes */
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

/** Predefined color schemes */
#define DEFAULT_COLOR MAKE_COLOR(BLACK, GREEN)
#define ERROR_COLOR MAKE_COLOR(BLACK, LIGHT_RED)
#define WARNING_COLOR MAKE_COLOR(BLACK, YELLOW)
#define INFO_COLOR MAKE_COLOR(BLACK, LIGHT_BLUE)

/** Number of columns in VGA text mode */
#define NUM_COLUMNS 80

/** Number of rows in VGA text mode */
#define NUM_ROWS 25

/** Base address of VGA text mode video memory */
#define VIDEO_MEMORY_BASE 0xb8000

/** Current cursor column position (0-79) */
extern Byte x;

/** Current cursor row position (0-24) */
extern Byte y;

/**
 * @brief Read byte from I/O port.
 *
 * This function reads a byte from the specified I/O port using
 * inline assembly for low-level port access.
 *
 * @param port I/O port number to read from.
 * @return Byte value read from the port.
 */
Byte inb(unsigned short port);

/**
 * @brief Write character to specific screen position.
 *
 * This function writes a character with color attribute directly
 * to video memory at the specified coordinates.
 *
 * @param x X coordinate (column) on the screen (0-79).
 * @param y Y coordinate (row) on the screen (0-24).
 * @param c Character to write.
 * @param color Color attribute for the character.
 */
void write_char_to_screen(Byte x, Byte y, char c, Word color);

/**
 * @brief Scroll the screen content up by one line.
 *
 * This function moves all screen content up by one line, clearing
 * the bottom line. Used when cursor reaches the last row.
 */
void scroll_screen(void);

/**
 * @brief Handle newline character.
 *
 * This function handles the newline character by resetting the
 * column to 0 and incrementing the row. If the row exceeds the
 * screen height, it triggers a scroll operation.
 */
void handle_newline(void);

/**
 * @brief Print character with color.
 *
 * This function prints a single character to the screen at the current
 * cursor position with the specified color. Handles newline characters
 * and automatic line wrapping.
 *
 * @param c Character to print.
 * @param color Color attribute for the character.
 */
void printc(char c, Word color);

/**
 * @brief Print character at specific position with color.
 *
 * This function prints a character at the specified screen coordinates
 * with the given color, then restores the cursor to its original position.
 *
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
 *
 * @param string Null-terminated string to print.
 */
void printk(char *string);

/**
 * @brief Print string with color.
 *
 * This function prints a null-terminated string to the screen with
 * the specified color.
 *
 * @param string Null-terminated string to print.
 * @param color Color attribute for the text.
 */
void printk_color(char *string, Word color);

/**
 * @brief Clear the screen.
 *
 * This function clears the entire screen by filling it with spaces
 * and resets the cursor position to the top-left corner (0, 0).
 */
void clear_screen(void);

/**
 * @brief Print formatted string with color.
 *
 * This function prints a formatted string to the screen with
 * the specified color. Supports format specifiers:
 *   - %d: Integer (decimal)
 *   - %x: Hexadecimal
 *   - %s: String
 *   - %c: Character
 *   - %%: Literal percent sign
 *
 * @param color Color attribute for the text.
 * @param fmt Format string.
 * @param ... Variable arguments matching format specifiers.
 */
void printk_color_fmt(Word color, char *fmt, ...);

#endif /* __IO_H__ */
