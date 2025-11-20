/**
 * @file sys.h
 * @brief System call interface definitions for ZeOS.
 *
 * This header defines system call function prototypes and
 * interfaces for user-kernel communication in ZeOS.
 */

#ifndef __SYS_H__
#define __SYS_H__

/**
 * KERNEL STACK LAYOUT FOR CHILD PROCESS (sys_fork)
 * ================================================
 * Lower memory addresses (bottom of stack)
 *
 *    +--------------------+ <- &child_union->stack[0] (lowest address)
 *    |                    |
 *    |   UNUSED SPACE     |
 *    |                    |
 *    +--------------------+ <- child_task->kernel_esp points here
 *    |   fake EBP (0)     | <- Fake base pointer for switch_context
 *    +--------------------+ <- stack[KERNEL_STACK_SIZE - 19]
 *    | @ret_from_fork     | <- Address of ret_from_fork function
 *    +--------------------+ <- stack[KERNEL_STACK_SIZE - 18]
 *    | @ret to syscall    | <- Return address to sysenter_return
 *    +--------------------+ <- stack[KERNEL_STACK_SIZE - 17] (approx)
 *    |                    |
 *    |   CTX SW (11 regs) | <- Software context saved by SAVE_ALL macro
 *    |   EBX              | <- General purpose registers
 *    |   ECX, EDX, ESI    | <- General purpose registers
 *    |   EDI, EBP, EAX    | <- General purpose registers
 *    |   DS, ES, FS, GS   | <- Segment registers
 *    +--------------------+ <- stack[KERNEL_STACK_SIZE - 6] (approx)
 *    |                    |
 *    |   CTX HW (5 regs)  | <- Hardware context saved by CPU during syscall
 *    |   user EIP         | <- User instruction pointer
 *    |   user CS          | <- User code segment
 *    |   user EFLAGS      | <- User flags
 *    |   user ESP         | <- User stack pointer
 *    |   user SS          | <- User stack segment
 *    +--------------------+
 *    |                    |
 *    |   UNUSED SPACE     |
 *    |                    |
 *    +--------------------+ <- &child_union->stack[KERNEL_STACK_SIZE-1] (highest address)
 *
 * Higher memory addresses (top of stack)
 */

/**
 * @brief Creates a new process (child process).
 *
 * This function creates a new process by duplicating the calling process.
 * The child process is a copy of the parent process with some important differences:
 * - Child gets a new PID
 * - Child gets its own address space (separate page directory and data/stack pages)
 * - Child gets a copy of parent's user data and stack
 * - Child starts execution from the same point but returns 0
 * - Parent continues execution and returns child's PID
 *
 * @return In the parent process, returns the PID of the child process.
 *         In the child process, returns 0.
 *         Returns -ENOMEM if no free task slots available.
 *         Returns -EAGAIN if insufficient memory for child's address space.
 */
int sys_fork(void);

/**
 * @brief Return from fork system call for child process.
 *
 * This function is used by child processes created by fork() to return
 * to user space with the appropriate return value (0 for child).
 * @return Always returns 0 in the child process after fork.
 */
int ret_from_fork(void);

/**
 * @brief Writes data to a file descriptor.
 *
 * This function writes a buffer of characters to a file descriptor.
 * Currently only supports writing to stdout (fd=1).
 * @param fd File descriptor where to write the data.
 * @param buffer Pointer to the character buffer to be written.
 * @param size Size of the buffer in bytes.
 * @return The number of bytes written, or a negative error code on failure.
 */
int sys_write(int fd, char *buffer, int size);

/**
 * @brief Gets the current system time.
 *
 * This function returns the current system time measured in timer ticks
 * since the system started.
 * @return The current system time in ticks.
 */
int sys_gettime(void);

/**
 * @brief Terminate the current process.
 *
 * This function terminates the calling process, freeing its resources
 * and scheduling a new process to run.
 */
void sys_exit(void);

/**
 * @brief Block the current process.
 *
 * This function blocks the current process unless there are pending
 * unblock operations.
 */
void sys_block(void);

/**
 * @brief Unblock a child process.
 *
 * This function unblocks a child process identified by its PID.
 * @param pid Process ID of the child to unblock.
 * @return 0 on success, -1 on error.
 */
int sys_unblock(int pid);

/**
 * @brief Not implemented system call handler.
 *
 * This function handles system calls that are not implemented in the system.
 * It serves as a placeholder for unimplemented system calls.
 * @return Returns -ENOSYS (function not implemented error code).
 */
int sys_ni_syscall(void);

/**
 * @brief Gets the process identifier (PID) of the current process.
 *
 * This function returns the process identifier of the calling process.
 * The PID is unique for each process in the system.
 * @return The process identifier (PID) of the current process.
 */
int sys_getpid(void);

/**
 * @brief Check file descriptor validity and permissions.
 *
 * This function validates a file descriptor and checks if the requested
 * permissions are allowed for that descriptor.
 *
 * @param fd File descriptor to check.
 * @param permissions Requested permissions to check.
 * @return 0 if valid, negative error code otherwise.
 */
int check_fd(int fd, int permissions);

#endif /* __SYS_H__ */
