/**
 * @file zeos_test.h
 * @brief Test suite interface definitions for ZeOS.
 *
 * This header defines test function prototypes and testing
 * utilities for ZeOS functionality verification.
 */

#ifndef __ZEOS_TEST_H__
#define __ZEOS_TEST_H__

/* Test configuration macros */

// clang-format off
#define WRITE_TEST              1
#define GETTIME_TEST            0
#define GETPID_TEST             0
#define FORK_TEST               0
#define EXIT_TEST               0
#define BLOCK_UNBLOCK_TEST      0
#define PAGEFAULT_TEST          0
// clang-format on

/* Reset errno macro */
#define RESET_ERRNO() errno = 0

/* Default Buffer size */
#define BUFFER_SIZE 256
/* Large Buffer size for write tests */
#define LARGE_BUFFER_SIZE 300

/**
 * @brief Test basic process operations.
 *
 * This function performs test of basic process operations
 * including fork(), block(), unblock(), and exit().
 * It creates a process hierarchy where:
 * - Parent process creates a first child
 * - First child creates a second child (grandchild)
 * - Second child blocks itself after showing activity
 * - First child unblocks the second child
 * - Second child exits gracefully after being unblocked
 * - All processes show their PID in a consistent format [PID X]
 */
void test_basic_process_operations(void);

/**
 * @brief Execute all ZeOS test suites.
 *
 * This function runs all enabled test suites for system calls and exceptions.
 * It coordinates the execution of individual test functions and provides
 * a test summary.
 */
void execute_zeos_tests(void);

/* ---- Sys call test functions ---- */

/**
 * @brief Test write system call functionality.
 *
 * This function tests the write() system call with various parameters
 * including normal writes, empty strings, large buffers, and error conditions.
 */
void test_write_syscall(void);

/**
 * @brief Test gettime system call functionality.
 *
 * This function tests the gettime() system call to verify it returns
 * valid time values and that time progresses correctly.
 */
void test_gettime_syscall(void);

/**
 * @brief Test getpid system call functionality.
 *
 * This function tests the getpid() system call to verify it returns
 * the correct process identifier.
 */
void test_getpid_syscall(void);

/**
 * @brief Test fork system call functionality.
 *
 * This function tests the fork() system call including process creation,
 * parent-child PID relationships, memory independence, and multiple forks.
 */
void test_fork_syscall(void);

/**
 * @brief Test exit system call functionality.
 *
 * This function tests the exit() system call.
 */
void test_exit_syscall(void);

/**
 * @brief Test block/unblock system calls functionality.
 *
 * This function tests the block() and unblock() system calls.
 */
void test_block_unblock_syscalls(void);

/* ---- Helper functions ---- */

/**
 * @brief Work for a specified number of ticks.
 *
 * This function makes the current process work (busy wait) for a specified
 * number of system ticks. 1000 ticks approximately corresponds to 1 second.
 * @param ticks Number of system ticks to work for.
 */
void work(int ticks);

/**
 * @brief Write the current process ID in a formatted manner.
 *
 * This function writes the current process ID to standard output
 * in the format: [PID X]
 */
void write_current_pid();

/**
 * @brief Print formatted test header.
 *
 * This function prints a formatted header for a test section.
 * @param test_name Name of the test being executed.
 */
void print_test_header(char *test_name);

/**
 * @brief Print test result.
 *
 * This function prints the result of a test (PASSED or FAILED).
 * @param test_name Name of the test that was executed.
 * @param passed 1 if test passed, 0 if test failed.
 */
void print_test_result(char *test_name, int passed);

/**
 * @brief Print final test summary.
 *
 * This function prints a summary of all executed tests
 * including total tests run, passed, and failed counts.
 */
void print_final_summary(void);

#endif /* __ZEOS_TEST_H__ */
