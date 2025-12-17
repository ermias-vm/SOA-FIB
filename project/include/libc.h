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

/** Buffer size for prints() formatting */
#define PRINTF_BUFFER_SIZE 256

/** Global errno variable for error handling */
extern int errno;

/****************************************/
/**    String Functions                **/
/****************************************/

/**
 * @brief Convert integer to ASCII string.
 *
 * This function converts an integer value to its ASCII string representation.
 *
 * @param a Integer value to convert.
 * @param b Buffer to store the resulting string.
 */
void itoa(int a, char *b);

/**
 * @brief Calculate string length.
 *
 * This function calculates the length of a null-terminated string.
 *
 * @param a Null-terminated string to measure.
 * @return Length of the string in characters.
 */
int strlen(const char *a);

/****************************************/
/**    Error Handling Functions        **/
/****************************************/

/**
 * @brief Print error message based on errno.
 *
 * This function prints an error message corresponding to the current
 * value of errno.
 */
void perror(void);

/****************************************/
/**    I/O Functions                   **/
/****************************************/

/**
 * @brief Formatted print to stdout (user space printf).
 *
 * This function formats a string with variable arguments and writes
 * it to stdout. Similar to printf but simplified for ZeOS user space.
 * Supports format specifiers: %d (int), %s (string), %c (char), %% (literal %).
 *
 * @param fmt Format string with optional format specifiers.
 * @param ... Variable arguments matching format specifiers.
 */
void prints(const char *fmt, ...);

/****************************************/
/**    System Call Wrappers            **/
/****************************************/

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
 * @brief Clear screen buffer by filling it with spaces.
 *
 * This function clears the specified screen file descriptor by writing
 * a buffer filled with spaces and default color attributes (light gray on black).
 *
 * @param fd File descriptor to clear (typically 10 for screen buffer).
 * @return Number of bytes written, or negative error code.
 */
int clear_screen_buffer(int fd);

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
int getpid(void);

/**
 * @brief User-space wrapper for gettid system call.
 *
 * This wrapper function provides the user-space interface for retrieving
 * the thread identifier of the calling thread.
 *
 * @see sys_gettid (defined in sys.h)
 * @return Thread ID of the current thread, or -1 on error with errno set
 */
int gettid(void);

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
int fork(void);

/**
 * @brief Terminate process.
 *
 * This function terminates the current process.
 */
void exit(void);

/****************************************/
/**    Process Synchronization         **/
/****************************************/

/**
 * @brief Block current process.
 *
 * This function blocks the current process.
 *
 * @return 0 on success, -1 on error with errno set to:
 *         - EINPROGRESS: called from within a keyboard handler
 */
int block(void);

/**
 * @brief Unblock a child process.
 *
 * This function unblocks a child process by PID.
 *
 * @param pid Process ID to unblock.
 * @return 0 on success, -1 on error with errno set to:
 *         - ESRCH: pid does not correspond to a child process
 *         - EINPROGRESS: called from within a keyboard handler
 */
int unblock(int pid);

/**
 * @brief Wait until next clock tick.
 *
 * This function blocks the current thread until the next clock
 * interrupt (timer tick) occurs. Multiple threads can be blocked
 * simultaneously waiting for a tick.
 *
 * @return 0 on success, -1 on error with errno set to:
 *         - EINPROGRESS: called from within a keyboard handler
 */
int WaitForTick(void);

/****************************************/
/**    Thread Functions                **/
/****************************************/

/**
 * @brief Create a new thread.
 *
 * This function creates a new thread that executes the given function
 * with the provided parameter. Internally, ThreadCreate passes the address
 * of thread_entry_wrapper to the kernel, which sets up the new thread to
 * start execution at that wrapper. The wrapper then calls function(parameter)
 * and ensures ThreadExit is always called when the function returns.
 *
 * @param function Pointer to the function the thread will execute.
 * @param parameter Parameter to pass to the thread function.
 * @return Thread ID (TID) on success, -1 on error with errno set to:
 *         - EINVAL: Invalid function pointer
 *         - ENOMEM: No available task structures or TID slots
 *         - EAGAIN: Could not allocate memory for thread stack
 *         - EINPROGRESS: Called from within a keyboard handler
 */
int ThreadCreate(void (*function)(void *), void *parameter);

/**
 * @brief Exit the current thread.
 *
 * This function terminates the calling thread and frees its resources
 * (user stack, TID slot). If this is the last thread in the process,
 * the entire process is terminated.
 *
 * Note: Returning from a thread function without calling ThreadExit
 * would crash the system, but thread_entry_wrapper ensures ThreadExit
 * is always called automatically.
 */
void ThreadExit(void);

/**
 * @brief Thread entry wrapper function (internal use).
 *
 * This function is the actual entry point for new threads. The kernel
 * sets up the thread's initial EIP to point to this wrapper, with the
 * user stack containing:
 *   - [esp+0]: unused return address
 *   - [esp+4]: function pointer
 *   - [esp+8]: parameter
 *
 * The wrapper:
 *   1. Extracts function and parameter from the stack
 *   2. Calls function(parameter)
 *   3. Calls ThreadExit when the function returns
 *
 * This guarantees ThreadExit is always called, even if the user forgets.
 */
void thread_entry_wrapper(void);

/****************************************/
/**    Keyboard Functions              **/
/****************************************/

/**
 * @brief Register a keyboard event handler.
 *
 * This function programs a callback that will be called every time a
 * keyboard event (key press or release) occurs. The callback receives
 * the key scancode and a flag indicating if the key was pressed (1) or
 * released (0).
 *
 * The callback is executed in the context of the current thread using
 * an auxiliary stack. After the callback returns, execution resumes
 * where it was interrupted (transparent to the user).
 *
 * System calls executed inside the callback return -EINPROGRESS.
 *
 * @param func Callback function, or NULL to disable keyboard events.
 * @return 0 on success, -1 on error with errno set to:
 *         - EFAULT: func is not a valid user address
 *         - ENOMEM: cannot allocate auxiliary stack
 *         - EINPROGRESS: called from within a keyboard handler
 */
int KeyboardEvent(void (*func)(char key, int pressed));

/****************************************/
/**    Math Functions                  **/
/****************************************/

/**
 * @brief Compute absolute value of an integer.
 *
 * @param x Integer value.
 * @return Absolute value of x.
 */
int abs(int x);

/**
 * @brief Keyboard event wrapper function (internal use).
 *
 * This function is the entry point called by the kernel when a keyboard
 * event occurs. The kernel modifies the interrupted thread's context to
 * jump here with the auxiliary stack containing:
 *   - [esp+0]:  unused return address
 *   - [esp+4]:  user handler function pointer
 *   - [esp+8]:  key scancode (0-127)
 *   - [esp+12]: pressed flag (1=pressed, 0=released)
 *
 * The wrapper:
 *   1. Extracts handler, key, and pressed from the stack
 *   2. Calls handler(key, pressed)
 *   3. Executes int 0x2b to signal the kernel to resume normal execution
 *
 * This makes keyboard handling transparent to the user - the interrupted
 * code continues exactly where it left off after the handler returns.
 *
 * @note This function never returns normally; int 0x2b restores context.
 */
void kbd_wrapper(void);

/****************************************/
/**    Test Functions                  **/
/****************************************/

/**
 * @brief Execute ZeOS test suite.
 *
 * This function runs the ZeOS test suite.
 */
void execute_zeos_tests(void);

#endif /* __LIBC_H__ */
