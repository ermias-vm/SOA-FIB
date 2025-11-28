/**
 * @file project_test.h
 * @brief Test suite interface for ZeOS project milestones.
 *
 * This header defines test function prototypes and testing
 * utilities for thread and keyboard functionality verification.
 */

#ifndef __PROJECT_TEST_H__
#define __PROJECT_TEST_H__

/* ============================================================
 *                    CONFIGURATION MACROS
 * ============================================================ */

// clang-format off
#define THREAD_TEST         0   /* Enable/disable thread tests */
#define KEYBOARD_TEST       1   /* Enable/disable keyboard tests */
#define IDLE_SWITCH_TEST    0   /* Test idle switch (exits init) */
// clang-format on

/* ============================================================
 *                    CONSTANTS AND MACROS
 * ============================================================ */

/* Reset errno macro */
#define RESET_ERRNO() errno = 0

/* Default Buffer size */
#define BUFFER_SIZE 256

/* Null pointer definition */
#define NULL ((void *)0)

/* Timing constants (in ticks, ~1000 ticks = 1 second) */
#define MIN_WORK_TIME 200     /* 200ms */
#define SHORT_WORK_TIME 500   /* 500ms */
#define MEDIUM_WORK_TIME 1500 /* 1.5 seconds */
#define LONG_WORK_TIME 3000   /* 3 seconds */
#define KBD_WAIT_TIME 5000    /* 5 seconds for keyboard test */

/* Thread test constants */
#define MAX_THREADS_PER_PROCESS 10 /* Maximum threads per process (TIDs X0-X9) */
#define MAX_SYNC_FLAGS 10          /* Synchronization flags for thread tests */

/* Keyboard test constants */
#define KBD_MAX_KEYS 5 /* Maximum keys to track in keyboard test */

/* ============================================================
 *                    MAIN TEST ENTRY POINTS
 * ============================================================ */

/**
 * @brief Execute all project test suites.
 *
 * This function runs all enabled test suites (threads, keyboard).
 * It coordinates the execution of individual test functions and provides
 * a test summary. Thread TID 10 (init's first thread) orchestrates all tests.
 */
void execute_project_tests(void);

/* ============================================================
 *                    THREAD TEST FUNCTIONS
 * ============================================================ */

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

/* ============================================================
 *                    KEYBOARD TEST FUNCTIONS
 * ============================================================ */

/**
 * @brief Main keyboard test suite.
 *
 * This function runs keyboard support tests:
 * - Subtest 1: Register keyboard handler successfully
 * - Subtest 2: Disable keyboard handler with NULL
 * - Subtest 3: Verify handler is called on key events
 * - Subtest 4: Verify syscalls return EINPROGRESS inside handler
 */
void keyboard_tests(void);

/**
 * @brief Test registering a keyboard handler.
 * @param passed Pointer to store result (1 = passed, 0 = failed)
 */
void subtest_kbd_register(int *passed);

/**
 * @brief Test disabling keyboard handler with NULL.
 * @param passed Pointer to store result (1 = passed, 0 = failed)
 */
void subtest_kbd_disable(int *passed);

/**
 * @brief Test that keyboard handler is called on key events.
 *
 * This test registers a handler and waits for key presses.
 * User must press keys to verify the handler is called.
 * Shows the last 5 keys pressed with their scancodes and characters.
 * @param passed Pointer to store result (1 = passed, 0 = failed)
 */
void subtest_kbd_handler_called(int *passed);

/**
 * @brief Test that syscalls return EINPROGRESS inside keyboard handler.
 *
 * This test verifies that system calls made from within a keyboard
 * event handler return -EINPROGRESS as specified.
 * @param passed Pointer to store result (1 = passed, 0 = failed)
 */
void subtest_kbd_einprogress(int *passed);

/* ============================================================
 *                    UTILITY FUNCTIONS
 * ============================================================ */

/**
 * @brief Wait for a specified number of ticks.
 * @param ticks Number of ticks to wait.
 */
void waitTicks(int ticks);

/**
 * @brief Write current process and thread information.
 *
 * This function writes the current PID and TID in the format: [PID X] [TID Y]
 */
void write_current_info(void);

/* ============================================================
 *                    THREAD WORK FUNCTIONS
 * ============================================================ */

/**
 * @brief Simple thread work function that sets a completion flag.
 * @param arg Pointer to the flag index to set when complete.
 */
void simple_thread_func(void *arg);

/**
 * @brief Thread function that works for a specified time and then exits.
 * @param arg Pointer to integer containing the number of ticks to work.
 */
void thread_work(void *arg);

/**
 * @brief Thread function that blocks after setting a flag.
 * @param arg Pointer to flag index to set before blocking.
 */
void thread_block_func(void *arg);

/* ============================================================
 *                    SYNCHRONIZATION UTILITIES
 * ============================================================ */

/**
 * @brief Wait for a synchronization flag to be set.
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
