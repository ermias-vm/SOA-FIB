/**
 * @file kernel_helpers.h
 * @brief Internal kernel helper functions for ZeOS.
 *
 * This header declares helper functions used internally by system calls
 * and kernel subsystems. These functions provide common functionality
 * like file descriptor validation and context checking.
 */

#ifndef __KERNEL_HELPERS_H__
#define __KERNEL_HELPERS_H__

#include <io.h>
#include <mm_address.h>
#include <sched.h>

/**
 * @brief Return from fork system call for child process.
 *
 * This function is used by child processes created by fork() to return
 * to user space with the appropriate return value (0 for child).
 *
 * @return Always returns 0 in the child process after fork.
 */
int ret_from_fork(void);

/**
 * @brief Validate file descriptor for given permissions.
 *
 * Checks if the specified file descriptor is valid and has the
 * required permissions for the requested operation.
 *
 * @param fd File descriptor to validate.
 * @param permissions Required permissions (O_WRONLY).
 * @return 0 if valid, -EBADF if invalid fd, -EACCES if wrong permissions.
 */
int check_fd(int fd, int permissions);

/**
 * @brief Check if currently executing in keyboard interrupt context.
 *
 * Determines whether the current code is running within a keyboard
 * event handler callback. Some syscalls are restricted in this context.
 *
 * @return 1 if in keyboard context, 0 otherwise.
 */
int in_keyboard_context(void);

/**
 * @brief Grow the user stack by one page.
 *
 * Called by the page fault handler when a stack access causes a fault
 * within the thread's allocated stack region. Allocates a new frame
 * and maps it to extend the stack downward.
 *
 * @param fault_addr The address that caused the page fault.
 * @return 0 on success, negative error code on failure.
 */
int grow_user_stack(unsigned int fault_addr);

/**
 * @brief Check if a stack region overlaps with an existing thread's stack.
 *
 * Determines whether a proposed stack region conflicts with an existing
 * thread's allocated stack space.
 *
 * @param task Pointer to the task structure to check.
 * @param start Starting page number of the proposed region.
 * @param pages Number of pages in the proposed region.
 * @return 1 if overlap exists, 0 otherwise.
 */
int stack_region_overlaps(const struct task_struct *task, unsigned int start, unsigned int pages);

/**
 * @brief Check if a stack region is already in use by any thread.
 *
 * Verifies whether a proposed stack region conflicts with any existing
 * thread's stack allocation in the process.
 *
 * @param master Pointer to the master thread of the process.
 * @param start Starting page number of the proposed region.
 * @param pages Number of pages in the proposed region.
 * @return 1 if region is in use, 0 if available.
 */
int stack_region_in_use(struct task_struct *master, unsigned int start, unsigned int pages);

/**
 * @brief Find a free region for thread stack allocation.
 *
 * Searches for an available memory region that can be used for a new thread's stack.
 *
 * @param master Pointer to the master thread of the process.
 * @return Starting page number of free region, or -1 if no space available.
 */
int find_free_stack_region(struct task_struct *master);

/**
 * @brief Map physical frames to thread stack pages.
 *
 * Allocates physical frames and maps them to the specified virtual pages
 * for thread stack usage.
 *
 * @param master Pointer to the master thread of the process.
 * @param first_page First virtual page number to map.
 * @param pages_to_map Number of pages to map.
 * @return 0 on success, negative error code on failure.
 */
int map_stack_pages(struct task_struct *master, unsigned int first_page, unsigned int pages_to_map);

/**
 * @brief Release memory allocated for a thread's stack.
 *
 * Frees all physical frames and unmaps all virtual pages associated
 * with a thread's stack region.
 *
 * @param thread Pointer to the thread whose stack should be released.
 */
void release_thread_stack(struct task_struct *thread);

/**
 * @brief Update time and FPS counters.
 *
 * Called by the clock interrupt to update internal time and FPS state.
 * This function recalculates seconds, milliseconds, and FPS values
 * but does NOT write to video memory.
 */
void update_time_and_fps(void);

/**
 * @brief Draw time and FPS to video memory.
 *
 * Writes the current cached time (SS:MMM) and FPS (XXXX FPS) values
 * to their respective screen positions. Call after update_time_and_fps()
 * in the clock interrupt, or standalone after screen buffer writes.
 */
void draw_time_and_fps(void);

#endif /* __KERNEL_HELPERS_H__ */
