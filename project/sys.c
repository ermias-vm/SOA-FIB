/**
 * @file sys.c
 * @brief System call implementations for ZeOS.
 *
 * This file contains the kernel-side implementation of system calls
 * including process management (fork, exit), I/O operations (write),
 * process synchronization (block, unblock), and system information.
 */
#include <debug.h>
#include <devices.h>
#include <errno.h>
#include <interrupt.h>
#include <io.h>
#include <libc.h>
#include <mm.h>
#include <mm_address.h>
#include <sched.h>
#include <sys.h>
#include <utils.h>

#define READ 0
#define WRITE 1
#define BUFFER_SIZE 256

char buffer_k[BUFFER_SIZE];

int check_fd(int fd, int permissions) {
    if (fd != 1) return -9;               /*EBADF*/
    if (permissions != WRITE) return -13; /*EACCES*/
    return 0;
}

int sys_ni_syscall() {
    return -38; /*ENOSYS*/
}

int sys_getpid() {
    return current_task->PID;
}

int ret_from_fork() {
    return 0;
}

int sys_fork() {
    int PID = -1;
    char debug_buf[12];

#if DEBUG_INFO_FORK
    printk("[FORK] PID ");
    itoa(current_task->PID, debug_buf);
    printk(debug_buf);
    printk(" calling fork\n");
#endif

    /*=== STEP a: Get a free task_struct ===*/
    if (list_empty(&freequeue)) {
        return -ENOMEM; // No space for new processes
    }

    struct list_head *free_node = list_first(&freequeue);
    list_del(free_node);
    struct task_struct *child_task = list_head_to_task_struct(free_node);
    union task_union *child_union = (union task_union *)child_task;

    /*=== STEP b: Inherit system data === */
    // Copy the entire task_union from parent to child
    copy_data(current_task, child_union, sizeof(union task_union));

    /* === STEP c: Get new page directory === */
    // Assign a new page directory to the child
    allocate_DIR(child_task);

    /* === STEP d: Search frames for data+stack === */
    // We need new physical frames for child's data+stack
    int new_frames[NUM_PAG_DATA];
    for (int frame = 0; frame < NUM_PAG_DATA; frame++) {
        new_frames[frame] = alloc_frame();
        if (new_frames[frame] < 0) {
            // Error: not enough free frames
            // Free the frames we already allocated
            for (int j = 0; j < frame; j++) {
                free_frame(new_frames[j]);
            }
            // Return the task_struct to the freequeue
            list_add_tail(&child_task->list, &freequeue);
            return -EAGAIN; // Not enough memory
        }
    }

    /* === STEP e: Initialize child's address space === */
    page_table_entry *child_PT = get_PT(child_task);
    page_table_entry *parent_PT = get_PT(current_task);

    /* e.i) Copy entries for system code, system data and user code (shared) */
    for (int page = 0; page < NUM_PAG_KERNEL; page++) {
        // System code and data (shared)
        set_ss_pag(child_PT, page, get_frame(parent_PT, page));
    }

    for (int page = 0; page < NUM_PAG_CODE; page++) {
        // User code (shared, read-only)
        set_ss_pag(child_PT, PAG_LOG_INIT_CODE + page,
                   get_frame(parent_PT, PAG_LOG_INIT_CODE + page));
    }

    /*=== STEP f: Inherit user data === */
    // Temporarily map child's pages in parent's address space
    int temp_pages = NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA; // Free pages

    for (int page = 0; page < NUM_PAG_DATA; page++) {
        /* e.ii) Assign new frames for user data+stack */
        set_ss_pag(child_PT, PAG_LOG_INIT_DATA + page, new_frames[page]);

        /* f.A) Temporary mapping in parent to access child's pages */
        set_ss_pag(parent_PT, temp_pages + page, new_frames[page]);

        /* f.B) Copy data+stack from parent to child */
        copy_data((void *)((PAG_LOG_INIT_DATA + page) << 12), (void *)((temp_pages + page) << 12),
                  PAGE_SIZE);

        /* f.C) Remove temporary mapping */
        del_ss_pag(parent_PT, temp_pages + page);
    }

    // Flush TLB to ensure parent cannot access child's pages
    set_cr3(get_DIR(current_task));

    /* === STEP g: Assign new PID === */
    child_task->PID = get_next_pid();
    PID = child_task->PID;

    // === STEP h: Initialize task_struct fields ===

    /* Initialize scheduling fields - inherit from parent */
    child_task->quantum = current_task->quantum;
    child_task->status = ST_READY; // new task is ready to run

    /* Fields already copied from parent need to be modified for child */
    INIT_LIST_HEAD(&child_task->list);

    /* Initialize process hierarchy */
    child_task->parent = current_task;
    INIT_LIST_HEAD(&child_task->children);
    INIT_LIST_HEAD(&child_task->child_list);

    /* Add child to parent's children list */
    list_add_tail(&child_task->child_list, &current_task->children);

    /* Initialize blocking mechanism */
    child_task->pending_unblocks = 0;

    /* Initialize thread support - child starts as single-threaded process */
    child_task->TID = 1;
    child_task->thread_count = 1;
    child_task->master_thread = child_task;
    INIT_LIST_HEAD(&child_task->threads);
    INIT_LIST_HEAD(&child_task->thread_list);
    child_task->user_stack_ptr = NULL;
    child_task->user_stack_frames = 0;

    /* === STEP i & j: Prepare registers and child stack for task_switch === */
    // Rebuild child's kernel stack so that when
    // switch_context pops EBP and returns, it jumps to ret_from_fork.
    // ret_from_fork sets EAX=0 and falls into the syscall return path
    // already present in the copied parent frame (sysexit to user).

    // We place a fake EBP and ret_from_fork as the return address, but we must
    // point into the copied syscall frame so that ret_from_fork "returns" into
    // the syscall epilogue. Offsets -19/-18 match the saved frame layout.
    child_union->stack[KERNEL_STACK_SIZE - 19] = (unsigned long)0;             // fake EBP
    child_union->stack[KERNEL_STACK_SIZE - 18] = (unsigned long)ret_from_fork; // return to stub
    child_task->kernel_esp = (unsigned long)&(child_union->stack[KERNEL_STACK_SIZE - 19]);

    /* === STEP k: Insert into ready queue === */
    list_add_tail(&child_task->list, &readyqueue);

#if DEBUG_INFO_FORK
    printk("[FORK] PID ");
    itoa(current_task->PID, debug_buf);
    printk(debug_buf);
    printk(" created child PID ");
    itoa(PID, debug_buf);
    printk(debug_buf);
    printk("\n");
#endif

    /* === STEP l: Return child PID === */
    return PID; // Parent returns child's PID
}

int sys_write(int fd, char *buffer, int size) {
    int ret;
    if (size < 0) return -EINVAL;
    if ((ret = check_fd(fd, WRITE))) return ret;
    if (!access_ok(VERIFY_READ, buffer, size)) return -EFAULT;

    int bytes_left = size;
    int written_bytes;

    while (bytes_left > BUFFER_SIZE) {
        copy_from_user(buffer, buffer_k, BUFFER_SIZE);
        written_bytes = sys_write_console(buffer_k, BUFFER_SIZE);
        bytes_left -= written_bytes;
        buffer += written_bytes;
    }

    if (bytes_left > 0) {
        copy_from_user(buffer, buffer_k, bytes_left);
        written_bytes = sys_write_console(buffer_k, bytes_left);
        bytes_left -= written_bytes;
    }

    return size - bytes_left;
}

int sys_gettime() {
    return zeos_ticks;
}

void sys_exit() {
    char debug_buf[12];
#if DEBUG_INFO_EXIT
    printk("[EXIT] PID ");
    itoa(current_task->PID, debug_buf);
    printk(debug_buf);
    printk(" TID ");
    itoa(current_task->TID, debug_buf);
    printk(debug_buf);
    printk(" calling exit\n");
#endif

    // If the process is task 1, it cannot exit
    if (current_task->PID == 1) {
        return;
    }

    struct task_struct *master = current_task->master_thread;

    /* === STEP 1: Free all threads in the process === */
    if (!list_empty(&master->threads)) {
        struct list_head *pos, *tmp;
        page_table_entry *PT = get_PT(master);

        list_for_each_safe(pos, tmp, &master->threads) {
            struct task_struct *thread = list_entry(pos, struct task_struct, thread_list);

            /* Free thread's user stack */
            if (thread->user_stack_ptr != NULL && thread->user_stack_frames > 0) {
                for (int i = 0; i < thread->user_stack_frames; i++) {
                    unsigned int stack_page = ((unsigned int)thread->user_stack_ptr >> 12) + i;
                    free_frame(get_frame(PT, stack_page));
                    del_ss_pag(PT, stack_page);
                }
            }

            /* Remove from thread list */
            list_del(&thread->thread_list);

            /* Remove from ready/blocked queue if present */
            if (thread->status != ST_RUN) {
                list_del(&thread->list);
            }

            /* Mark as unused and add to free queue */
            thread->PID = -1;
            thread->TID = -1;
            list_add_tail(&thread->list, &freequeue);
        }
    }

    /* === STEP 2: Handle orphaned children === */
    // Move children to idle process
    struct list_head *pos, *tmp;
    list_for_each_safe(pos, tmp, &current_task->children) {
        struct task_struct *child = list_entry(pos, struct task_struct, child_list);
        list_del(&child->child_list);
        child->parent = idle_task;
        list_add_tail(&child->child_list, &idle_task->children);
    }

    /* === STEP 3: Remove from parent's children list === */
    if (current_task->parent != NULL) {
        list_del(&current_task->child_list);
    }

    /* === STEP 4: Free process resources === */
    // Free data pages and clear page table entries
    page_table_entry *PT = get_PT(master);
    for (int page = 0; page < NUM_PAG_DATA; page++) {
        int frame = get_frame(PT, PAG_LOG_INIT_DATA + page);
        if (frame >= 0) {
            free_frame(frame);
            del_ss_pag(PT, PAG_LOG_INIT_DATA + page);
        }
    }

    /* === STEP 5: Return task_struct to free queue === */
    list_add_tail(&master->list, &freequeue);

    /* === STEP 6: Schedule new process === */
    sched_next_rr();
}

void sys_block() {
    if (current_task->pending_unblocks == 0) {
        update_process_state_rr(current_task, &blockedqueue);
        scheduler();
    } else {
        current_task->pending_unblocks--;
    }
}

int sys_unblock(int pid) {
    struct list_head *pos;

    /* Search for child with given PID */
    list_for_each(pos, &current_task->children) {
        struct task_struct *child = list_entry(pos, struct task_struct, child_list);
        if (child->PID == pid) {
            if (child->status == ST_BLOCKED) {
                update_process_state_rr(child, &readyqueue);
                return 0;
            } else {
                child->pending_unblocks++;
                return 0;
            }
        }
    }
    return -1;
}

/* Helper function to search for contiguous free pages in page table */
static int search_free_user_stack_pages(page_table_entry *PT, int start_page, int pages_needed) {
    if (pages_needed <= 0) return -1;

    for (int i = start_page; i < TOTAL_PAGES - pages_needed; i++) {
        int all_free = 1;

        /* Check if pages_needed consecutive pages are free */
        for (int j = 0; j < pages_needed && all_free; j++) {
            if (PT[i + j].entry != 0) {
                all_free = 0;
                i += j; /* Skip to next potential start */
            }
        }

        if (all_free) {
            return i;
        }
    }

    return -1;
}

int sys_create_thread(void (*function)(void *), void *parameter) {
    char debug_buf[12];
#if DEBUG_INFO_THREAD_CREATE
    printk("[THREAD_CREATE] PID ");
    itoa(current_task->PID, debug_buf);
    printk(debug_buf);
    printk(" TID ");
    itoa(current_task->TID, debug_buf);
    printk(debug_buf);
    printk(" creating new thread\n");
#endif

    /* Validate parameters */
    if (!function) return -EINVAL;
    if (!access_ok(VERIFY_READ, function, sizeof(void (*)(void *)))) return -EFAULT;
    if (parameter && !access_ok(VERIFY_READ, parameter, sizeof(void *))) return -EFAULT;

    /* Get free task_struct */
    if (list_empty(&freequeue)) return -ENOMEM;

    struct list_head *free_node = list_first(&freequeue);
    list_del(free_node);

    struct task_struct *new_thread = list_head_to_task_struct(free_node);
    union task_union *thread_union = (union task_union *)new_thread;

    /* Copy current task data to new thread */
    copy_data(current_task, thread_union, sizeof(union task_union));

    /* Determine master thread */
    struct task_struct *master =
        (current_task->TID == 1) ? current_task : current_task->master_thread;

    /* Get page table */
    page_table_entry *PT = get_PT(new_thread);

    /* Allocate user stack (1 page = 4KB) */
    int stack_pages = 1;
    int stack_start =
        search_free_user_stack_pages(PT, PAG_LOG_INIT_DATA + NUM_PAG_DATA, stack_pages);

    if (stack_start == -1) {
        list_add_tail(free_node, &freequeue);
        return -ENOMEM;
    }

    /* Allocate and map physical pages for user stack */
    for (int i = 0; i < stack_pages; i++) {
        int frame = alloc_frame();
        if (frame < 0) {
            /* Free allocated frames on error */
            for (int j = 0; j < i; j++) {
                free_frame(get_frame(PT, stack_start + j));
                del_ss_pag(PT, stack_start + j);
            }
            list_add_tail(free_node, &freequeue);
            return -EAGAIN;
        }
        set_ss_pag(PT, stack_start + i, frame);
    }

    /* Setup thread fields */
    master->thread_count++;
    new_thread->TID = master->thread_count;
    new_thread->master_thread = master;
    new_thread->PID = master->PID;

    /* Initialize thread-specific data - user stack pointer must point to TOP of stack */
    /* In x86, stacks grow downward, so ESP should point to the highest address */
    new_thread->user_stack_ptr =
        (int *)((stack_start << 12) + PAGE_SIZE * stack_pages - sizeof(int));
    new_thread->user_stack_frames = stack_pages;

    /* Calculate stack parameters */
    int stack_size_bytes = PAGE_SIZE * stack_pages;

    /* Setup user stack - Map temporarily to write parameter and function */
    int temp_page = NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA;
    for (int i = 0; i < stack_pages; i++) {
        set_ss_pag(get_PT(current_task), temp_page + i, get_frame(PT, stack_start + i));
    }

    /* Write to user stack using kernel temp mapping */
    int *temp_stack = (int *)(temp_page << 12);
    int stack_size_ints = stack_size_bytes / sizeof(int);

    /* Place parameter and function at the TOP of the stack (highest addresses) */
    /* The stack grows downward, so we use the last valid int positions */
    temp_stack[stack_size_ints - 1] = (int)parameter; /* Parameter for thread function */
    temp_stack[stack_size_ints - 2] = (int)function;  /* Function pointer */
    temp_stack[stack_size_ints - 3] = 0;              /* Return address (never used) */

    /* Update user_stack_ptr to point to return address position */
    new_thread->user_stack_ptr = (int *)((stack_start << 12) + (stack_size_ints - 3) * sizeof(int));

    /* Remove temporary mapping */
    for (int i = 0; i < stack_pages; i++) {
        del_ss_pag(get_PT(current_task), temp_page + i);
    }
    set_cr3(get_DIR(current_task));

    /* Setup kernel stack with simple approach:
     * - Don't copy parent context, create minimal context
     * - Thread starts in kernel mode and calls user function directly
     * - Stack setup like init_idle: return address to function, fake EBP
     */

    /* Clear the entire stack to avoid garbage */
    for (int i = 0; i < KERNEL_STACK_SIZE; i++) {
        thread_union->stack[i] = 0;
    }

    /* Set up minimal kernel stack like init_idle */
    thread_union->stack[KERNEL_STACK_SIZE - 1] = (unsigned long)function; /* Return address */
    thread_union->stack[KERNEL_STACK_SIZE - 2] = 0;                       /* fake EBP */

    /* Set kernel_esp for proper context switching */
    new_thread->kernel_esp =
        (unsigned long)&thread_union->stack[KERNEL_STACK_SIZE - 2]; /* Initialize process state */
    new_thread->quantum = DEFAULT_QUANTUM;
    new_thread->status = ST_READY;

    /* Initialize thread lists */
    INIT_LIST_HEAD(&new_thread->thread_list);
    list_add_tail(&new_thread->thread_list, &master->threads);

    /* Initialize blocking mechanism */
    new_thread->pending_unblocks = 0;

    /* Add to ready queue */
    list_add_tail(&new_thread->list, &readyqueue);

#if DEBUG_INFO_THREAD_CREATE
    printk("[THREAD_CREATE] PID ");
    itoa(new_thread->PID, debug_buf);
    printk(debug_buf);
    printk(" created TID ");
    itoa(new_thread->TID, debug_buf);
    printk(debug_buf);
    printk("\n");
#endif

    return new_thread->TID;
}
void sys_exit_thread(void) {
    struct task_struct *thread = current_task;
    struct task_struct *master = thread->master_thread;
    char debug_buf[12];

#if DEBUG_INFO_THREAD_EXIT
    printk("[THREAD_EXIT] PID ");
    itoa(thread->PID, debug_buf);
    printk(debug_buf);
    printk(" TID ");
    itoa(thread->TID, debug_buf);
    printk(debug_buf);
    printk(" exiting (master has ");
    itoa(master->thread_count, debug_buf);
    printk(debug_buf);
    printk(" threads)\n");
#endif

    /* If this is the only thread (or master thread with no other threads), exit the process */
    if (master->thread_count == 1 || (thread == master && list_empty(&master->threads))) {
        sys_exit();
        return; /* Never reached */
    }

    /* Free thread's user stack */
    page_table_entry *PT = get_PT(thread);
    if (thread->user_stack_ptr != NULL && thread->user_stack_frames > 0) {
        for (int i = 0; i < thread->user_stack_frames; i++) {
            unsigned int stack_page = ((unsigned int)thread->user_stack_ptr >> 12) + i;
            free_frame(get_frame(PT, stack_page));
            del_ss_pag(PT, stack_page);
        }
    }

    /* Decrement thread count */
    master->thread_count--;

    /* Remove from thread list */
    list_del(&thread->thread_list);

    /* Mark thread as unused */
    thread->PID = -1;
    thread->TID = -1;

    /* Add to free queue */
    list_add_tail(&thread->list, &freequeue);

    /* Schedule next process/thread */
    sched_next_rr();
}
