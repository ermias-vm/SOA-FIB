/**
 * @file keyboard.h
 * @brief Keyboard device driver interface definitions for ZeOS.
 *
 * This header defines keyboard driver constants, structures,
 * and keyboard management interfaces for the ZeOS kernel.
 * The keyboard support allows user processes to register a callback
 * function that is executed whenever a key is pressed or released.
 */

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <types.h>

/* Forward declaration */
struct task_struct;

/* Keyboard hardware constants */
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

/* Auxiliary stack configuration for keyboard handler */
#define KBD_AUX_STACK_PAGES 1
#define KBD_AUX_STACK_SIZE (KBD_AUX_STACK_PAGES * PAGE_SIZE)

/* Virtual page for auxiliary keyboard stack (near end of address space) */
#define KBD_AUX_STACK_PAGE (TOTAL_PAGES - 3)

/**
 * @brief Initialize keyboard fields in a task structure.
 *
 * This function initializes all keyboard-related fields in a task_struct
 * to their default values (no handler registered).
 *
 * @param task Pointer to the task structure to initialize.
 */
void init_keyboard_fields(struct task_struct *task);

/**
 * @brief Setup auxiliary user stack for keyboard handler execution.
 *
 * Allocates a physical frame and maps it to a fixed virtual address
 * in the task's address space. This stack is used to execute the
 * user's keyboard callback function safely without corrupting the
 * normal user stack.
 *
 * @param task Pointer to the task structure.
 * @return 0 on success, -ENOMEM if no frames available.
 */
int setup_kbd_aux_stack(struct task_struct *task);

/**
 * @brief Free the auxiliary keyboard stack.
 *
 * Unmaps and frees the physical frame used for the auxiliary stack.
 *
 * @param task Pointer to the task structure.
 */
void free_kbd_aux_stack(struct task_struct *task);

/**
 * @brief Clean up all keyboard handler resources for a task.
 *
 * Resets all keyboard fields and frees the auxiliary stack.
 * Called when a process exits or when disabling keyboard events.
 *
 * @param task Pointer to the task structure.
 */
void cleanup_kbd_handler(struct task_struct *task);

/**
 * @brief Handle keyboard IRQ and dispatch to user handler.
 *
 * Called from the keyboard interrupt handler (IRQ 1). Reads the scancode,
 * determines if a key was pressed or released, and if the current task
 * has a registered handler, modifies the saved context to execute the
 * user's callback function on the auxiliary stack.
 *
 * The user function receives: key scancode (0-127) and pressed flag (1=pressed, 0=released).
 */
void kbd_irq_handler(void);

/**
 * @brief Handle int 0x2b to resume normal execution.
 *
 * Called when user code executes int 0x2b. If we are currently in a
 * keyboard handler context, restores the original EIP and ESP so
 * execution continues where it was interrupted.
 * If called outside keyboard context, does nothing.
 */
void kbd_resume_handler(void);

#endif /* __KEYBOARD_H__ */
