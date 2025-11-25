#include <keyboard.h>
#include <sched.h>
#include <mm.h>
#include <mm_address.h>
#include <io.h>
#include <interrupt.h>
#include <errno.h>
#include <utils.h>
#include <segment.h>

#define KEYBOARD_IRQ 1
#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define KEYBOARD_AUX_STACK_PAGES 1
#define KEYBOARD_AUX_STACK_SIZE (KEYBOARD_AUX_STACK_PAGES * PAGE_SIZE)

/* Stack frame offsets - same as in sys.c */
#define STACK_USER_EIP (KERNEL_STACK_SIZE - 5)
#define STACK_USER_ESP (KERNEL_STACK_SIZE - 2)
#define STACK_EAX (KERNEL_STACK_SIZE - 10)
#define STACK_EBP (KERNEL_STACK_SIZE - 11)

/* Libc wrapper function that will be called instead of user function */
extern void __kbd_wrapper_start;
extern void __kbd_wrapper_end;

void init_keyboard(void) {
    /* Initialize keyboard interrupt handler */
    /* This will be called from system initialization */
}

int setup_keyboard_aux_stack(struct task_struct *task) {
    if (task->kbd_aux_stack != NULL) {
        return 0; /* Already allocated */
    }

    /* Allocate physical frame for auxiliary stack */
    int frame = alloc_frame();
    if (frame < 0) {
        return -ENOMEM;
    }

    /* Find a free virtual page in user space for the auxiliary stack */
    /* Use a fixed virtual address range for auxiliary stacks */
    unsigned int aux_stack_page = 0x8000;  /* Example: use page at 0x8000000 */
    
    /* Map the frame to user space */
    page_table_entry *PT = get_PT(task);
    set_ss_pag(PT, aux_stack_page, frame);
    set_cr3(get_DIR(task));

    /* Calculate stack top (stacks grow downward) */
    task->kbd_aux_stack = (void *)((aux_stack_page + 1) << 12);

    return 0;
}

void free_keyboard_aux_stack(struct task_struct *task) {
    if (task->kbd_aux_stack == NULL) {
        return;
    }

    /* Get the page number */
    unsigned int aux_page = (((unsigned long)task->kbd_aux_stack) >> 12) - 1;
    
    /* Get the physical frame and free it */
    page_table_entry *PT = get_PT(task);
    int frame = get_frame(PT, aux_page);
    
    if (frame >= 0) {
        free_frame(frame);
        del_ss_pag(PT, aux_page);
        set_cr3(get_DIR(task));
    }

    task->kbd_aux_stack = NULL;
}

void cleanup_keyboard_handler(struct task_struct *task) {
    task->kbd_handler = NULL;
    task->kbd_in_handler = 0;
    task->kbd_saved_esp = 0;
    task->kbd_saved_eip = 0;
    free_keyboard_aux_stack(task);
}

int sys_KeyboardEvent(void (*func)(char key, int pressed)) {
    struct task_struct *task = current_task;

    /* Validate user function pointer - use a simpler check for now */
    if (func != NULL) {
        /* Basic validation: check if it's in user space */
        if ((unsigned long)func < 0x1000000) { /* Adjust according to your memory layout */
            return -EFAULT;
        }
    }

    /* If func is NULL, disable keyboard events */
    if (func == NULL) {
        cleanup_keyboard_handler(task);
        return 0;
    }

    /* Setup auxiliary stack */
    int ret = setup_keyboard_aux_stack(task);
    if (ret < 0) {
        return ret;
    }

    /* Register the handler */
    task->kbd_handler = func;
    task->kbd_in_handler = 0;

    return 0;
}

void keyboard_irq_handler(void) {
    /* Read scancode from keyboard */
    unsigned char scancode = inb(KEYBOARD_DATA_PORT);
    
    /* Determine if key is pressed or released */
    int pressed = !(scancode & 0x80);
    char key = scancode & 0x7F;

    struct task_struct *task = current_task;
    
    /* Check if current task has a keyboard handler registered */
    if (task == NULL || task->kbd_handler == NULL || task->kbd_in_handler) {
        return; /* No handler or already in handler */
    }

    /* Mark that we're entering keyboard handler */
    task->kbd_in_handler = 1;

    /* Get current saved context from kernel stack */
    union task_union *task_union = (union task_union *)task;
    
    /* Save original EIP and ESP - use the same constants as in your sys.c */
    task->kbd_saved_eip = task_union->stack[STACK_USER_EIP];
    task->kbd_saved_esp = task_union->stack[STACK_USER_ESP];

    /* Setup auxiliary stack for parameters */
    unsigned long aux_stack_top = (unsigned long)task->kbd_aux_stack;
    unsigned long new_esp = aux_stack_top - 4 * sizeof(unsigned long);

    /* Set up parameters on auxiliary stack */
    unsigned long *stack_params = (unsigned long *)(new_esp);
    stack_params[0] = 0;                           /* Return address (dummy) */
    stack_params[1] = (unsigned long)key;          /* First parameter: key */
    stack_params[2] = (unsigned long)pressed;      /* Second parameter: pressed */
    
    /* Modify saved context to call user function */
    task_union->stack[STACK_USER_EIP] = (unsigned long)task->kbd_handler;
    task_union->stack[STACK_USER_ESP] = new_esp;
    
    /* Clear EAX register (return value) */
    task_union->stack[STACK_EAX] = 0;
}

void keyboard_resume_handler(void) {
    struct task_struct *task = current_task;
    
    /* Check if we're actually in a keyboard handler */
    if (!task || !task->kbd_in_handler) {
        return; /* int 0x2b called outside keyboard context - do nothing */
    }

    /* Restore original execution context */
    union task_union *task_union = (union task_union *)task;
    
    task_union->stack[STACK_USER_EIP] = task->kbd_saved_eip;
    task_union->stack[STACK_USER_ESP] = task->kbd_saved_esp;

    /* Clear keyboard handler flag */
    task->kbd_in_handler = 0;
}