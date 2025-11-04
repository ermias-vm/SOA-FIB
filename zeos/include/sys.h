/*
 * sys.h - Header for system call functions
 */

#ifndef __SYS_H__
#define __SYS_H__

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

#endif /* __SYS_H__ */
