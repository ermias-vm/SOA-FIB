/*
 * interrupt.h - Definició de les diferents rutines de tractament d'exepcions
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>

#define IDT_ENTRIES 256

extern Gate idt[IDT_ENTRIES];
extern Register idtR;

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL);
void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

void setIdt();

void keyboard_handler();
void clock_handler();
void syscall_handler_sysenter();
void writeMSR(unsigned long msr, unsigned long val);
void pageFault_handler();

// Test functions
void testTaskSwitch(char key);
#endif /* __INTERRUPT_H__ */
