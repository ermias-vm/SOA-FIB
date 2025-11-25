/**
 * @file keyboard.h
 * @brief Keyboard device driver interface definitions for ZeOS.
 *
 * This header defines keyboard driver constants, key codes,
 * and keyboard management interfaces for the ZeOS kernel.
 */

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <types.h>

/* Forward declaration */
struct task_struct;

/* Keyboard event handler function type */
typedef void (*keyboard_handler_t)(char key, int pressed);

/* System call interface */
int sys_KeyboardEvent(void (*func)(char key, int pressed));

/* Interrupt handler for keyboard IRQ */
void keyboard_irq_handler(void);

/* Interrupt 0x2b handler for resuming execution */
void keyboard_resume_handler(void);

/* Internal keyboard management functions */
void init_keyboard(void);
void cleanup_keyboard_handler(struct task_struct *task);

/* Helper functions */
int setup_keyboard_aux_stack(struct task_struct *task);
void free_keyboard_aux_stack(struct task_struct *task);

#endif /* __KEYBOARD_H__ */
