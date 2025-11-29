/**
 * @file interrupt.h
 * @brief Interrupt handling and IDT management definitions for ZeOS.
 *
 * This header defines interrupt descriptors, handler function prototypes,
 * and interrupt management interfaces for the ZeOS kernel.
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>

/** Maximum number of entries in the Interrupt Descriptor Table */
#define IDT_ENTRIES 256

/** Interrupt Descriptor Table - array of interrupt/trap gates */
extern Gate idt[IDT_ENTRIES];

/** IDT register structure for loading the IDT */
extern Register idtR;

/** Global system tick counter - incremented on each clock interrupt */
extern int zeos_ticks;

/** Keyboard scancode to ASCII character mapping table */
extern char char_map[];

/**
 * @brief Set an interrupt handler in the IDT.
 *
 * Configures an interrupt gate in the Interrupt Descriptor Table for
 * handling hardware interrupts. The handler will be called when the
 * corresponding interrupt occurs.
 *
 * @param vector Interrupt vector number (0-255)
 * @param handler Pointer to the interrupt handler function
 * @param maxAccessibleFromPL Maximum privilege level that can access this interrupt
 */
void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

/**
 * @brief Set a trap handler in the IDT.
 *
 * Configures a trap gate in the Interrupt Descriptor Table for
 * handling exceptions and software interrupts. Unlike interrupt gates,
 * trap gates don't automatically disable interrupts.
 *
 * @param vector Trap vector number (0-255)
 * @param handler Pointer to the trap handler function
 * @param maxAccessibleFromPL Maximum privilege level that can access this trap
 */
void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

/**
 * @brief Initialize the Interrupt Descriptor Table.
 *
 * Sets up the complete IDT with all necessary interrupt handlers,
 * exception handlers, and system call entry points. Configures
 * the IDTR register to point to the IDT.
 */
void setIdt(void);

/**
 * @brief Keyboard interrupt routine.
 *
 * Called from the keyboard handler when a key event occurs.
 * Reads the scancode from port 0x60 and prints the corresponding
 * character to the screen.
 */
void keyboard_routine(void);

/**
 * @brief Clock interrupt routine.
 *
 * Called on each timer tick (IRQ 0). Increments the global tick counter,
 * updates the clock display, and invokes the scheduler.
 */
void clock_routine(void);

/**
 * @brief Page fault exception routine.
 *
 * Called when a page fault exception occurs. Attempts to grow the user
 * stack if the fault is within the stack region. Otherwise, displays
 * an error message and terminates the process.
 *
 * @param eip Instruction pointer where the fault occurred
 * @param fault_addr Memory address that caused the fault
 */
void pageFault_routine(unsigned int eip, unsigned int fault_addr);

/**
 * @brief Write value to Model Specific Register.
 *
 * Assembly routine to write a value to a specific MSR (Model Specific Register).
 * Used for configuring processor-specific features like SYSENTER/SYSEXIT.
 * Implemented in kernel_asm.S.
 *
 * @param msr MSR number to write to
 * @param val Value to write to the MSR
 */
extern void writeMSR(unsigned long msr, unsigned long val);

#endif /* __INTERRUPT_H__ */
