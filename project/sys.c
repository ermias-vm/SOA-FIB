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
#include <kernel_helpers.h>
#include <keyboard.h>
#include <libc.h>
#include <mm.h>
#include <mm_address.h>
#include <sched.h>
#include <screen.h>
#include <sys.h>
#include <utils.h>

/* Kernel buffer for system operations */
char buffer_k[SYS_BUFFER_SIZE];

int sys_ni_syscall(void) {
    return -38; /*ENOSYS*/
}

int sys_getpid(void) {
    if (in_keyboard_context()) {
        return -EINPROGRESS;
    }
    return current_task->PID;
}

int sys_gettid(void) {
    if (in_keyboard_context()) {
        return -EINPROGRESS;
    }
    return current_task->TID;
}

int sys_fork(void) {
    int PID = -1;

#if DEBUG_INFO_FORK
    printk_color_fmt(INFO_COLOR, "DEBUG->[FORK] PID %d TID %d calling fork\n", current_task->PID,
                     current_task->TID);
#endif

    if (in_keyboard_context()) {
        return -EINPROGRESS;
    }

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
    /* Use FORK_TEMP_MAPPING_PAGE at end of address space to avoid conflicts with thread stacks */
    unsigned int temp_pages = FORK_TEMP_MAPPING_PAGE;

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
    child_task->thread_count = 1;
    child_task->master_thread = child_task;
    INIT_LIST_HEAD(&child_task->threads);
    INIT_LIST_HEAD(&child_task->thread_list);
    init_tid_slots(child_task);
    child_task->TID = child_task->PID * 10; /* TID = PID*10 + 0 (master uses slot 0) */

    /* Initialize keyboard fields - child does NOT inherit keyboard handler */
    init_keyboard_fields(child_task);

    /*=== STEP: Copy parent thread's user stack if it exists ===*/
    /* This is required because the calling thread may have a dedicated stack outside data+stack */
    if (current_task->user_stack_ptr != NULL && current_task->user_stack_frames > 0) {
        /* Find a free stack region in child's address space */
        unsigned int region_start = THREAD_STACK_BASE_PAGE;
        unsigned int region_end = region_start + THREAD_STACK_REGION_PAGES;
        unsigned int first_mapped_page = region_end - current_task->user_stack_frames;

        /* Allocate and map frames for child's user stack */
        for (int i = 0; i < current_task->user_stack_frames; i++) {
            int frame = alloc_frame();
            if (frame < 0) {
                /* Rollback on error */
                for (int j = 0; j < i; j++) {
                    free_frame(get_frame(child_PT, first_mapped_page + j));
                    del_ss_pag(child_PT, first_mapped_page + j);
                }
                /* Also free data frames */
                for (int j = 0; j < NUM_PAG_DATA; j++) {
                    free_frame(new_frames[j]);
                }
                list_add_tail(&child_task->list, &freequeue);
                return -EAGAIN;
            }
            set_ss_pag(child_PT, first_mapped_page + i, frame);
        }

        /* Copy parent's user stack to child using temporary mapping */
        unsigned int parent_stack_page = ((unsigned int)current_task->user_stack_ptr) >> 12;
        for (int i = 0; i < current_task->user_stack_frames; i++) {
            /* Map child's stack page temporarily in parent's address space */
            set_ss_pag(parent_PT, temp_pages + i, get_frame(child_PT, first_mapped_page + i));

            /* Copy from parent's stack to child's stack */
            copy_data((void *)((parent_stack_page + i) << 12), (void *)((temp_pages + i) << 12),
                      PAGE_SIZE);

            /* Remove temporary mapping */
            del_ss_pag(parent_PT, temp_pages + i);
        }

        child_task->user_stack_region_start = region_start;
        child_task->user_stack_region_pages = THREAD_STACK_REGION_PAGES;
        child_task->user_stack_frames = current_task->user_stack_frames;
        child_task->user_stack_ptr = (int *)(first_mapped_page << 12);

        /* Update child's user ESP to point to equivalent position in new stack */
        unsigned int parent_esp = ((union task_union *)current_task)->stack[KERNEL_STACK_SIZE - 2];
        unsigned int offset = parent_esp - (unsigned int)(current_task->user_stack_ptr);
        unsigned int child_esp = (unsigned int)(child_task->user_stack_ptr) + offset;
        ((union task_union *)child_task)->stack[KERNEL_STACK_SIZE - 2] = child_esp;

        child_task->user_initial_esp = child_esp;
    } else {
        child_task->user_stack_ptr = NULL;
        child_task->user_stack_frames = 0;
        child_task->user_stack_region_start = 0;
        child_task->user_stack_region_pages = 0;
        child_task->user_initial_esp = 0;
    }
    child_task->user_entry = 0;

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
    printk_color_fmt(INFO_COLOR, "DEBUG->[FORK] PID %d TID %d created child PID %d TID %d\n",
                     current_task->PID, current_task->TID, PID, PID * 10);
#endif

    /* === STEP l: Return child PID === */
    return PID; // Parent returns child's PID
}

int sys_write(int fd, char *buffer, int size) {
    int ret;
    if (size < 0) return -EINVAL;
    if ((ret = check_fd(fd, O_WRONLY))) return ret;
    if (!access_ok(VERIFY_READ, buffer, size)) return -EFAULT;

    if (in_keyboard_context()) {
        return -EINPROGRESS;
    }

    /* Direct screen buffer write (fd = 10) */
    if (fd == FD_SCREEN) {
        return sys_write_screen(buffer, size);
    }

    /* Console output (fd = 1) */
    int bytes_left = size;
    int written_bytes;

    while (bytes_left > SYS_BUFFER_SIZE) {
        copy_from_user(buffer, buffer_k, SYS_BUFFER_SIZE);
        written_bytes = sys_write_console(buffer_k, SYS_BUFFER_SIZE);
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

int sys_gettime(void) {
    return zeos_ticks;
}

void sys_exit(void) {
#if DEBUG_INFO_EXIT
    printk_color_fmt(INFO_COLOR, "DEBUG->[EXIT] PID %d TID %d calling exit\n", current_task->PID,
                     current_task->TID);
#endif
    struct task_struct *master = current_task->master_thread;

    /* === STEP 1: Free all threads in the process === */
    if (!list_empty(&master->threads)) {
        struct list_head *pos, *tmp;

        list_for_each_safe(pos, tmp, &master->threads) {
            struct task_struct *thread = list_entry(pos, struct task_struct, thread_list);

            release_thread_stack(thread);

            /* Free TID slot */
            free_tid(master, thread->TID);

            list_del(&thread->thread_list);

            if (thread->status != ST_RUN) {
                list_del(&thread->list);
            }

            list_add_tail(&thread->list, &freequeue);
        }
    }

    /* === STEP 2: Handle orphaned children === */
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
    page_table_entry *PT = get_PT(master);
    for (int page = 0; page < NUM_PAG_DATA; page++) {
        int frame = get_frame(PT, PAG_LOG_INIT_DATA + page);
        if (frame >= 0) {
            free_frame(frame);
            del_ss_pag(PT, PAG_LOG_INIT_DATA + page);
        }
    }

    /* === STEP 5: Return master task_struct to free queue === */
    list_add_tail(&master->list, &freequeue);

    /* Clean up keyboard handler if registered */
    cleanup_kbd_handler(current_task);

    /* === STEP 6: Schedule new process (will go to idle if no processes left) === */
    sched_next_rr();
}

int sys_block(void) {
    /* Cannot block from keyboard handler context */
    if (in_keyboard_context()) {
        return -EINPROGRESS;
    }

    if (current_task->pending_unblocks == 0) {
        update_process_state_rr(current_task, &blockedqueue);
        scheduler();
    } else {
        current_task->pending_unblocks--;
    }
    return 0;
}

int sys_waitfortick(void) {
    /* Cannot call from keyboard handler context */
    if (in_keyboard_context()) {
        return -EINPROGRESS;
    }

    /* Block current thread on tick_blockedqueue until next clock interrupt */
    update_process_state_rr(current_task, &tick_blockedqueue);
    sched_next_rr();

    /* When we return here, a clock tick has occurred and we were unblocked */
    return 0;
}

int sys_unblock(int pid) {
    /* Cannot unblock from keyboard handler context */
    if (in_keyboard_context()) {
        return -EINPROGRESS;
    }

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
    return -ESRCH;
}

int sys_create_thread(void (*function)(void *), void *parameter, void (*wrapper)(void)) {
#if DEBUG_INFO_THREAD_CREATE
    printk_color_fmt(INFO_COLOR, "DEBUG->[THREAD_CREATE] PID %d TID %d creating new thread\n",
                     current_task->PID, current_task->TID);
#endif

    /* Cannot create thread from keyboard handler context */
    if (in_keyboard_context()) {
        return -EINPROGRESS;
    }

    if (!function) return -EINVAL;
    if (!wrapper) return -EINVAL;
    if (!access_ok(VERIFY_READ, function, sizeof(void (*)(void *)))) return -EFAULT;
    if (!access_ok(VERIFY_READ, wrapper, sizeof(void (*)(void)))) return -EFAULT;
    if (parameter && !access_ok(VERIFY_READ, parameter, sizeof(void *))) return -EFAULT;

    if (list_empty(&freequeue)) return -ENOMEM;

    struct list_head *free_node = list_first(&freequeue);
    list_del(free_node);

    struct task_struct *new_thread = list_head_to_task_struct(free_node);
    union task_union *thread_union = (union task_union *)new_thread;

    copy_data(current_task, thread_union, sizeof(union task_union));

    struct task_struct *master = current_task->master_thread;

    /* Allocate new TID */
    int new_tid = allocate_tid(master);
    if (new_tid < 0) {
        list_add_tail(&new_thread->list, &freequeue);
        return -ENOMEM; /* No available TID slots */
    }

    new_thread->dir_pages_baseAddr = master->dir_pages_baseAddr;

    INIT_LIST_HEAD(&new_thread->list);
    INIT_LIST_HEAD(&new_thread->children);
    INIT_LIST_HEAD(&new_thread->child_list);
    INIT_LIST_HEAD(&new_thread->threads);
    INIT_LIST_HEAD(&new_thread->thread_list);

    new_thread->PID = master->PID;
    new_thread->TID = new_tid;
    new_thread->parent = master;
    new_thread->master_thread = master;
    new_thread->quantum = DEFAULT_QUANTUM;
    new_thread->status = ST_READY;
    new_thread->pending_unblocks = 0;
    new_thread->user_stack_ptr = NULL;
    new_thread->user_stack_frames = 0;
    new_thread->user_stack_region_start = 0;
    new_thread->user_stack_region_pages = 0;
    new_thread->user_initial_esp = 0;
    new_thread->user_entry = 0;

    /* Initialize keyboard fields - threads share master's keyboard handler */
    init_keyboard_fields(new_thread);

    int region_start = find_free_stack_region(master);
    if (region_start < 0) {
        free_tid(master, new_tid);
        list_add_tail(&new_thread->list, &freequeue);
        return -ENOMEM;
    }

    unsigned int region_end = region_start + THREAD_STACK_REGION_PAGES;
    unsigned int first_mapped_page = region_end - THREAD_STACK_INITIAL_PAGES;

    int map_result = map_stack_pages(master, first_mapped_page, THREAD_STACK_INITIAL_PAGES);
    if (map_result < 0) {
        free_tid(master, new_tid);
        list_add_tail(&new_thread->list, &freequeue);
        return map_result;
    }

    unsigned long stack_top = (unsigned long)(region_end << 12);
    /* Stack layout for thread_wrapper:
     * stack_top - 4:  parameter (offset +8 from ESP)
     * stack_top - 8:  function pointer (offset +4 from ESP)
     * stack_top - 12: return address (offset +0 from ESP, unused)
     * ESP points to stack_top - 12 */
    unsigned long user_esp = stack_top - 3 * sizeof(unsigned long);

    page_table_entry *current_PT = get_PT(current_task);
    page_table_entry *process_PT = get_PT(master);
    for (unsigned int i = 0; i < THREAD_STACK_INITIAL_PAGES; ++i) {
        set_ss_pag(current_PT, TEMP_STACK_MAPPING_PAGE + i,
                   get_frame(process_PT, first_mapped_page + i));
    }

    unsigned long stack_bytes = THREAD_STACK_INITIAL_PAGES * PAGE_SIZE;
    unsigned long *temp_stack = (unsigned long *)(TEMP_STACK_MAPPING_PAGE << 12);
    unsigned long stack_words = stack_bytes / sizeof(unsigned long);

    /* Set up stack for thread_wrapper:
     * wrapper expects: [esp+0]=retaddr, [esp+4]=function, [esp+8]=parameter */
    temp_stack[stack_words - 1] = (unsigned long)parameter; /* offset +8: parameter */
    temp_stack[stack_words - 2] = (unsigned long)function;  /* offset +4: function ptr */
    temp_stack[stack_words - 3] = 0;                        /* offset +0: return addr (unused) */

    for (unsigned int i = 0; i < THREAD_STACK_INITIAL_PAGES; ++i) {
        del_ss_pag(current_PT, TEMP_STACK_MAPPING_PAGE + i);
    }
    set_cr3(get_DIR(current_task));
    set_cr3(get_DIR(master));

    new_thread->user_stack_region_start = region_start;
    new_thread->user_stack_region_pages = THREAD_STACK_REGION_PAGES;
    new_thread->user_stack_frames = THREAD_STACK_INITIAL_PAGES;
    new_thread->user_stack_ptr = (int *)(first_mapped_page << 12);
    new_thread->user_initial_esp = user_esp;
    new_thread->user_entry = (unsigned long)wrapper; /* Entry point is the wrapper */

    /* Thread starts at wrapper function, which will call the actual function */
    thread_union->stack[STACK_USER_EIP] = (unsigned long)wrapper;
    thread_union->stack[STACK_USER_ESP] = user_esp;
    thread_union->stack[STACK_EAX] = 0;
    thread_union->stack[STACK_EBP] = 0;
    thread_union->stack[STACK_FAKE_EBP] = 0;
    thread_union->stack[STACK_RET_ADDR] = (unsigned long)ret_from_fork;
    new_thread->kernel_esp = (unsigned long)&thread_union->stack[STACK_FAKE_EBP];

    master->thread_count++;
    list_add_tail(&new_thread->thread_list, &master->threads);

    list_add_tail(&new_thread->list, &readyqueue);

#if DEBUG_INFO_THREAD_CREATE
    printk_color_fmt(INFO_COLOR, "DEBUG->[THREAD_CREATE] PID %d created TID %d\n", new_thread->PID,
                     new_thread->TID);
#endif

    return new_thread->TID;
}

void sys_exit_thread(void) {
    struct task_struct *thread = current_task;
    struct task_struct *master = thread->master_thread;

#if DEBUG_INFO_THREAD_EXIT
    printk_color_fmt(INFO_COLOR,
                     "DEBUG->[THREAD_EXIT] PID %d TID %d exiting (master has %d threads)\n",
                     thread->PID, thread->TID, master->thread_count);
#endif

    /* If this is the only thread, terminate the whole process (including init) */
    if (master->thread_count == 1) {
        sys_exit();
        return; /* Normally not reached */
    }

    release_thread_stack(thread);

    /* Free TID slot */
    free_tid(master, thread->TID);

    /* Decrement thread count */
    master->thread_count--;

    /* Remove from thread list */
    list_del(&thread->thread_list);

    /* If we are exiting a non-master thread, just free it */
    if (thread != master) {
        list_add_tail(&thread->list, &freequeue);
        sched_next_rr();
        return;
    }

    /* We are exiting the master but there are other threads: choose new master */
    struct list_head *pos;
    struct task_struct *new_master = NULL;

    list_for_each(pos, &master->threads) {
        struct task_struct *candidate = list_entry(pos, struct task_struct, thread_list);
        if (candidate != master) {
            new_master = candidate;
            break;
        }
    }

    if (new_master == NULL) {
        /* Fallback: no other thread found, destroy process */
        sys_exit();
        return;
    }

    /* Transfer master role to new_master */
    new_master->master_thread = new_master;
    new_master->thread_count = master->thread_count;

    /* Copy TID slots to new master (array size is MAX_TIDS_PER_PROCESS: indices 0-9) */
    for (int i = 0; i < MAX_TIDS_PER_PROCESS; i++) {
        new_master->tid_slots[i] = master->tid_slots[i];
    }

    /* Free old master TID slot (TID = PID*10 + slot) */
    free_tid(new_master, master->TID);

    /* Remove new_master from thread list and rebuild as master */
    list_del(&new_master->thread_list);
    INIT_LIST_HEAD(&new_master->threads);

    /* Update all threads to point to new master */
    /* Must use list_for_each_safe because we modify the list during iteration */
    struct list_head *tmp;
    list_for_each_safe(pos, tmp, &master->threads) {
        struct task_struct *t = list_entry(pos, struct task_struct, thread_list);
        if (t == master) continue;
        t->master_thread = new_master;
        /* Must remove from old list BEFORE adding to new list */
        list_del(&t->thread_list);
        list_add_tail(&t->thread_list, &new_master->threads);
    }

    /* Finally, free the old master thread structure */
    list_add_tail(&master->list, &freequeue);

    /* Schedule next process/thread */
    sched_next_rr();
}

int sys_keyboard_event(void (*func)(char key, int pressed), void (*wrapper)(void)) {
    struct task_struct *task = current_task;

    /* Cannot register handler from within keyboard handler */
    if (in_keyboard_context()) {
        return -EINPROGRESS;
    }

    /* If func is NULL, disable keyboard events */
    if (func == NULL) {
        cleanup_kbd_handler(task);
        return 0;
    }

    /* Validate user function pointer is in user space */
    if (!access_ok(VERIFY_READ, func, sizeof(void *))) {
        return -EFAULT;
    }

    /* Validate wrapper function pointer */
    if (!access_ok(VERIFY_READ, wrapper, sizeof(void *))) {
        return -EFAULT;
    }

    /* Setup auxiliary stack for keyboard handler execution */
    int ret = setup_kbd_aux_stack(task);
    if (ret < 0) {
        return ret;
    }

    /* Register the handler and wrapper */
    task->kbd_handler = func;
    task->kbd_wrapper = wrapper;
    task->in_kbd_context = 0;

    return 0;
}
