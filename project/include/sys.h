/**
 * @file sys.h
 * @brief System call interface definitions for ZeOS.
 *
 * This header defines system call function prototypes and
 * interfaces for user-kernel communication in ZeOS.
 */

#ifndef __SYS_H__
#define __SYS_H__

#include <sched.h>

/** System buffer size for kernel operations */
#define SYS_BUFFER_SIZE 256

/** Kernel stack offsets for accessing saved context (matches entry.S SAVE_ALL layout) */
#define STACK_EBX (KERNEL_STACK_SIZE - 16)      /**< EBX in software context */
#define STACK_ECX (KERNEL_STACK_SIZE - 15)      /**< ECX in software context */
#define STACK_EDX (KERNEL_STACK_SIZE - 14)      /**< EDX in software context */
#define STACK_ESI (KERNEL_STACK_SIZE - 13)      /**< ESI in software context */
#define STACK_EDI (KERNEL_STACK_SIZE - 12)      /**< EDI in software context */
#define STACK_EBP (KERNEL_STACK_SIZE - 11)      /**< EBP in software context */
#define STACK_EAX (KERNEL_STACK_SIZE - 10)      /**< EAX in software context */
#define STACK_DS (KERNEL_STACK_SIZE - 9)        /**< DS in software context */
#define STACK_ES (KERNEL_STACK_SIZE - 8)        /**< ES in software context */
#define STACK_FS (KERNEL_STACK_SIZE - 7)        /**< FS in software context */
#define STACK_GS (KERNEL_STACK_SIZE - 6)        /**< GS in software context */
#define STACK_USER_EIP (KERNEL_STACK_SIZE - 5)  /**< User EIP in hardware context */
#define STACK_USER_CS (KERNEL_STACK_SIZE - 4)   /**< User CS in hardware context */
#define STACK_EFLAGS (KERNEL_STACK_SIZE - 3)    /**< EFLAGS in hardware context */
#define STACK_USER_ESP (KERNEL_STACK_SIZE - 2)  /**< User ESP in hardware context */
#define STACK_USER_SS (KERNEL_STACK_SIZE - 1)   /**< User SS in hardware context */
#define STACK_RET_ADDR (KERNEL_STACK_SIZE - 18) /**< Return address for switch_context */
#define STACK_FAKE_EBP (KERNEL_STACK_SIZE - 19) /**< Fake EBP for switch_context pop */

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

/** Kernel buffer for system operations */
extern char buffer_k[SYS_BUFFER_SIZE];

/* Note: check_fd() and in_keyboard_context() are declared in kernel_helpers.h */

/**
 * @brief Not implemented system call handler.
 *
 * This function handles system calls that are not implemented in the system.
 * It serves as a placeholder for unimplemented system calls.
 *
 * @return -1 on error with errno set to:
 *         -ENOSYS function not implemented
 */
int sys_ni_syscall(void);

/**
 * @brief Gets the process identifier (PID) of the current process.
 *
 * This function returns the process identifier of the calling process.
 * The PID is unique for each process in the system.
 *
 * @return The process identifier (PID) of the current process, or -1 on error with errno set to:
 *         -EINPROGRESS if called from within a keyboard handler
 */
int sys_getpid(void);

/**
 * @brief Gets the thread identifier (TID) of the current thread.
 *
 * @return The thread identifier (TID) of the current thread, or -1 on error with errno set to:
 *         -EINPROGRESS if called from within a keyboard handler
 */
int sys_gettid(void);

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
 *         On error returns -1 with errno set to:
 *         -ENOMEM if no free task slots available
 *         -EAGAIN if insufficient memory for child's address space
 *         -EINPROGRESS if called from within a keyboard handler
 */
int sys_fork(void);

/**
 * @brief Writes data to a file descriptor.
 *
 * This function writes a buffer of characters to a file descriptor.
 * Supported file descriptors:
 *   - fd=1 (FD_CONSOLE): Console output, character by character with cursor management.
 *   - fd=10 (FD_SCREEN): Direct screen buffer, writes 80x25x2 bytes to video memory.
 *
 * @param fd File descriptor where to write the data.
 * @param buffer Pointer to the character buffer to be written.
 * @param size Size of the buffer in bytes.
 * @return The number of bytes written on success, or -1 on error with errno set to:
 *         -EINVAL if size is negative
 *         -EBADF if fd is not a valid file descriptor
 *         -EFAULT if buffer is not a valid user address
 *         -EINPROGRESS if called from within a keyboard handler
 */
int sys_write(int fd, char *buffer, int size);

/**
 * @brief Gets the current system time.
 *
 * This function returns the current system time measured in timer ticks
 * since the system started.
 *
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
 *
 * @return 0 on success, -1 on error with errno set to:
 *         -EINPROGRESS if called from within a keyboard handler
 */
int sys_block(void);

/**
 * @brief Unblock a child process.
 *
 * This function unblocks a child process identified by its PID.
 *
 * @param pid Process ID of the child to unblock.
 * @return 0 on success, -1 on error with errno set to:
 *         -ESRCH if pid does not correspond to a child process
 *         -EINPROGRESS if called from within a keyboard handler
 */
int sys_unblock(int pid);

/**
 * @brief Wait until next clock tick.
 *
 * This system call blocks the current thread until the next clock
 * interrupt (timer tick) occurs. All threads blocked on this call
 * are woken up simultaneously when a tick occurs.
 *
 * @return 0 on success, -1 on error with errno set to:
 *         -EINPROGRESS if called from within a keyboard handler
 */
int sys_waitfortick(void);

/**
 * @brief Create a new thread in the current process.
 *
 * This function creates a new thread that executes the specified function
 * with the given parameter. The thread shares the same address space as
 * the parent but has its own user stack. The wrapper function is called
 * first, which in turn calls the actual thread function and ensures
 * ThreadExit is called when the function returns.
 *
 * @param function Pointer to the function the thread will execute.
 * @param parameter Parameter to pass to the thread function.
 * @param wrapper Pointer to the thread wrapper function that calls function and ThreadExit.
 * @return Thread ID (TID) of the new thread on success, or -1 on error with errno set to:
 *         -EINVAL if function or wrapper is NULL
 *         -EFAULT if function, wrapper or parameter is not a valid user address
 *         -ENOMEM if no free task slots or TID slots available
 *         -EINPROGRESS if called from within a keyboard handler
 */
int sys_create_thread(void (*function)(void *), void *parameter, void (*wrapper)(void));

/**
 * @brief Exit the current thread.
 *
 * This function terminates the calling thread. If it's the last thread
 * in the process, the entire process is terminated.
 */
void sys_exit_thread(void);

/* Note: grow_user_stack() is declared in kernel_helpers.h */

/**
 * @brief Register a keyboard event handler.
 *
 * This system call programs a function that will be called every time
 * a keyboard event (pressing or releasing a key) is triggered.
 * The function receives the scancode of the key and whether it was
 * pressed (1) or released (0).
 *
 * When a key event occurs, the OS immediately executes the handler
 * in the context of the current thread using an auxiliary stack.
 * The handler must be transparent to the user - a wrapper in libc
 * executes the user function and then calls int 0x2b to resume.
 *
 * System calls executed inside the handler return -EINPROGRESS.
 *
 * @param func User callback function, or NULL to disable keyboard events.
 * @param wrapper User wrapper function that calls func and int 0x2b.
 * @return 0 on success, -1 on error with errno set to:
 *         -EFAULT if func is not a valid user address
 *         -ENOMEM if cannot allocate auxiliary stack
 *         -EINPROGRESS if called from within a keyboard handler
 */
int sys_keyboard_event(void (*func)(char key, int pressed), void (*wrapper)(void));

#endif /* __SYS_H__ */
