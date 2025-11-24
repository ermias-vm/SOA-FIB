/**
 * @file project_test.h
 * @brief Test suite interface for thread support in ZeOS.
 *
 * This header defines test function prototypes and testing
 * utilities for thread functionality verification.
 */

#ifndef __PROJECT_TEST_H__
#define __PROJECT_TEST_H__

/* Test configuration macros */

// clang-format off
#define THREAD_CREATE_TEST      1
#define THREAD_EXIT_TEST        1
#define THREAD_FORK_TEST        1
#define THREAD_STRESS_TEST      1
// clang-format on

/* Reset errno macro */
#define RESET_ERRNO() errno = 0

/* Default Buffer size */
#define BUFFER_SIZE 256

/**
 * @brief Execute all thread test suites.
 *
 * This function runs all enabled test suites for thread functionality.
 * It coordinates the execution of individual test functions and provides
 * a test summary.
 */
void execute_project_tests(void);

/* ---- Thread test functions ---- */

/**
 * @brief Test ThreadCreate system call functionality.
 *
 * This function tests the ThreadCreate() system call with various parameters
 * including normal thread creation, parameter passing, and error conditions.
 */
void test_thread_create(void);

/**
 * @brief Test ThreadExit system call functionality.
 *
 * This function tests the ThreadExit() system call including proper
 * cleanup of thread resources and process termination when last thread exits.
 */
void test_thread_exit(void);

/**
 * @brief Test fork behavior with threads.
 *
 * This function tests that fork() only copies the current thread,
 * not all threads in the process.
 */
void test_thread_fork(void);

/**
 * @brief Stress test for thread creation and termination.
 *
 * This function creates multiple threads to test system limits
 * and proper resource management.
 */
void test_thread_stress(void);

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
 * @brief Write the current thread ID in a formatted manner.
 *
 * This function writes the current thread ID to standard output
 * in the format: [TID X]
 */
void write_current_tid(void);

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

#endif /* __PROJECT_TEST_H__ */
