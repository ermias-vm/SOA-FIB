/**
 * @file project_test.h
 * @brief Test suite interface for ZeOS project milestones.
 *
 * This header defines test function prototypes and testing
 * utilities for thread and keyboard functionality verification.
 */

#ifndef __PROJECT_TEST_H__
#define __PROJECT_TEST_H__

/**********************/
/**   Test Enables   **/
/**********************/

// clang-format off
#define THREAD_TEST             1   /**< Enable/disable thread tests */
#define KEYBOARD_TEST           1   /**< Enable/disable keyboard tests */
#define SCREEN_TEST             1   /**< Enable/disable screen functional tests */
#define SCREEN_PERFORMANCE_TEST 1   /**< Enable/disable screen performance test */
#define WAITFORTICK_TEST        1   /**< Enable/disable WaitForTick tests */

#define IDLE_SWITCH_TEST        1   /**< Test idle switch (exits init) */
// clang-format on

/** Reset errno macro */
#define RESET_ERRNO() errno = 0

/** Default Buffer size */
#define BUFFER_SIZE 256

#define MIN_WORK_TIME 50      /**< 50ms - minimal delay after flag sync */
#define SHORT_WORK_TIME 100   /**< 100ms - short operations */
#define DEFAULT_WORK_TIME 200 /**< 200ms - default delay */
#define MEDIUM_WORK_TIME 300  /**< 300ms - child process completion */
#define LONG_WORK_TIME 500    /**< 500ms - longer thread work */

#define VISUAL_DISPLAY_TIME 700 /**< 700ms - time to display visual patterns */
#define VISUAL_PAUSE_TIME 400   /**< 400ms - pause before visual changes */

#define KBD_WAIT_TIME 3000  /**< 3 seconds for keyboard test */
#define KBD_PAUSE_TIME 1000 /**< 1 second pause between keyboard subtests */

#define MAX_THREADS_PER_PROCESS 10 /** Maximum threads per process (TIDs X0-X9) */
#define MAX_SYNC_FLAGS 10          /** Synchronization flags for thread tests */

#define KBD_MAX_KEYS 10 /* Maximum keys to track in keyboard test */

#define SCREEN_WRITE_ITERATIONS 1000 /** Number of iterations for screen write performance test */

/****************************************/
/**    Utility Functions               **/
/****************************************/

/**
 * @brief Wait for a specified number of ticks.
 *
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
 * @brief Clear screen buffer by filling it with spaces.
 *
 * This function clears the specified screen file descriptor by writing
 * a buffer filled with spaces and default color attributes.
 *
 * @param fd File descriptor to clear.
 * @return Number of bytes written, or negative error code.
 */
int clear_screen_buffer(int fd);

/****************************************/
/**    Synchronization Functions       **/
/****************************************/

/**
 * @brief Clear all synchronization flags.
 */
void clear_all_flags(void);

/**
 * @brief Set a synchronization flag.
 *
 * @param flag_index Index of the flag to set.
 */
void set_flag(int flag_index);

/**
 * @brief Check if a synchronization flag is set.
 *
 * @param flag_index Index of the flag to check.
 * @return 1 if set, 0 if not set.
 */
int is_flag_set(int flag_index);

/**
 * @brief Wait for a synchronization flag to be set.
 *
 * @param flag_index Index of the flag to wait for.
 */
void wait_for_flag(int flag_index);

/****************************************/
/**    Thread Work Functions           **/
/****************************************/

/**
 * @brief Simple thread work function that sets a completion flag.
 *
 * @param arg Pointer to the flag index to set when complete.
 */
void simple_thread_func(void *arg);

/**
 * @brief Thread function that works for a specified time and then exits.
 *
 * @param arg Pointer to integer containing the number of ticks to work.
 */
void thread_work(void *arg);

/**
 * @brief Thread function that blocks after setting a flag.
 *
 * @param arg Pointer to flag index to set before blocking.
 */
void thread_block_func(void *arg);

/****************************************/
/**    Thread Test Functions           **/
/****************************************/

/**
 * @brief Test creating maximum threads and verify limit.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_max_threads(int *passed);

/**
 * @brief Test that TIDs are reused after thread deletion.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_tid_reuse(int *passed);

/**
 * @brief Test that process terminates when last thread exits.
 *
 * This must be run in a child process.
 */
void subtest_last_thread_exits(void);

/**
 * @brief Test master thread reassignment.
 *
 * This must be run in a child process.
 */
void subtest_master_reassignment(void);

/**
 * @brief Test that fork copies only the current thread.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_fork_single_thread(int *passed);

/**
 * @brief Test that thread wrapper calls ThreadExit on function return.
 *
 * Creates a thread that returns without calling ThreadExit explicitly.
 * The wrapper should automatically call ThreadExit, preventing a crash.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_wrapper_calls_exit(int *passed);

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

/****************************************/
/**    Keyboard Test Functions         **/
/****************************************/

/**
 * @brief Test registering a keyboard handler.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_kbd_register(int *passed);

/**
 * @brief Test disabling keyboard handler with NULL.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_kbd_disable(int *passed);

/**
 * @brief Test that keyboard handler is called on key events.
 *
 * This test registers a handler and waits for key presses.
 * User must press keys to verify the handler is called.
 * Shows the first 10 keys pressed with their scancodes and characters.
 * Test ends after 3 seconds or 10 keys, whichever comes first.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_kbd_handler_called(int *passed);

/**
 * @brief Test that syscalls return EINPROGRESS inside keyboard handler.
 *
 * This test verifies that system calls made from within a keyboard
 * event handler return -EINPROGRESS as specified.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_kbd_einprogress(int *passed);

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

/****************************************/
/**    Screen Support Test Functions   **/
/****************************************/

/**
 * @brief Test invalid file descriptor access.
 *
 * Tests that write to invalid file descriptors returns appropriate errors.
 *
 * @return 1 if test passed, 0 if failed.
 */
int test_screen_write_invalid_fd(void);

/**
 * @brief Test basic screen buffer write functionality.
 *
 * This test creates a simple checkerboard pattern and writes it
 * to the screen buffer (fd=10). Verifies that write() returns
 * the correct number of bytes written (4000 for full screen).
 *
 * @return 1 if test passed, 0 if failed.
 */
int test_screen_write_basic(void);

/**
 * @brief Test screen buffer size limits.
 *
 * Tests that write(10, ...) properly handles buffer sizes larger than screen.
 *
 * @return 1 if test passed, 0 if failed.
 */
int test_screen_write_size_limits(void);

/**
 * @brief Test screen buffer visual patterns display.
 *
 * This test displays the three different screen patterns with delays
 * to visually verify that the screen buffer is working correctly.
 * Waits 0.5 seconds before starting. Displays each pattern, waits 1.5 seconds
 * to allow viewing, clears the screen, waits 0.5 seconds, then displays the next pattern.
 *
 * @return 1 if test passed, 0 if failed.
 */
int test_screen_visual_patterns(void);

/**
 * @brief Test screen buffer performance timing.
 *
 * This test measures the time required to write 30 consecutive
 * frames to the screen buffer using gettime() before and after
 * the writes. Reports total time and average time per frame.
 * Clears screen before starting the performance test and before
 * printing the results.
 *
 * @return 1 if test passed, 0 if failed.
 */
int test_screen_write_performance(void);

/**
 * @brief Main screen support test suite (Functional Tests).
 *
 * This function runs screen buffer functional tests:
 * - Subtest 1: Invalid file descriptor access (write to fd 99)
 * - Subtest 2: Basic screen buffer write (checkerboard pattern)
 * - Subtest 3: Screen buffer size limits (truncation test)
 * - Subtest 4: Screen visual patterns display (with clear between patterns)
 *
 * Tests the sys_write_screen() functionality through fd=10 writes.
 * Verifies error handling, basic functionality, and size limits.
 */
void screen_tests(void);

/****************************************/
/**    WaitForTick Test Functions      **/
/****************************************/

/**
 * @brief Test basic WaitForTick functionality.
 *
 * Verifies that WaitForTick blocks the thread and returns after a tick.
 * Measures time before and after to confirm a tick occurred.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_waitfortick_basic(int *passed);

/**
 * @brief Test multiple threads waiting for tick simultaneously.
 *
 * Creates multiple threads that all call WaitForTick and verifies
 * they are all woken up when the next tick occurs.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_waitfortick_multiple_threads(int *passed);

/**
 * @brief Test WaitForTick timing accuracy.
 *
 * Calls WaitForTick multiple times and measures total time to verify
 * it approximately matches the expected number of ticks.
 *
 * @param passed Pointer to store result (1 = passed, 0 = failed).
 */
void subtest_waitfortick_timing(int *passed);

/**
 * @brief Main WaitForTick test suite.
 *
 * This function runs WaitForTick tests:
 * - Subtest 1: Basic functionality and return value
 * - Subtest 2: Multiple threads waiting simultaneously
 * - Subtest 3: Timing accuracy test
 */
void waitfortick_tests(void);

/****************************************/
/**    Main Test Entry Point           **/
/****************************************/

/**
 * @brief Execute all project test suites.
 *
 * This function runs all enabled test suites (threads, keyboard).
 * It coordinates the execution of individual test functions and provides
 * a test summary. Thread TID 10 (init's first thread) orchestrates all tests.
 */
void execute_project_tests(void);

#endif /* __PROJECT_TEST_H__ */
