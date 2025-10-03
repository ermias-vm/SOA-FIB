/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>

/**************/
/** Colors  **/
/**************/

// Color attribute format (upper 8 bits): bits 8-11 text color, bits 12-15 background color
#define MAKE_COLOR(background, text) (((background & 0xF) << 12) | ((text & 0xF) << 8))

#define BLACK         0x0
#define BLUE          0x1
#define GREEN         0x2
#define CYAN          0x3
#define RED           0x4
#define MAGENTA       0x5
#define BROWN         0x6
#define LIGHT_GRAY    0x7
#define DARK_GRAY     0x8
#define LIGHT_BLUE    0x9
#define LIGHT_GREEN   0xA
#define LIGHT_CYAN    0xB
#define LIGHT_RED     0xC
#define LIGHT_MAGENTA 0xD
#define YELLOW        0xE
#define WHITE         0xF

#define DEFAULT_COLOR MAKE_COLOR(BLACK, GREEN)
#define ERROR_COLOR MAKE_COLOR(BLACK, LIGHT_RED)
#define WARNING_COLOR MAKE_COLOR(BLACK, YELLOW)
#define INFO_COLOR MAKE_COLOR(BLACK, LIGHT_BLUE)




/** Screen functions **/
/**********************/

Byte inb (unsigned short port);
void printc(char c, Word color);
void printc_xy(Byte x, Byte y, char c, Word color);
void printk(char *string);

void printk_color(char *string, Word color);

#endif // __IO_H__
