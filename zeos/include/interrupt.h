/*
 * interrupt.h - Definició de les diferents rutines de tractament d'exepcions
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>

#define IDT_ENTRIES 256

extern Gate idt[IDT_ENTRIES];
extern Register idtR;

/**
 * @brief Set an interrupt handler in the IDT
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
 * @brief Set a trap handler in the IDT
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
 * @brief Initialize the Interrupt Descriptor Table
 *
 * Sets up the complete IDT with all necessary interrupt handlers,
 * exception handlers, and system call entry points. Configures
 * the IDTR register to point to the IDT.
 */
void setIdt();

/**
 * @brief Assembly keyboard interrupt handler
 *
 * Low-level keyboard interrupt handler that saves all registers,
 * calls the keyboard_routine to process the key press, sends
 * End Of Interrupt signal to the PIC, and restores registers.
 * Implemented in entry.S.
 */
void keyboard_handler();

/**
 * @brief Assembly clock interrupt handler
 *
 * Low-level clock timer interrupt handler that saves all registers,
 * sends EOI to PIC immediately, calls clock_routine for scheduling,
 * and restores registers. Implemented in entry.S.
 */
void clock_handler();

/**
 * @brief SYSENTER system call entry point handler
 *
 * Fast system call entry point using Intel's SYSENTER instruction.
 * Handles user-to-kernel mode transition, validates system call number,
 * calls the appropriate system call handler, and returns to user mode
 * using SYSEXIT. Implemented in entry.S.
 */
void syscall_handler_sysenter();

/**
 * @brief Write value to Model Specific Register
 *
 * Assembly routine to write a value to a specific MSR (Model Specific Register).
 * Used for configuring processor-specific features like SYSENTER/SYSEXIT.
 * Implemented in kernel_asm.S.
 *
 * @param msr MSR number to write to
 * @param val Value to write to the MSR
 */
void writeMSR(unsigned long msr, unsigned long val);

/**
 * @brief Page fault exception handler
 *
 * Low-level page fault handler that saves all registers, pushes the
 * faulting instruction pointer, calls pageFault_routine for handling,
 * and attempts to restore registers (though typically doesn't return).
 * Implemented in entry.S.
 */
void pageFault_handler();

/**
 * @brief Test function for task switching functionality
 *
 * Test function that demonstrates task switching capabilities based on
 * keyboard input. Used for debugging and testing the scheduler.
 *
 * @param key Character key pressed that triggers the test
 */
void testTaskSwitch(char key);
#endif /* __INTERRUPT_H__ */
