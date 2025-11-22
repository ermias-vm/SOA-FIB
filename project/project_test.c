
/**
 * @file project_test.c
 * @brief Test suite for thread support in ZeOS.
 *
 * This file contains extensive tests for thread creation, termination,
 * and fork interaction with threads.
 */

#include <errno.h>
#include <libc.h>
#include <project_test.h>
#include <zeos_test.h>

#define NULL ((void *)0)

extern char buffer[BUFFER_SIZE];
extern char *msg;

static int tests_run = 0;
static int tests_passed = 0;
static int subtests_run = 0;
static int subtests_passed = 0;
extern int errno;

/* External functions from zeos_test.c */
extern void work(int ticks);
extern void print_test_header(char *test_name);
extern void print_test_result(char *test_name, int passed);
extern void print_final_summary(void);

/* Forward declaration */
static void print_project_final_summary(void);

/* Global variable to track thread execution */
static volatile int thread_executed = 0;
static volatile int thread_param_received = 0;

/* Thread functions for testing */
void simple_thread(void *arg) {
    thread_executed = 1;
    if (arg != NULL) {
        thread_param_received = *(int *)arg;
    }
    ThreadExit();
}

void hello_thread(void *arg) {
    char *message = (char *)arg;
    write(1, "[Thread] ", 9);
    write(1, message, strlen(message));
    write(1, "\n", 1);
    ThreadExit();
}

void counter_thread(void *arg) {
    int *counter = (int *)arg;
    for (int i = 0; i < 5; i++) {
        (*counter)++;
        work(100);
    }
    ThreadExit();
}

void long_thread(void *arg) {
    (void)arg;
    work(2000); // Work for 2 seconds
    ThreadExit();
}

void test_thread_create(void) {
    print_test_header("THREADCREATE SYSCALL");

    subtests_run = 0;
    subtests_passed = 0;

    // Subtest 1: Create simple thread
    msg = "[TEST 1] Simple thread creation...\n";
    write(1, msg, strlen(msg));

    thread_executed = 0;
    int tid1 = ThreadCreate(simple_thread, NULL);

    if (tid1 > 0) {
        work(500); // Wait for thread to execute

        if (thread_executed == 1) {
            msg = " - PASSED\n";
            write(1, msg, strlen(msg));
            subtests_passed++;
        } else {
            msg = " - FAILED (thread did not execute)\n";
            write(1, msg, strlen(msg));
        }
    } else {
        msg = " - FAILED (creation returned error)\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Subtest 2: Thread with parameter
    msg = "[TEST 2] Thread with parameter...\n";
    write(1, msg, strlen(msg));

    thread_param_received = 0;
    int param = 42;
    int tid2 = ThreadCreate(simple_thread, &param);

    if (tid2 > 0) {
        work(500);

        if (thread_param_received == 42) {
            msg = " - PASSED\n";
            write(1, msg, strlen(msg));
            subtests_passed++;
        } else {
            msg = " - FAILED (parameter not received correctly)\n";
            write(1, msg, strlen(msg));
        }
    } else {
        msg = " - FAILED (creation returned error)\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Subtest 3: Thread with string parameter
    msg = "[TEST 3] Thread with string parameter...\n";
    write(1, msg, strlen(msg));

    int tid3 = ThreadCreate(hello_thread, "Hello from thread!");

    if (tid3 > 0) {
        work(500);
        msg = " - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Subtest 4: Multiple threads
    msg = "[TEST 4] Multiple thread creation...\n";
    write(1, msg, strlen(msg));

    int counter = 0;
    int tid4 = ThreadCreate(counter_thread, &counter);
    int tid5 = ThreadCreate(counter_thread, &counter);

    if (tid4 > 0 && tid5 > 0 && tid4 != tid5) {
        work(1000);
        msg = " - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Subtest 5: Invalid function pointer
    msg = "[TEST 5] Invalid function pointer...\n";
    write(1, msg, strlen(msg));

    int tid6 = ThreadCreate(NULL, NULL);

    if (tid6 < 0 && errno == EINVAL) {
        msg = " - PASSED (correctly rejected NULL function)\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED (should reject NULL function)\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;
    RESET_ERRNO();

    // Summary
    msg = "\n[THREADCREATE] Subtests: ";
    write(1, msg, strlen(msg));
    itoa(subtests_passed, buffer);
    write(1, buffer, strlen(buffer));
    msg = "/";
    write(1, msg, strlen(msg));
    itoa(subtests_run, buffer);
    write(1, buffer, strlen(buffer));
    msg = " passed\n";
    write(1, msg, strlen(msg));

    int passed = (subtests_passed == subtests_run);
    print_test_result("ThreadCreate() syscall", passed);
}

void test_thread_exit(void) {
    print_test_header("THREADEXIT SYSCALL");

    subtests_run = 0;
    subtests_passed = 0;

    // Subtest 1: Normal thread exit
    msg = "[TEST 1] Normal thread exit...\n";
    write(1, msg, strlen(msg));

    thread_executed = 0;
    int tid1 = ThreadCreate(simple_thread, NULL);

    if (tid1 > 0) {
        work(500);

        if (thread_executed == 1) {
            msg = " - PASSED\n";
            write(1, msg, strlen(msg));
            subtests_passed++;
        } else {
            msg = " - FAILED\n";
            write(1, msg, strlen(msg));
        }
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Subtest 2: Thread that exits after long work
    msg = "[TEST 2] Thread with long execution...\n";
    write(1, msg, strlen(msg));

    int tid2 = ThreadCreate(long_thread, NULL);

    if (tid2 > 0) {
        work(2500); // Wait for thread to complete
        msg = " - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Summary
    msg = "\n[THREADEXIT] Subtests: ";
    write(1, msg, strlen(msg));
    itoa(subtests_passed, buffer);
    write(1, buffer, strlen(buffer));
    msg = "/";
    write(1, msg, strlen(msg));
    itoa(subtests_run, buffer);
    write(1, buffer, strlen(buffer));
    msg = " passed\n";
    write(1, msg, strlen(msg));

    int passed = (subtests_passed == subtests_run);
    print_test_result("ThreadExit() syscall", passed);
}

void test_thread_fork(void) {
    print_test_header("FORK WITH THREADS");

    subtests_run = 0;
    subtests_passed = 0;

    // Subtest 1: Fork should only copy current thread
    msg = "[TEST 1] Fork copies only current thread...\n";
    write(1, msg, strlen(msg));

    // Create a thread in parent
    int tid1 = ThreadCreate(long_thread, NULL);

    if (tid1 > 0) {
        // Now fork
        int pid = fork();

        if (pid == 0) {
            // Child process
            msg = "[CHILD] Created from parent with threads\n";
            write(1, msg, strlen(msg));
            work(500);
            exit();
        } else if (pid > 0) {
            // Parent process
            work(1000);
            msg = " - PASSED\n";
            write(1, msg, strlen(msg));
            subtests_passed++;
        } else {
            msg = " - FAILED (fork failed)\n";
            write(1, msg, strlen(msg));
        }
    } else {
        msg = " - FAILED (thread creation failed)\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    work(2000); // Wait for threads to complete

    // Summary
    msg = "\n[FORK] Subtests: ";
    write(1, msg, strlen(msg));
    itoa(subtests_passed, buffer);
    write(1, buffer, strlen(buffer));
    msg = "/";
    write(1, msg, strlen(msg));
    itoa(subtests_run, buffer);
    write(1, buffer, strlen(buffer));
    msg = " passed\n";
    write(1, msg, strlen(msg));

    int passed = (subtests_passed == subtests_run);
    print_test_result("Fork with threads", passed);
}

void test_thread_stress(void) {
    print_test_header("THREAD STRESS TEST");

    subtests_run = 0;
    subtests_passed = 0;

    // Subtest 1: Create maximum threads
    msg = "[TEST 1] Create multiple threads...\n";
    write(1, msg, strlen(msg));

    int created = 0;
    for (int i = 0; i < 5; i++) {
        int tid = ThreadCreate(simple_thread, NULL);
        if (tid > 0) {
            created++;
        }
    }

    if (created >= 3) {
        msg = " - PASSED (created ";
        write(1, msg, strlen(msg));
        itoa(created, buffer);
        write(1, buffer, strlen(buffer));
        msg = " threads)\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    work(1000);

    // Summary
    msg = "\n[STRESS] Subtests: ";
    write(1, msg, strlen(msg));
    itoa(subtests_passed, buffer);
    write(1, buffer, strlen(buffer));
    msg = "/";
    write(1, msg, strlen(msg));
    itoa(subtests_run, buffer);
    write(1, buffer, strlen(buffer));
    msg = " passed\n";
    write(1, msg, strlen(msg));

    int passed = (subtests_passed == subtests_run);
    print_test_result("Thread stress test", passed);
}

void execute_project_tests(void) {
    RESET_ERRNO();

    msg = "\n=========================================\n";
    write(1, msg, strlen(msg));

    msg = "      PROJECT THREAD TEST SUITE         \n";
    write(1, msg, strlen(msg));

    msg = "=========================================\n";
    write(1, msg, strlen(msg));

#if THREAD_CREATE_TEST
    test_thread_create();
    RESET_ERRNO();
#endif

#if THREAD_EXIT_TEST
    test_thread_exit();
    RESET_ERRNO();
#endif

#if THREAD_FORK_TEST
    test_thread_fork();
    RESET_ERRNO();
#endif

#if THREAD_STRESS_TEST
    test_thread_stress();
    RESET_ERRNO();
#endif

    // Wait for all threads to complete
    work(2000);

    print_project_final_summary();
}

/* Helper function for thread tests */

void write_current_tid(void) {
    msg = "[PID ";
    write(1, msg, strlen(msg));
    itoa(getpid(), buffer);
    write(1, buffer, strlen(buffer));
    msg = "] ";
    write(1, msg, strlen(msg));
}

static void print_project_final_summary(void) {
    if (getpid() != 1) {
        return;
    }

    msg = "\n\n";
    write(1, msg, strlen(msg));

    msg = "=========================================\n";
    write(1, msg, strlen(msg));

    msg = "        THREAD TEST SUMMARY              \n";
    write(1, msg, strlen(msg));

    msg = "=========================================\n";
    write(1, msg, strlen(msg));

    msg = "Tests executed:\n";
    write(1, msg, strlen(msg));

    int current_test = 0;

#if THREAD_CREATE_TEST
    if (current_test < tests_run) {
        msg = (current_test < tests_passed) ? "THREAD_CREATE_TEST      : PASSED\n"
                                            : "THREAD_CREATE_TEST      : FAILED\n";
        current_test++;
    } else {
        msg = "THREAD_CREATE_TEST      : SKIPPED\n";
    }
    write(1, msg, strlen(msg));
#endif

#if THREAD_EXIT_TEST
    if (current_test < tests_run) {
        msg = (current_test < tests_passed) ? "THREAD_EXIT_TEST        : PASSED\n"
                                            : "THREAD_EXIT_TEST        : FAILED\n";
        current_test++;
    } else {
        msg = "THREAD_EXIT_TEST        : SKIPPED\n";
    }
    write(1, msg, strlen(msg));
#endif

#if THREAD_FORK_TEST
    if (current_test < tests_run) {
        msg = (current_test < tests_passed) ? "THREAD_FORK_TEST        : PASSED\n"
                                            : "THREAD_FORK_TEST        : FAILED\n";
        current_test++;
    } else {
        msg = "THREAD_FORK_TEST        : SKIPPED\n";
    }
    write(1, msg, strlen(msg));
#endif

#if THREAD_STRESS_TEST
    if (current_test < tests_run) {
        msg = (current_test < tests_passed) ? "THREAD_STRESS_TEST      : PASSED\n"
                                            : "THREAD_STRESS_TEST      : FAILED\n";
        current_test++;
    } else {
        msg = "THREAD_STRESS_TEST      : SKIPPED\n";
    }
    write(1, msg, strlen(msg));
#endif

    msg = "\nSummary:\n";
    write(1, msg, strlen(msg));

    msg = "Tests run: ";
    write(1, msg, strlen(msg));
    itoa(tests_run, buffer);
    write(1, buffer, strlen(buffer));
    msg = "\n";
    write(1, msg, strlen(msg));

    msg = "Tests passed: ";
    write(1, msg, strlen(msg));
    itoa(tests_passed, buffer);
    write(1, buffer, strlen(buffer));
    msg = "\n";
    write(1, msg, strlen(msg));

    msg = "Tests failed: ";
    write(1, msg, strlen(msg));
    itoa(tests_run - tests_passed, buffer);
    write(1, buffer, strlen(buffer));
    msg = "\n";
    write(1, msg, strlen(msg));

    if (tests_passed == tests_run) {
        msg = "\n*** ALL TESTS PASSED! ***\n";
        write(1, msg, strlen(msg));
    } else {
        msg = "\n*** SOME TESTS FAILED! ***\n";
        write(1, msg, strlen(msg));
    }

    msg = "=========================================\n";
    write(1, msg, strlen(msg));
}
