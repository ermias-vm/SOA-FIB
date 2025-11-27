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
#define THREAD_TEST         1   /* Enable/disable thread tests */
#define IDLE_SWITCH_TEST    1   /* Test idle switch (exits init) */
// clang-format on

/* Reset errno macro */
#define RESET_ERRNO() errno = 0

/* Default Buffer size */
#define BUFFER_SIZE 256

#define NULL ((void *)0) /* Null pointer definition */

/* Timing constants (in ticks, ~1000 ticks = 1 second) */
#define MIN_WORK_TIME 200     /* 200ms */
#define SHORT_WORK_TIME 500   /* 500ms */
#define MEDIUM_WORK_TIME 1500 /* 1.5 seconds */
#define LONG_WORK_TIME 3000   /* 3 seconds */

#define MAX_THREADS_PER_PROCESS 10 /* Maximum threads per process (TIDs X0-X9) */

/* Synchronization flags - shared between threads */
#define MAX_SYNC_FLAGS 10

/**
 * @brief Execute all thread test suites.
 *
 * This function runs all enabled test suites for thread functionality.
 * It coordinates the execution of individual test functions and provides
 * a test summary. Thread TID 1 (init's first thread) orchestrates all tests.
 */
void execute_project_tests(void);

/**
 * @brief Main thread test suite.
 *
 * This function runs all thread tests:
 * - Subtest 1: Create threads up to max limit (10 per process)
 * - Subtest 2: Verify creation fails when limit reached
 * - Subtest 3: TID reuse after thread deletion
 * - Subtest 4: Process termination when last thread exits
 * - Subtest 5: Master thread reassignment when current master exits
 * - Subtest 6: Fork copies only current thread
 * - Subtest 7: Thread wrapper calls ThreadExit on function return
 */
void thread_tests(void);

/* ---- Thread Test Helper Functions ---- */

/**
 * @brief Test creating maximum threads and verify limit.
 * @param passed Pointer to store result (1 = passed, 0 = failed)
 */
void subtest_max_threads(int *passed);

/**
 * @brief Test that TIDs are reused after thread deletion.
 * @param passed Pointer to store result (1 = passed, 0 = failed)
 */
void subtest_tid_reuse(int *passed);

/**
 * @brief Test that process terminates when last thread exits.
 * This must be run in a child process.
 */
void subtest_last_thread_exits(void);

/**
 * @brief Test master thread reassignment.
 * This must be run in a child process.
 */
void subtest_master_reassignment(void);

/**
 * @brief Test that fork copies only the current thread.
 * @param passed Pointer to store result (1 = passed, 0 = failed)
 */
void subtest_fork_single_thread(int *passed);

/**
 * @brief Test that thread wrapper calls ThreadExit on function return.
 *
 * Creates a thread that returns without calling ThreadExit explicitly.
 * The wrapper should automatically call ThreadExit, preventing a crash.
 * @param passed Pointer to store result (1 = passed, 0 = failed)
 */
void subtest_wrapper_calls_exit(int *passed);

/* ---- Utility Functions ---- */

/**
 * @brief Wait for a specified number of ticks.
 *
 * This function waits for the specified number of ticks, printing
 * PID/TID info at start and end of the wait period.
 * @param ticks Number of ticks to wait.
 */
void waitTicks(int ticks);

/**
 * @brief Write current process and thread information.
 *
 * This function writes the current PID and TID in the format: [PID X] [TID Y]
 */
void write_current_info(void);

/**
 * @brief Simple thread work function that sets a completion flag.
 *
 * This function does minimal work and sets its assigned flag.
 * @param arg Pointer to the flag index to set when complete.
 */
void simple_thread_func(void *arg);

/**
 * @brief Thread function that works for a specified time and then exits.
 *
 * This function is executed by created threads. It works for the specified
 * number of ticks and then exits, setting a flag to indicate completion.
 * @param arg Pointer to integer containing the number of ticks to work.
 */
void thread_work(void *arg);

/**
 * @brief Thread function that blocks after setting a flag.
 * @param arg Pointer to flag index to set before blocking.
 */
void thread_block_func(void *arg);

/**
 * @brief Wait for a synchronization flag to be set.
 *
 * Spins until the specified flag becomes non-zero.
 * @param flag_index Index of the flag to wait for.
 */
void wait_for_flag(int flag_index);

/**
 * @brief Set a synchronization flag.
 * @param flag_index Index of the flag to set.
 */
void set_flag(int flag_index);

/**
 * @brief Clear all synchronization flags.
 */
void clear_all_flags(void);

/**
 * @brief Check if a synchronization flag is set.
 * @param flag_index Index of the flag to check.
 * @return 1 if set, 0 if not set.
 */
int is_flag_set(int flag_index);

#endif /* __PROJECT_TEST_H__ */
