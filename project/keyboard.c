/**
 * @file keyboard.c
 * @brief Keyboard device driver implementation for ZeOS.
 *
 * This file implements the keyboard support functionality including
 * auxiliary stack management, IRQ handling, and user callback dispatch.
 */

#include <errno.h>
#include <io.h>
#include <keyboard.h>
#include <mm.h>
#include <mm_address.h>
#include <sched.h>
#include <segment.h>
#include <sys.h>


void init_keyboard_fields(struct task_struct *task) {
    task->kbd_handler = NULL;
    task->kbd_wrapper = NULL;
    task->kbd_aux_stack = NULL;
    task->in_kbd_context = 0;
    for (int i = 0; i < KBD_CTX_SIZE; i++) {
        task->kbd_saved_ctx[i] = 0;
    }
}

int setup_kbd_aux_stack(struct task_struct *task) {
    if (task->kbd_aux_stack != NULL) {
        return 0; /* Already allocated */
    }

    /* Allocate physical frame for auxiliary stack */
    int frame = alloc_frame();
    if (frame < 0) {
        return -ENOMEM;
    }

    /* Map the frame to a fixed virtual page in user space */
    page_table_entry *PT = get_PT(task);
    set_ss_pag(PT, KBD_AUX_STACK_PAGE, frame);

    /* Flush TLB */
    set_cr3(get_DIR(task));

    /* Stack top is at the end of the page (stacks grow downward) */
    task->kbd_aux_stack = (void *)((KBD_AUX_STACK_PAGE + 1) << 12);

    return 0;
}

void free_kbd_aux_stack(struct task_struct *task) {
    if (task->kbd_aux_stack == NULL) {
        return;
    }

    /* Get the page table and free the frame */
    page_table_entry *PT = get_PT(task);
    int frame = get_frame(PT, KBD_AUX_STACK_PAGE);

    if (frame >= 0) {
        free_frame(frame);
        del_ss_pag(PT, KBD_AUX_STACK_PAGE);
        set_cr3(get_DIR(task));
    }

    task->kbd_aux_stack = NULL;
}

void cleanup_kbd_handler(struct task_struct *task) {
    task->kbd_handler = NULL;
    task->in_kbd_context = 0;
    for (int i = 0; i < KBD_CTX_SIZE; i++) {
        task->kbd_saved_ctx[i] = 0;
    }
    free_kbd_aux_stack(task);
}

void kbd_irq_handler(void) {
    /* Read scancode from keyboard data port */
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);

    /* Determine if key was pressed (bit 7 = 0) or released (bit 7 = 1) */
    int pressed = !(scancode & 0x80);
    char key = scancode & 0x7F;

    struct task_struct *task = current_task;

    /* Check if current task has a keyboard handler and is not already in handler */
    if (task == NULL || task->kbd_handler == NULL || task->in_kbd_context) {
        return;
    }

    /* Mark that we're entering keyboard handler context */
    task->in_kbd_context = 1;

    /* Get the task's kernel stack to modify saved context */
    union task_union *task_union = (union task_union *)task;

    /*
     * Save the complete context (SW + HW) to restore later.
     * This includes all registers saved by SAVE_ALL plus the hardware context.
     */
    task->kbd_saved_ctx[0] = task_union->stack[STACK_EBX];
    task->kbd_saved_ctx[1] = task_union->stack[STACK_ECX];
    task->kbd_saved_ctx[2] = task_union->stack[STACK_EDX];
    task->kbd_saved_ctx[3] = task_union->stack[STACK_ESI];
    task->kbd_saved_ctx[4] = task_union->stack[STACK_EDI];
    task->kbd_saved_ctx[5] = task_union->stack[STACK_EBP];
    task->kbd_saved_ctx[6] = task_union->stack[STACK_EAX];
    task->kbd_saved_ctx[7] = task_union->stack[STACK_DS];
    task->kbd_saved_ctx[8] = task_union->stack[STACK_ES];
    task->kbd_saved_ctx[9] = task_union->stack[STACK_FS];
    task->kbd_saved_ctx[10] = task_union->stack[STACK_GS];
    task->kbd_saved_ctx[11] = task_union->stack[STACK_USER_EIP];
    task->kbd_saved_ctx[12] = task_union->stack[STACK_USER_CS];
    task->kbd_saved_ctx[13] = task_union->stack[STACK_EFLAGS];
    task->kbd_saved_ctx[14] = task_union->stack[STACK_USER_ESP];
    task->kbd_saved_ctx[15] = task_union->stack[STACK_USER_SS];

    /*
     * Setup auxiliary stack for the wrapper function.
     * Stack layout (growing downward):
     *   aux_stack_top - 4:  pressed (3rd param for wrapper)
     *   aux_stack_top - 8:  key (2nd param for wrapper)
     *   aux_stack_top - 12: kbd_handler (1st param for wrapper)
     *   aux_stack_top - 16: return address (unused, wrapper does int 0x2b)
     *
     * The wrapper will be called with these parameters on stack.
     */
    unsigned long aux_stack_top = (unsigned long)task->kbd_aux_stack;
    unsigned long *stack_ptr = (unsigned long *)(aux_stack_top - 16);

    stack_ptr[0] = 0;                                /* Return address (unused) */
    stack_ptr[1] = (unsigned long)task->kbd_handler; /* 1st param: user handler */
    stack_ptr[2] = (unsigned long)key;               /* 2nd param: key scancode */
    stack_ptr[3] = (unsigned long)pressed;           /* 3rd param: pressed flag */

    /* Modify saved context to jump to wrapper function on auxiliary stack */
    task_union->stack[STACK_USER_EIP] = (unsigned long)task->kbd_wrapper;
    task_union->stack[STACK_USER_ESP] = (unsigned long)stack_ptr;
}

void kbd_resume_handler(void) {
    struct task_struct *task = current_task;

    /* If not in keyboard handler context, do nothing */
    if (task == NULL || !task->in_kbd_context) {
        return;
    }

    /* Get the task's kernel stack */
    union task_union *task_union = (union task_union *)task;

    /* Restore the complete saved context (SW + HW) */
    task_union->stack[STACK_EBX] = task->kbd_saved_ctx[0];
    task_union->stack[STACK_ECX] = task->kbd_saved_ctx[1];
    task_union->stack[STACK_EDX] = task->kbd_saved_ctx[2];
    task_union->stack[STACK_ESI] = task->kbd_saved_ctx[3];
    task_union->stack[STACK_EDI] = task->kbd_saved_ctx[4];
    task_union->stack[STACK_EBP] = task->kbd_saved_ctx[5];
    task_union->stack[STACK_EAX] = task->kbd_saved_ctx[6];
    task_union->stack[STACK_DS] = task->kbd_saved_ctx[7];
    task_union->stack[STACK_ES] = task->kbd_saved_ctx[8];
    task_union->stack[STACK_FS] = task->kbd_saved_ctx[9];
    task_union->stack[STACK_GS] = task->kbd_saved_ctx[10];
    task_union->stack[STACK_USER_EIP] = task->kbd_saved_ctx[11];
    task_union->stack[STACK_USER_CS] = task->kbd_saved_ctx[12];
    task_union->stack[STACK_EFLAGS] = task->kbd_saved_ctx[13];
    task_union->stack[STACK_USER_ESP] = task->kbd_saved_ctx[14];
    task_union->stack[STACK_USER_SS] = task->kbd_saved_ctx[15];

    /* Clear keyboard handler flag */
    task->in_kbd_context = 0;
}
