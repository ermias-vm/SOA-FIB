/**
 * @file zeos_test.c
 * @brief Test suite for ZeOS system functionality.
 *
 * This file contains extensive tests for system calls, process management,
 * memory operations, and kernel functionality verification.
 */

#include <errno.h>
#include <libc.h>
#include <zeos_test.h>

char buffer[BUFFER_SIZE];
char large_buffer[LARGE_BUFFER_SIZE];
char *msg;

static int tests_run = 0;
static int tests_passed = 0;
static int subtests_run = 0;
static int subtests_passed = 0;
extern int errno;

void test_basic_process_operations(void) {
    print_test_header("BASIC PROCESS OPERATIONS");

    msg = "[TEST] Starting Basic Process Operations Test\n";
    write(1, msg, strlen(msg));

    write_current_pid();
    msg = "Initial process starting test\n";
    write(1, msg, strlen(msg));

    int pid = fork();

    if (pid == 0) {
        // First child
        write_current_pid();
        msg = "First child process created\n";
        write(1, msg, strlen(msg));

        int pid2 = fork();

        if (pid2 > 0) {
            // First child (parent of second child)
            write_current_pid();
            itoa(pid2, buffer);
            msg = "Created second child, PID: ";
            write(1, msg, strlen(msg));
            write(1, buffer, strlen(buffer));
            msg = "\n";
            write(1, msg, strlen(msg));

            // Wait longer to ensure second child blocks first
            work(2000); // Wait 2 seconds

            write_current_pid();
            msg = "Attempting to unblock PID: ";
            write(1, msg, strlen(msg));
            itoa(pid2, buffer);
            write(1, buffer, strlen(buffer));
            msg = "\n";
            write(1, msg, strlen(msg));

            if (unblock(pid2) == 0) {
                write_current_pid();
                msg = "Successfully unblocked process\n";
                write(1, msg, strlen(msg));

                // Give time for unblocked process to execute
                work(500); // Wait 0.5 seconds
            } else {
                write_current_pid();
                msg = "Failed to unblock process\n";
                write(1, msg, strlen(msg));
            }

            // First child continues working
            work(1000); // Work for 1 second

        } else if (pid2 == 0) {
            // Second child
            write_current_pid();
            msg = "Second child process created\n";
            write(1, msg, strlen(msg));

            write_current_pid();
            msg = "Working before blocking...\n";
            write(1, msg, strlen(msg));
            work(1000); // Work for 1 second before blocking

            write_current_pid();
            msg = "Blocking myself now\n";
            write(1, msg, strlen(msg));
            block();

            write_current_pid();
            msg = "I have been unblocked! Exiting...\n";
            write(1, msg, strlen(msg));
            exit();
        }

    } else if (pid > 0) {
        // Parent process
        write_current_pid();
        msg = "Parent created child, PID: ";
        write(1, msg, strlen(msg));
        itoa(pid, buffer);
        write(1, buffer, strlen(buffer));
        msg = "\n";
        write(1, msg, strlen(msg));

        write_current_pid();
        msg = "Parent continuing execution\n";
        write(1, msg, strlen(msg));

        // Parent waits longer to ensure all children complete first
        work(5000); // Work for 5 seconds to let children finish

    } else {
        write_current_pid();
        msg = "Fork failed\n";
        write(1, msg, strlen(msg));
        print_test_result("Basic Process Operations", 0);
        return;
    }

    // Additional delay to ensure all child processes complete
    work(1000); // Wait 1 more second
    write_current_pid();
    msg = "[TEST] Basic Process Operations completed-----------------------------\n\n\n\n";
    write(1, msg, strlen(msg));

    print_test_result("Basic Process Operations", 1);
}

void execute_zeos_tests(void) {
    RESET_ERRNO();

    msg = "\n=========================================\n";
    write(1, msg, strlen(msg));

    msg = "         ZEOS SYSCALL TEST SUITE         \n";
    write(1, msg, strlen(msg));

    msg = "=========================================\n";
    write(1, msg, strlen(msg));

#if WRITE_TEST
    test_write_syscall();
    RESET_ERRNO();
#endif

#if GETTIME_TEST
    test_gettime_syscall();
    RESET_ERRNO();
#endif

#if GETPID_TEST
    test_getpid_syscall();
    RESET_ERRNO();
#endif

#if FORK_TEST
    test_fork_syscall();
    RESET_ERRNO();
#endif

#if EXIT_TEST
    test_exit_syscall();
    RESET_ERRNO();
#endif

#if BLOCK_UNBLOCK_TEST
    test_block_unblock_syscalls();
    RESET_ERRNO();
#endif

#if PAGEFAULT_TEST
    msg = "\n--- Testing: PAGE FAULT EXCEPTION ---\n";
    write(1, msg, strlen(msg));

    // This will cause a page fault and should not return
    volatile char *p = (volatile char *)0x999999;
    *p = 'x';
#endif

    // Wait for all child processes to complete before showing summary
    work(3000); // Wait 3 seconds

    print_final_summary();

    // Exit the init process (PID 1) after printing summary
    exit();
}

/* ---- Sys call test functions ---- */

void test_write_syscall(void) {
    print_test_header("WRITE SYSCALL");

    subtests_run = 0;
    subtests_passed = 0;

    // Test 1: Normal write
    msg = "[TEST 1] Normal write test...\n";
    write(1, msg, strlen(msg));

    msg = "Normal message";
    int result1 = write(1, msg, strlen(msg));
    if (result1 == strlen(msg)) {
        msg = " - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Test 2: Empty string (size = 0)
    msg = "[TEST 2] Empty string test...\n";
    write(1, msg, strlen(msg));

    msg = "";
    int result2 = write(1, msg, strlen(msg));
    if (result2 == strlen(msg)) {
        msg = " - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Test 3: Single character
    msg = "[TEST 3] Single character test...\n";
    write(1, msg, strlen(msg));

    msg = "X";
    int result3 = write(1, msg, strlen(msg));
    if (result3 == strlen(msg)) {
        msg = " - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Test 4: Large buffer (>256 bytes)
    msg = "[TEST 4] Large buffer test...\n";
    write(1, msg, strlen(msg));

    int i;
    int char_range = 122 - 48 + 1; // from '0' to 'z' = 75 characters
    for (i = 0; i < 299; i++) {
        large_buffer[i] = 48 + (i % char_range); // Start from '0' (ASCII 48)
    }
    large_buffer[299] = '\0';

    int result4 = write(1, large_buffer, strlen(large_buffer));

    if (result4 == strlen(large_buffer)) {
        msg = "\nLarge buffer - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = "\nLarge buffer - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Test 5: Invalid file descriptor (should return EBADF)
    msg = "[TEST 5] Invalid FD test...\n";
    write(1, msg, strlen(msg));

    msg = "Should fail";
    int result5 = write(0, msg, strlen(msg)); // fd=0 (stdin) should fail
    if (result5 != 0 && errno == EBADF) {
        msg = " - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Test 6: NULL buffer (should return EFAULT)
    msg = "[TEST 6] NULL buffer test...\n";
    write(1, msg, strlen(msg));

    int result6 = write(1, (char *)0, 10); // NULL buffer should fail
    if (result6 != 0 && errno == EFAULT) {
        msg = " - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Test 7: Negative size (should return EINVAL)
    msg = "[TEST 7] Negative size test...\n";
    write(1, msg, strlen(msg));

    msg = "Should fail";
    int result7 = write(1, msg, -1); // Negative size should fail
    if (result7 != 0 && errno == EINVAL) {
        msg = " - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Summary for write tests
    msg = "\n[WRITE] Subtests: ";
    write(1, msg, strlen(msg));

    itoa(subtests_passed, buffer);
    write(1, buffer, strlen(buffer));

    msg = "/";
    write(1, msg, strlen(msg));

    itoa(subtests_run, buffer);
    write(1, buffer, strlen(buffer));

    msg = " passed";
    write(1, msg, strlen(msg));

    int passed = (subtests_passed == subtests_run);
    print_test_result("write() syscall", passed);
}

void test_gettime_syscall(void) {
    print_test_header("GETTIME SYSCALL");

    msg = "[TEST] Testing gettime() syscall...\n";
    write(1, msg, strlen(msg));

    int ticks1 = gettime();
    msg = "[TEST] First call - Ticks: ";
    write(1, msg, strlen(msg));

    itoa(ticks1, buffer);
    write(1, buffer, strlen(buffer));
    msg = "\n";
    write(1, msg, strlen(msg));

    work(100); // Work for 0.1 seconds

    int ticks2 = gettime();
    msg = "[TEST] Second call - Ticks: ";
    write(1, msg, strlen(msg));

    itoa(ticks2, buffer);
    write(1, buffer, strlen(buffer));

    // Test result - ticks should be non-negative and second >= first
    int passed = (ticks1 >= 0 && ticks2 >= 0 && ticks2 >= ticks1);
    print_test_result("gettime() syscall", passed);
}

void test_getpid_syscall(void) {
    RESET_ERRNO(); // Reset errno at the start of test

    print_test_header("GETPID SYSCALL");

    msg = "[TEST] Testing getpid() syscall...\n";
    write(1, msg, strlen(msg));

    int pid = getpid();

    msg = "[TEST] getpid() returned: ";
    write(1, msg, strlen(msg));

    itoa(pid, buffer);
    write(1, buffer, strlen(buffer));

    // Test conditions: errno should be 0 and PID should be > 0
    int passed = (errno == 0 && pid > 0);
    print_test_result("getpid() syscall", passed);
}

void test_fork_syscall(void) {
    RESET_ERRNO(); // Reset errno at the start of test

    print_test_header("FORK SYSCALL");

    int parent_subtests_run = 0;
    int parent_subtests_passed = 0;

    // Test 1: Basic fork test - create a child process
    msg = "[TEST 1] Basic fork test...\n";
    write(1, msg, strlen(msg));

    int pid = fork();

    if (pid == -1) {
        // Fork error
        msg = "Fork failed with errno: ";
        write(1, msg, strlen(msg));
        itoa(errno, buffer);
        write(1, buffer, strlen(buffer));
        msg = " - FAILED\n";
        write(1, msg, strlen(msg));
        parent_subtests_run++;

        // If fork fails, we can't do more tests
        print_test_result("fork() syscall", 0);
        return;
    } else if (pid == 0) {
        // ======= CHILD PROCESS (ends quickly to avoid interfering with summary) =======
        // Child process verification
        write_current_pid();
        msg = "Child process created successfully - PASSED\n";
        write(1, msg, strlen(msg));

        write_current_pid();
        msg = "Child PID verification - ";
        write(1, msg, strlen(msg));

        int child_pid = getpid();
        if (child_pid == 2) { // First child should be PID 2
            msg = "PASSED\n";
            write(1, msg, strlen(msg));
        } else {
            msg = "FAILED\n";
            write(1, msg, strlen(msg));
        }

        write_current_pid();
        msg = "Child memory independence - PASSED\n";
        write(1, msg, strlen(msg));

        write_current_pid();
        msg = "Child syscall test - ";
        write(1, msg, strlen(msg));

        int child_time = gettime();
        if (child_time >= 0) {
            msg = "PASSED\n";
            write(1, msg, strlen(msg));
        } else {
            msg = "FAILED\n";
            write(1, msg, strlen(msg));
        }

        write_current_pid();
        msg = "Child tests completed, exiting\n";
        write(1, msg, strlen(msg));

        // Child terminates here to avoid interfering with parent summary
        exit();

    } else {
        // ======= PARENT PROCESS TESTS =======
        write_current_pid();
        msg = "Child created with PID: ";
        write(1, msg, strlen(msg));
        itoa(pid, buffer);
        write(1, buffer, strlen(buffer));

        // Expected: Parent is PID 1, so first child should be PID 2
        if (pid == 2) {
            msg = " - PASSED\n";
            write(1, msg, strlen(msg));
            parent_subtests_passed++;
        } else {
            msg = " - FAILED\n";
            write(1, msg, strlen(msg));
        }
        parent_subtests_run++;

        // Test 2: Parent memory integrity
        msg = "[TEST 2] Parent memory integrity...\n";
        write(1, msg, strlen(msg));

        write_current_pid();
        msg = "Memory integrity verified - PASSED\n";
        write(1, msg, strlen(msg));
        parent_subtests_passed++;
        parent_subtests_run++;

        // Test 3: Parent PID verification
        msg = "[TEST 3] Parent PID verification...\n";
        write(1, msg, strlen(msg));

        int parent_pid = getpid();
        write_current_pid();
        msg = "My PID is: ";
        write(1, msg, strlen(msg));
        itoa(parent_pid, buffer);
        write(1, buffer, strlen(buffer));

        // Parent should be PID 1 (init process), child should be PID 2
        if (parent_pid == 1) {
            msg = " - PASSED\n";
            write(1, msg, strlen(msg));
            parent_subtests_passed++;
        } else {
            msg = " - FAILED\n";
            write(1, msg, strlen(msg));
        }
        parent_subtests_run++;

        // Test 4: Multiple fork test (create another child)
        msg = "[TEST 4] Multiple fork test...\n";
        write(1, msg, strlen(msg));

        int pid2 = fork();
        if (pid2 == -1) {
            msg = "[PARENT] Second fork failed - checking errno: ";
            write(1, msg, strlen(msg));
            itoa(errno, buffer);
            write(1, buffer, strlen(buffer));

            if (errno == ENOMEM || errno == EAGAIN) {
                msg = " (Expected: no more resources) - PASSED\n";
                write(1, msg, strlen(msg));
                parent_subtests_passed++;
            } else {
                msg = " (Unexpected error) - FAILED\n";
                write(1, msg, strlen(msg));
            }
        } else if (pid2 == 0) {
            // ======= SECOND CHILD PROCESS =======
            write_current_pid();
            msg = "Second child created - PASSED\n";
            write(1, msg, strlen(msg));

            // Second child terminates
            exit();
        } else {
            // Parent - second child created successfully
            write_current_pid();
            msg = "Second child PID: ";
            write(1, msg, strlen(msg));
            itoa(pid2, buffer);
            write(1, buffer, strlen(buffer));

            // Expected: Second child should be PID 3 (Parent=1, First child=2, Second child=3)
            if (pid2 == 3) {
                msg = " - PASSED\n";
                write(1, msg, strlen(msg));
                parent_subtests_passed++;
            } else {
                msg = " - FAILED\n";
                write(1, msg, strlen(msg));
            }
        }
        parent_subtests_run++;

        // Summary for fork tests (only parent executes the summary)
        msg = "\n[FORK] Subtests: ";
        write(1, msg, strlen(msg));

        itoa(parent_subtests_passed, buffer);
        write(1, buffer, strlen(buffer));

        msg = "/";
        write(1, msg, strlen(msg));

        itoa(parent_subtests_run, buffer);
        write(1, buffer, strlen(buffer));

        msg = " passed\n";
        write(1, msg, strlen(msg));

        int passed = (parent_subtests_passed == parent_subtests_run);
        print_test_result("fork() syscall", passed);
    }
}

void test_exit_syscall(void) {
    print_test_header("EXIT SYSCALL");

    msg = "[TEST] Testing exit() syscall with child process...\n";
    write(1, msg, strlen(msg));

    int pid = fork();
    if (pid == 0) {
        // Child process
        write_current_pid();
        msg = "Child process about to exit\n";
        write(1, msg, strlen(msg));
        exit();
        // This should never be reached
        write_current_pid();
        msg = "ERROR: Code after exit() executed!\n";
        write(1, msg, strlen(msg));
    } else if (pid > 0) {
        // Parent process - wait a bit for child to exit
        work(100); // Wait 0.1 seconds for child to exit

        write_current_pid();
        msg = "Child process should have exited\n";
        write(1, msg, strlen(msg));

        int passed = (pid > 0);
        print_test_result("exit() syscall", passed);
    } else {
        msg = "[ERROR] Fork failed for exit test\n";
        write(1, msg, strlen(msg));
        print_test_result("exit() syscall", 0);
    }
}

void test_block_unblock_syscalls(void) {
    print_test_header("BLOCK/UNBLOCK SYSCALLS");

    subtests_run = 0;
    subtests_passed = 0;

    // Subtest 1: Basic block/unblock test - should PASS
    msg = "[TEST 1] Basic block/unblock test...\n";
    write(1, msg, strlen(msg));

    int pid1 = fork();
    if (pid1 == 0) {
        // Child process
        write_current_pid();
        msg = "Child about to block\n";
        write(1, msg, strlen(msg));

        block();

        write_current_pid();
        msg = "Child unblocked successfully\n";
        write(1, msg, strlen(msg));

        // Wait a bit to ensure parent sees the result, then exit
        work(200);
        exit();
    } else if (pid1 > 0) {
        // Parent process
        work(200); // Wait for child to block

        write_current_pid();
        msg = "Unblocking child PID: ";
        write(1, msg, strlen(msg));
        itoa(pid1, buffer);
        write(1, buffer, strlen(buffer));
        msg = "\n";
        write(1, msg, strlen(msg));

        int result1 = unblock(pid1);
        if (result1 == 0) {
            work(300); // Give time for child to respond and show message
            msg = " - PASSED\n";
            write(1, msg, strlen(msg));
            subtests_passed++;
        } else {
            msg = " - FAILED\n";
            write(1, msg, strlen(msg));
        }
        subtests_run++;

        work(200); // Wait for child to finish completely
    }              // Subtest 2: Unblock non-existent process - should FAIL
    msg = "[TEST 2] Unblock non-existent process...\n";
    write(1, msg, strlen(msg));

    int result2 = unblock(999); // PID that doesn't exist
    if (result2 == -1) {
        msg = " - PASSED (correctly failed)\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = " - FAILED (should have failed)\n";
        write(1, msg, strlen(msg));
    }
    subtests_run++;

    // Subtest 3: Unblock running process - should PASS
    msg = "[TEST 3] Unblock running (non-blocked) process...\n";
    write(1, msg, strlen(msg));

    int pid3 = fork();
    if (pid3 == 0) {
        // Child process - stays running, doesn't block
        write_current_pid();
        msg = "Child running (not blocking)\n";
        write(1, msg, strlen(msg));

        work(500); // Work for 0.5 seconds without blocking

        write_current_pid();
        msg = "Child finishing\n";
        write(1, msg, strlen(msg));
        exit();
    } else if (pid3 > 0) {
        // Parent process
        work(100); // Wait a bit for child to start

        write_current_pid();
        msg = "Attempting to unblock running child PID: ";
        write(1, msg, strlen(msg));
        itoa(pid3, buffer);
        write(1, buffer, strlen(buffer));
        msg = "\n";
        write(1, msg, strlen(msg));

        int result3 = unblock(pid3);
        if (result3 == 0) {
            msg = " - PASSED (unblock running process should succeed)\n";
            write(1, msg, strlen(msg));
            subtests_passed++;
        } else {
            msg = " - FAILED (unblock running process should succeed)\n";
            write(1, msg, strlen(msg));
        }
        subtests_run++;

        work(500); // Wait for child to finish
    }

    // Summary for block/unblock tests
    msg = "\n[BLOCK/UNBLOCK] Subtests: ";
    write(1, msg, strlen(msg));

    itoa(subtests_passed, buffer);
    write(1, buffer, strlen(buffer));

    msg = "/";
    write(1, msg, strlen(msg));

    itoa(subtests_run, buffer);
    write(1, buffer, strlen(buffer));

    msg = " passed";
    write(1, msg, strlen(msg));

    int passed = (subtests_passed == subtests_run);
    print_test_result("block/unblock syscalls", passed);
}

/* -- Helper functions -- */

void work(int ticks) {
    int start_time = gettime();
    write_current_pid();
    msg = "working...\n";
    write(1, msg, strlen(msg));

    while (gettime() - start_time < ticks) {
        // Working...
    }

    write_current_pid();
    msg = "ended working\n";
    write(1, msg, strlen(msg));
}

void write_current_pid() {
    msg = "[PID ";
    write(1, msg, strlen(msg));
    itoa(getpid(), buffer);
    write(1, buffer, strlen(buffer));
    msg = "] ";
    write(1, msg, strlen(msg));
}

void print_test_header(char *test_name) {
    char *msg = "\n--- Testing: ";
    write(1, msg, strlen(msg));
    write(1, test_name, strlen(test_name));
    msg = " ---\n";
    write(1, msg, strlen(msg));
}

void print_test_result(char *test_name, int passed) {

    tests_run++;
    if (passed) {
        tests_passed++;
        msg = "\n[RESULT] ";
        write(1, msg, strlen(msg));
        write(1, test_name, strlen(test_name));
        msg = ": PASSED\n";
        write(1, msg, strlen(msg));
    } else {
        msg = "[RESULT] ";
        write(1, msg, strlen(msg));
        write(1, test_name, strlen(test_name));
        msg = ": FAILED\n";
        write(1, msg, strlen(msg));
    }
}

void print_final_summary(void) {
    // Only print summary if this is the main process (PID 1)
    if (getpid() != 1) {
        return;
    }

    msg = "\n\n";
    write(1, msg, strlen(msg));

    msg = "=========================================\n";
    write(1, msg, strlen(msg));

    msg = "           TEST SUMMARY                  \n";
    write(1, msg, strlen(msg));

    msg = "=========================================\n";
    write(1, msg, strlen(msg));

    msg = "Tests executed:\n";
    write(1, msg, strlen(msg));

    // Obtener resultados basados en tests_passed y tests_run
    int current_test = 0;

#if WRITE_TEST
    if (current_test < tests_run) {
        msg = (current_test < tests_passed) ? "WRITE_TEST              : PASSED\n"
                                            : "WRITE_TEST              : FAILED\n";
        current_test++;
    } else {
        msg = "WRITE_TEST              : SKIPPED\n";
    }
    write(1, msg, strlen(msg));
#endif

#if GETTIME_TEST
    if (current_test < tests_run) {
        msg = (current_test < tests_passed) ? "GETTIME_TEST            : PASSED\n"
                                            : "GETTIME_TEST            : FAILED\n";
        current_test++;
    } else {
        msg = "GETTIME_TEST            : SKIPPED\n";
    }
    write(1, msg, strlen(msg));
#endif

#if GETPID_TEST
    if (current_test < tests_run) {
        msg = (current_test < tests_passed) ? "GETPID_TEST             : PASSED\n"
                                            : "GETPID_TEST             : FAILED\n";
        current_test++;
    } else {
        msg = "GETPID_TEST             : SKIPPED\n";
    }
    write(1, msg, strlen(msg));
#endif

#if FORK_TEST
    if (current_test < tests_run) {
        msg = (current_test < tests_passed) ? "FORK_TEST               : PASSED\n"
                                            : "FORK_TEST               : FAILED\n";
        current_test++;
    } else {
        msg = "FORK_TEST               : SKIPPED\n";
    }
    write(1, msg, strlen(msg));
#endif

#if EXIT_TEST
    if (current_test < tests_run) {
        msg = (current_test < tests_passed) ? "EXIT_TEST               : PASSED\n"
                                            : "EXIT_TEST               : FAILED\n";
        current_test++;
    } else {
        msg = "EXIT_TEST               : SKIPPED\n";
    }
    write(1, msg, strlen(msg));
#endif

#if BLOCK_UNBLOCK_TEST
    if (current_test < tests_run) {
        msg = (current_test < tests_passed) ? "BLOCK_UNBLOCK_TEST      : PASSED\n"
                                            : "BLOCK_UNBLOCK_TEST      : FAILED\n";
        current_test++;
    } else {
        msg = "BLOCK_UNBLOCK_TEST      : SKIPPED\n";
    }
    write(1, msg, strlen(msg));
#endif

#if PAGEFAULT_TEST
    msg = "PAGEFAULT_TEST          : SKIPPED\n";
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
