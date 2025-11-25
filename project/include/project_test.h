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
#define THREAD_ADVANCED_TEST    1
#define THREAD_FORK_TEST        1
#define IDLE_SWITCH_TEST        1
// clang-format on

/* Reset errno macro */
#define RESET_ERRNO() errno = 0

/* Default Buffer size */
#define BUFFER_SIZE 256

#define NULL ((void *)0) /* Null pointer definition */

#define SHORT_WORK_TIME 1000  /* 1 second */
#define MIN_WORK_TIME 300     /* 300 ticks */
#define MEDIUM_WORK_TIME 3000 /* 3 seconds */
#define LONG_WORK_TIME 5000   /* 5 seconds */

#define MAX_THREADS_PER_PROCESS 5 /* Maximum threads per process */

/**
 * @brief Execute all thread test suites.
 *
 * This function runs all enabled test suites for thread functionality.
 * It coordinates the execution of individual test functions and provides
 * a test summary.
 */
void execute_project_tests(void);

/**
 * @brief Execute basic thread tests.
 *
 * This function runs basic thread creation and exit tests.
 * Can be called from user code independently.
 */
void execute_basic_thread_tests(void);

/* ---- Thread test functions ---- */

/**
 * @brief Test advanced thread functionality.
 *
 * This function tests:
 * - Creating threads up to max limit (5 per process)
 * - Verifying creation fails when limit reached
 * - TID reuse after thread deletion
 * - Process termination when last thread exits
 * - Master thread reassignment when current master exits
 */
void test_thread_advanced(void);

/**
 * @brief Test fork behavior with threads.
 *
 * This function verifies that fork() only copies the current thread,
 * not all threads in the process.
 */
void test_thread_fork(void);

/* ---- Helper functions ---- */

/**
 * @brief Wait for a specified number of ticks.
 *
 * This function waits for the specified number of ticks, printing
 * PID/TID info at start and end of the wait period.
 * @param ticks Number of ticks to wait.
 */
void wait_for_ticks(int ticks);

/**
 * @brief Write current process and thread information.
 *
 * This function writes the current PID and TID in the format: [PID X] [TID Y]
 */
void write_current_info(void);

/**
 * @brief Thread function that works for a specified time and then exits.
 *
 * This function is executed by created threads. It works for the specified
 * number of ticks and then exits, setting a flag to indicate completion.
 * @param arg Pointer to integer containing the number of ticks to work.
 */
void thread_work(void *arg);

#endif /* __PROJECT_TEST_H__ */
