/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>
#include <errno.h>
#include <io.h>
#include <mm.h>
#include <mm_address.h>
#include <sched.h>
#include <sys.h>
#include <utils.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define BUFFER_SIZE 256

char buffer_k[BUFFER_SIZE];

extern int zeos_ticks;

int check_fd(int fd, int permissions) {
    if (fd != 1) return -9;                    /*EBADF*/
    if (permissions != ESCRIPTURA) return -13; /*EACCES*/
    return 0;
}

int sys_ni_syscall() {
    return -38; /*ENOSYS*/
}

int sys_getpid() {
    return current()->PID;
}

int sys_fork() {
    int PID = -1;

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
    copy_data(current(), child_union, sizeof(union task_union));

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
    page_table_entry *parent_PT = get_PT(current());

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

    /* e.ii) Assign new frames for user data+stack */
    for (int page = 0; page < NUM_PAG_DATA; page++) {
        set_ss_pag(child_PT, PAG_LOG_INIT_DATA + page, new_frames[page]);
    }

    /*=== STEP f: Inherit user data === */
    // Temporarily map child's pages in parent's address space
    int temp_pages = NUM_PAG_KERNEL + NUM_PAG_CODE + NUM_PAG_DATA; // Free pages

    for (int page = 0; page < NUM_PAG_DATA; page++) {
        /* f.A) Temporary mapping in parent to access child's pages */
        set_ss_pag(parent_PT, temp_pages + page, new_frames[page]);

        /* f.B) Copy data+stack from parent to child */
        void *src = (void *)((PAG_LOG_INIT_DATA + page) << 12); // Parent's original page
        void *dst = (void *)((temp_pages + page) << 12);        // Child's temporary page
        copy_data(src, dst, PAGE_SIZE);

        /* f.C) Remove temporary mapping */
        del_ss_pag(parent_PT, temp_pages + page);
    }

    // Flush TLB to ensure parent cannot access child's pages
    set_cr3(get_DIR(current()));

    /* === STEP g: Assign new PID === */
    child_task->PID = get_next_pid();
    PID = child_task->PID;

    // === STEP h: Initialize task_struct fields ===
    // Fields already copied from parent need to be modified for child
    INIT_LIST_HEAD(&child_task->list); // New list

    /* Initialize scheduling fields - inherit from parent */
    child_task->quantum = current()->quantum;

    /* Initialize process hierarchy */
    child_task->parent = current();
    INIT_LIST_HEAD(&child_task->children);
    INIT_LIST_HEAD(&child_task->child_list);

    /* Add child to parent's children list */
    list_add_tail(&child_task->child_list, &current()->children);

    /* Initialize blocking mechanism */
    child_task->pending_unblocks = 0;

    /* === STEP i & j: Prepare registers and child stack for task_switch === */
    // We need to prepare the child's stack so that when task_switch occurs,
    // the child can return correctly to user space with %eax = 0

    // Place return address and fake ebp at the end of the stack
    child_union->stack[KERNEL_STACK_SIZE - 1] = (unsigned long)ret_from_fork;
    child_union->stack[KERNEL_STACK_SIZE - 2] = 0; // fake ebp
    child_task->kernel_esp = (unsigned long)&child_union->stack[KERNEL_STACK_SIZE - 2];

    // When task_switch occurs to the child:
    // 1. switch_context will restore ebp (fake ebp = 0)
    // 2. ret will jump to ret_from_fork
    // 3. ret_from_fork will restore the complete context and return to user space

    /* === STEP k: Insert into ready queue === */
    list_add_tail(&child_task->list, &readyqueue);

    /* === STEP l: Return child PID === */
    return PID; // Parent returns child's PID
}

int sys_write(int fd, char *buffer, int size) {

    int fd_error = check_fd(fd, ESCRIPTURA);
    if (fd_error) return fd_error;
    if (buffer == NULL) return -EFAULT;
    if (size < 0) return -EINVAL;

    int bytes_restantes = size;
    int written_bytes;
    // TODO: test this syscall with sizes larger than BUFFER_SIZE and optimize it
    while (bytes_restantes > BUFFER_SIZE) {
        copy_from_user(buffer, buffer_k, BUFFER_SIZE);
        written_bytes = sys_write_console(buffer_k, BUFFER_SIZE);
        bytes_restantes -= written_bytes;
        buffer += written_bytes;
    }

    if (bytes_restantes > 0) {
        copy_from_user(buffer, buffer_k, bytes_restantes);
        written_bytes = sys_write_console(buffer_k, bytes_restantes);
        bytes_restantes -= written_bytes;
    }

    return size - bytes_restantes;
}

int sys_gettime() {
    return zeos_ticks;
}

void sys_exit() {
    struct task_struct *current_task = current();

    /* === STEP a: Free process resources === */
    // Free data pages
    page_table_entry *PT = get_PT(current_task);
    for (int page = 0; page < NUM_PAG_DATA; page++) {
        int frame = get_frame(PT, PAG_LOG_INIT_DATA + page);
        if (frame >= 0) {
            free_frame(frame);
        }
    }

    /* === STEP b: Update process hierarchy === */
    // Remove from parent's children list if parent exists
    if (current_task->parent != NULL) {
        list_del(&current_task->child_list);
    }

    // Move children to idle process
    struct list_head *pos, *tmp;
    list_for_each_safe(pos, tmp, &current_task->children) {
        struct task_struct *child = list_entry(pos, struct task_struct, child_list);
        list_del(&child->child_list);
        child->parent = idle_task;
        list_add_tail(&child->child_list, &idle_task->children);
    }

    /* === STEP c: Return task_struct to free queue === */
    update_process_state_rr(current_task, &freequeue);

    /* === STEP d: Schedule new process === */
    sched_next_rr();

    // This point should never be reached
}

void sys_block() {
    struct task_struct *current_task = current();

    /* Check for pending unblocks */
    if (current_task->pending_unblocks > 0) {
        current_task->pending_unblocks--;
        return; // Don't block, continue execution
    }

    /* Block the process */
    update_process_state_rr(current_task, &blocked);

    /* Schedule next process */
    sched_next_rr();
}

int sys_unblock(int pid) {
    struct task_struct *current_task = current();
    struct task_struct *target_task = NULL;

    /* Find the target process in children list */
    struct list_head *pos;
    list_for_each(pos, &current_task->children) {
        struct task_struct *child = list_entry(pos, struct task_struct, child_list);
        if (child->PID == pid) {
            target_task = child;
            break;
        }
    }

    /* Check if target is a child of current process */
    if (target_task == NULL) {
        return -1; // Process not found or not a child
    }

    /* Check if target is in blocked queue */
    struct list_head *blocked_pos;
    int is_blocked = 0;
    list_for_each(blocked_pos, &blocked) {
        struct task_struct *blocked_task = list_head_to_task_struct(blocked_pos);
        if (blocked_task == target_task) {
            is_blocked = 1;
            break;
        }
    }

    if (is_blocked) {
        /* Unblock the process */
        update_process_state_rr(target_task, &readyqueue);
    } else {
        /* Process not blocked, increase pending unblocks */
        target_task->pending_unblocks++;
    }

    return 0;
}
