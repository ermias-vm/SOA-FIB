/*
 * zeos_test.h - Header for ZeOS test suite
 */

#ifndef __ZEOS_TEST_H__
#define __ZEOS_TEST_H__

// Test configuration macros
// clang-format off
#define WRITE_TEST              1
#define GETTIME_TEST            1
#define GETPID_TEST             1
#define FORK_TEST               1
#define EXIT_TEST               1
#define BLOCK_UNBLOCK_TEST      1
#define PAGEFAULT_TEST          0
// clang-format on

// Utility macros
#define RESET_ERRNO() errno = 0

// Buffer sizes
#define BUFFER_SIZE 256
#define LARGE_BUFFER_SIZE 300

// Main test execution function
/**
 * @brief Execute all ZeOS test suites.
 *
 * This function runs all enabled test suites for system calls and exceptions.
 * It coordinates the execution of individual test functions and provides
 * a comprehensive test summary.
 */
void execute_zeos_tests(void);

// Sys call test functions
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
 * @brief Test page fault exception handling.
 *
 * This function tests the system's ability to handle page fault exceptions
 * correctly when accessing invalid memory addresses.
 */
void test_pagefault_exception(void);

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

// Helper functions
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
 * This function prints a comprehensive summary of all executed tests
 * including total tests run, passed, and failed counts.
 */
void print_final_summary(void);

#endif /* __ZEOS_TEST_H__ */
