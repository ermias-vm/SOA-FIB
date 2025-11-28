
/**
 * @file entry.h
 * @brief Low-level entry points and assembly macro definitions for ZeOS.
 *
 * This header defines assembly entry points for interrupt handlers, system calls,
 * and context switching macros used throughout the ZeOS kernel.
 */

#ifndef __ENTRY_H__
#define __ENTRY_H__

/**
 * @brief SYSENTER system call entry point handler
 *
 * Fast system call entry point using Intel's SYSENTER instruction.
 * Handles user-to-kernel mode transition, validates system call number,
 * calls the appropriate system call handler, and returns to user mode
 * using SYSEXIT. Implemented in entry.S.
 */
extern void syscall_handler_sysenter();

/**
 * @brief Assembly clock interrupt handler
 *
 * Low-level clock timer interrupt handler that saves all registers,
 * sends EOI to PIC immediately, calls clock_routine for scheduling,
 * and restores registers. Implemented in entry.S.
 */
extern void clock_handler();

/**
 * @brief Assembly keyboard interrupt handler
 *
 * Low-level keyboard interrupt handler that saves all registers,
 * calls the keyboard_routine to process the key press, sends
 * End Of Interrupt signal to the PIC, and restores registers.
 * Implemented in entry.S.
 */
extern void keyboard_handler();

/**
 * @brief Page fault exception handler
 *
 * Low-level page fault handler that saves all registers, pushes the
 * faulting instruction pointer, calls pageFault_routine for handling,
 * and attempts to restore registers (though typically doesn't return).
 * Implemented in entry.S.
 */
extern void pageFault_handler();

/**
 * @brief Keyboard IRQ entry point for user keyboard events.
 *
 * Low-level keyboard interrupt handler that saves context, calls
 * kbd_irq_handler to dispatch to user callback if registered,
 * sends EOI, and restores context. Implemented in entry.S.
 */
extern void kbd_irq_entry();

/**
 * @brief Int 0x2b entry point to resume from keyboard handler.
 *
 * Low-level handler for int 0x2b that saves context, calls
 * kbd_resume_handler to restore original execution context,
 * and returns. Implemented in entry.S.
 */
extern void kbd_resume_entry();

#endif /* __ENTRY_H__ */
