/**
 * @file libc.h
 * @brief Standard C library interface definitions for ZeOS.
 *
 * This header defines standard C library functions, string operations,
 * memory functions, and system call wrappers for user applications.
 */

#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

/**
 * @brief Convert integer to ASCII string.
 *
 * This function converts an integer value to its ASCII string representation.
 * @param a Integer value to convert.
 * @param b Buffer to store the resulting string.
 */
void itoa(int a, char *b);

/**
 * @brief Calculate string length.
 *
 * This function calculates the length of a null-terminated string.
 * @param a Null-terminated string to measure.
 * @return Length of the string in characters.
 */
int strlen(char *a);

/**
 * @brief Execute ZeOS test suite.
 *
 * This function runs the ZeOS test suite.
 */
void execute_zeos_tests(void);

/**
 * @brief User-space wrapper for gettime system call.
 *
 * This wrapper function provides the user-space interface for retrieving
 * the current system time. It calls the kernel's sys_gettime() function
 * through the SYSENTER mechanism.
 *
 * @see sys_gettime (defined in sys.h)
 * @return Current system time in timer ticks, or -1 on error
 */
int gettime(void);

/**
 * @brief User-space wrapper for write system call.
 *
 * This wrapper function provides the user-space interface for writing
 * data to a file descriptor. It calls the kernel's sys_write() function
 * through the SYSENTER mechanism with proper error handling.
 *
 * @see sys_write (defined in sys.h)
 * @param fd File descriptor to write to (currently only supports stdout=1)
 * @param buffer Pointer to data buffer to write
 * @param size Number of bytes to write
 * @return Number of bytes written on success, or -1 on error with errno set
 */
int write(int fd, char *buffer, int size);

/**
 * @brief User-space wrapper for getpid system call.
 *
 * This wrapper function provides the user-space interface for retrieving
 * the process identifier of the calling process. It calls the kernel's
 * sys_getpid() function through the SYSENTER mechanism.
 *
 * @see sys_getpid (defined in sys.h)
 * @return Process ID of the current process, or -1 on error with errno set
 */
int getpid();

/**
 * @brief User-space wrapper for fork system call.
 *
 * This wrapper function provides the user-space interface for creating
 * a new child process by duplicating the current process. It calls the
 * kernel's sys_fork() function through the SYSENTER mechanism.
 *
 * The child process is an exact copy of the parent with separate memory
 * space and a unique process ID.
 *
 * @see sys.h::sys_fork (defined in sys.h)
 * @return In parent process: PID of the child process (positive value)
 *         In child process: 0
 *         On error: -1 with errno set
 */
int fork();

/**
 * @brief Terminate process.
 *
 * This function terminates the current process.
 */
void exit();

/**
 * @brief Block current process.
 *
 * This function blocks the current process.
 */
void block(void);

/**
 * @brief Unblock a child process.
 *
 * This function unblocks a child process by PID.
 * @param pid Process ID to unblock.
 * @return 0 on success, -1 on error.
 */
int unblock(int pid);


/**
 * @brief Create a new thread.
 *
 * This function creates a new thread that executes the given function
 * with the provided parameter. A wrapper ensures ThreadExit is called.
 *
 * @param function Pointer to the function the thread will execute.
 * @param parameter Parameter to pass to the thread function.
 * @return Thread ID on success, -1 on error with errno set.
 */
int ThreadCreate(void (*function)(void *), void *parameter);

/**
 * @brief Exit the current thread.
 *
 * This function terminates the calling thread. Must be called to
 * properly clean up thread resources.
 */
void ThreadExit(void);

#endif /* __LIBC_H__ */
