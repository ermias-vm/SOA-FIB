/**
 * @file kernel_helpers.c
 * @brief Internal kernel helper function implementations for ZeOS.
 *
 * This file contains helper functions used by system calls and kernel
 * subsystems for common operations like file descriptor validation
 * and execution context checking.
 */

#include <errno.h>
#include <kernel_helpers.h>
#include <mm.h>
#include <sched.h>

int ret_from_fork(void) {
    return 0;
}

int check_fd(int fd, int permissions) {
    if (fd != FD_CONSOLE && fd != FD_SCREEN) return -EBADF;
    if (permissions != O_WRONLY) return -EACCES;
    return 0;
}

int in_keyboard_context(void) {
    return (current_task && current_task->in_kbd_context);
}

int grow_user_stack(unsigned int fault_addr) {
    struct task_struct *thread = current_task;
    if (!thread || thread->user_stack_region_pages == 0 || thread->user_stack_ptr == NULL)
        return -EFAULT;

    unsigned int fault_page = fault_addr >> 12;
    unsigned int region_start = thread->user_stack_region_start;
    unsigned int region_end = region_start + thread->user_stack_region_pages;

    if (fault_page < region_start || fault_page >= region_end) return -EFAULT;

    if ((unsigned int)thread->user_stack_frames >= thread->user_stack_region_pages) return -ENOMEM;

    unsigned int first_mapped_page = ((unsigned int)thread->user_stack_ptr) >> 12;
    if (fault_page != first_mapped_page - 1) return -EFAULT;

    page_table_entry *PT = get_PT(thread);
    int frame = alloc_frame();
    if (frame < 0) return frame;

    set_ss_pag(PT, fault_page, frame);
    thread->user_stack_ptr = (int *)(fault_page << 12);
    thread->user_stack_frames++;

    set_cr3(get_DIR(thread));

    return 0;
}

int stack_region_overlaps(const struct task_struct *task, unsigned int start, unsigned int pages) {
    if (!task || task->user_stack_region_pages == 0) return 0;

    unsigned int end = start + pages;
    unsigned int existing_start = task->user_stack_region_start;
    unsigned int existing_end = existing_start + task->user_stack_region_pages;

    return !(end <= existing_start || start >= existing_end);
}

int stack_region_in_use(struct task_struct *master, unsigned int start, unsigned int pages) {
    if (stack_region_overlaps(master, start, pages)) return 1;

    struct list_head *pos;
    list_for_each(pos, &master->threads) {
        struct task_struct *thread = list_entry(pos, struct task_struct, thread_list);
        if (stack_region_overlaps(thread, start, pages)) return 1;
    }

    return 0;
}

int find_free_stack_region(struct task_struct *master) {
    for (unsigned int page = THREAD_STACK_BASE_PAGE;
         page + THREAD_STACK_REGION_PAGES <= TOTAL_PAGES; page += THREAD_STACK_REGION_PAGES) {
        if (!stack_region_in_use(master, page, THREAD_STACK_REGION_PAGES)) return page;
    }
    return -1;
}

int map_stack_pages(struct task_struct *master, unsigned int first_page,
                    unsigned int pages_to_map) {
    page_table_entry *PT = get_PT(master);

    for (unsigned int i = 0; i < pages_to_map; ++i) {
        int frame = alloc_frame();
        if (frame < 0) {
            /* Roll back previously mapped pages */
            while (i > 0) {
                --i;
                unsigned int page = first_page + i;
                free_frame(get_frame(PT, page));
                del_ss_pag(PT, page);
            }
            return -EAGAIN;
        }
        set_ss_pag(PT, first_page + i, frame);
    }

    return 0;
}

void release_thread_stack(struct task_struct *thread) {
    if (!thread || thread->user_stack_frames <= 0 || thread->user_stack_ptr == NULL) {
        thread->user_stack_region_start = 0;
        thread->user_stack_region_pages = 0;
        thread->user_initial_esp = 0;
        thread->user_entry = 0;
        return;
    }

    page_table_entry *PT = get_PT(thread);
    unsigned int first_page = ((unsigned int)thread->user_stack_ptr) >> 12;

    for (int i = 0; i < thread->user_stack_frames; ++i) {
        unsigned int page = first_page + i;
        free_frame(get_frame(PT, page));
        del_ss_pag(PT, page);
    }

    set_cr3(get_DIR(thread));

    thread->user_stack_ptr = NULL;
    thread->user_stack_frames = 0;
    thread->user_stack_region_start = 0;
    thread->user_stack_region_pages = 0;
    thread->user_initial_esp = 0;
    thread->user_entry = 0;
}
