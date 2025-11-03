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
#define PAGEFAULT_TEST          0
// clang-format on

// Utility macros
#define RESET_ERRNO() errno = 0

// Buffer sizes
#define BUFFER_SIZE 256
#define LARGE_BUFFER_SIZE 300

// Main test execution function
void execute_zeos_tests(void);

// Sys call test functions
void test_write_syscall(void);
void test_gettime_syscall(void);
void test_getpid_syscall(void);
void test_pagefault_exception(void);

// Helper functions
void print_test_header(char *test_name);
void print_test_result(char *test_name, int passed);
void print_final_summary(void);

#endif /* __ZEOS_TEST_H__ */