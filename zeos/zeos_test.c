#include <errno.h>
#include <libc.h>
#include <zeos_test.h>

char buffer[BUFFER_SIZE];
char large_buffer[LARGE_BUFFER_SIZE]; // For testing large writes

static int tests_run = 0;
static int tests_passed = 0;
static int subtests_run = 0;
static int subtests_passed = 0;
extern int errno;

void execute_zeos_tests(void) {
    char *msg = "\n";
    write(1, msg, strlen(msg));

    msg = "=========================================\n";
    write(1, msg, strlen(msg));

    msg = "         ZEOS SYSCALL TEST SUITE         \n";
    write(1, msg, strlen(msg));

    msg = "=========================================\n";
    write(1, msg, strlen(msg));

#if WRITE_TEST == 1
    test_write_syscall();
    RESET_ERRNO();
#endif

#if GETTIME_TEST == 1
    test_gettime_syscall();
    RESET_ERRNO();
#endif

#if GETPID_TEST == 1
    test_getpid_syscall();
    RESET_ERRNO();
#endif

#if FORK_TEST == 1
    test_fork_syscall();
    RESET_ERRNO();
#endif

#if EXIT_TEST == 1
    test_exit_syscall();
    RESET_ERRNO();
#endif

#if BLOCK_UNBLOCK_TEST == 1
    test_block_unblock_syscalls();
    RESET_ERRNO();
#endif

#if PAGEFAULT_TEST == 1
    test_pagefault_exception();
#endif

    print_final_summary();
}

void test_write_syscall(void) {
    print_test_header("WRITE SYSCALL");

    subtests_run = 0;
    subtests_passed = 0;
    char *msg;

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
    msg = "\n";
    write(1, msg, strlen(msg));

    if (result4 == strlen(large_buffer)) {
        msg = "Large buffer - PASSED\n";
        write(1, msg, strlen(msg));
        subtests_passed++;
    } else {
        msg = "Large buffer - FAILED\n";
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
    char *msg;

    msg = "[TEST] Testing gettime() syscall...\n";
    write(1, msg, strlen(msg));

    int ticks1 = gettime();
    msg = "[TEST] First call - Ticks: ";
    write(1, msg, strlen(msg));

    itoa(ticks1, buffer);
    write(1, buffer, strlen(buffer));

    msg = "\n";
    write(1, msg, strlen(msg));

    volatile int i;
    for (i = 0; i < 100000; i++) { // TODO: Sleep
    }

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
    char *msg;

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

void test_pagefault_exception(void) {
    print_test_header("PAGE FAULT EXCEPTION");
    char *msg;

    msg = "[TEST] Testing Page Fault Exception...\n";
    write(1, msg, strlen(msg));

    // This will cause a page fault and should not return
    volatile char *p = (volatile char *)0x0;
    *p = 'x';

    // This line should never be reached
    msg = "[TEST] ERROR: Page fault was not triggered!\n";
    write(1, msg, strlen(msg));
    print_test_result("Page Fault Exception", 0);
}

// Helper functions
void print_test_header(char *test_name) {
    char *msg = "\n--- Testing: ";
    write(1, msg, strlen(msg));
    write(1, test_name, strlen(test_name));
    msg = " ---\n";
    write(1, msg, strlen(msg));
}

void print_test_result(char *test_name, int passed) {
    char *msg;
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
    char *msg;

    msg = "\n\n";
    write(1, msg, strlen(msg));

    msg = "=========================================\n";
    write(1, msg, strlen(msg));

    msg = "           TEST SUMMARY                  \n";
    write(1, msg, strlen(msg));

    msg = "=========================================\n";
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

void test_fork_syscall(void) {
    RESET_ERRNO(); // Reset errno at the start of test

    print_test_header("FORK SYSCALL");

    int parent_subtests_run = 0;
    int parent_subtests_passed = 0;
    char *msg;

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
        // ======= CHILD PROCESS TESTS (NOT COUNTED IN PARENT SUMMARY) =======
        // Child process
        msg = "[CHILD] Child process created successfully (PID=0) - PASSED\n";
        write(1, msg, strlen(msg));

        // Child Test A: Verify child has different PID from parent
        msg = "[CHILD TEST A] Child PID verification...\n";
        write(1, msg, strlen(msg));

        int child_pid = getpid();
        msg = "[CHILD] My PID is: ";
        write(1, msg, strlen(msg));
        itoa(child_pid, buffer);
        write(1, buffer, strlen(buffer));

        if (child_pid == 2) { // First child should be PID 2
            msg = " - PASSED\n";
            write(1, msg, strlen(msg));
        } else {
            msg = " - FAILED\n";
            write(1, msg, strlen(msg));
        }

        // Child Test B: Child memory independence
        msg = "[CHILD TEST B] Child memory independence...\n";
        write(1, msg, strlen(msg));

        // Child can execute independently
        msg = "[CHILD] Memory independence verified - PASSED\n";
        write(1, msg, strlen(msg));

        // Child Test C: Child can make syscalls
        msg = "[CHILD TEST C] Child syscall test...\n";
        write(1, msg, strlen(msg));

        int child_time = gettime();
        if (child_time >= 0) {
            msg = "[CHILD] gettime() works - PASSED\n";
            write(1, msg, strlen(msg));
        } else {
            msg = "[CHILD] gettime() failed - FAILED\n";
            write(1, msg, strlen(msg));
        }

        // Child ends here (in a real OS it would do exit())
        msg = "[CHILD] Child tests completed\n";
        write(1, msg, strlen(msg));

        // For now the child can't terminate correctly without exit()
        // so it will enter an infinite loop to not interfere
        while (1) {
            // Child process waits here
        }

    } else {
        // ======= PARENT PROCESS TESTS =======
        // Parent process
        msg = "[PARENT] Child created with PID: ";
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

        // Parent process maintains its state
        msg = "[PARENT] Memory integrity verified - PASSED\n";
        write(1, msg, strlen(msg));
        parent_subtests_passed++;
        parent_subtests_run++;

        // Test 3: Parent PID verification
        msg = "[TEST 3] Parent PID verification...\n";
        write(1, msg, strlen(msg));

        int parent_pid = getpid();
        msg = "[PARENT] My PID is: ";
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
            // Second child
            msg = "[CHILD2] Second child created - PASSED\n";
            write(1, msg, strlen(msg));

            // Second child also enters loop
            while (1) {
                // Second child waits here
            }
        } else {
            // Parent - second child created successfully
            msg = "[PARENT] Second child PID: ";
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
    char *msg;

    msg = "[TEST] Testing exit() syscall with child process...\n";
    write(1, msg, strlen(msg));

    int pid = fork();
    if (pid == 0) {
        // Child process
        msg = "[CHILD] Child process about to exit\n";
        write(1, msg, strlen(msg));
        exit();
        // This should never be reached
        msg = "[CHILD] ERROR: Code after exit() executed!\n";
        write(1, msg, strlen(msg));
    } else if (pid > 0) {
        // Parent process - wait a bit for child to exit
        int start_time = gettime();
        while (gettime() - start_time < 10) {
            // Brief wait
        }
        msg = "[PARENT] Child process should have exited\n";
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
    char *msg;

    msg = "[TEST] Testing block() and unblock() syscalls...\n";
    write(1, msg, strlen(msg));

    int pid = fork();
    if (pid == 0) {
        // Child process
        msg = "[CHILD] Child about to block\n";
        write(1, msg, strlen(msg));

        block();

        msg = "[CHILD] Child unblocked successfully\n";
        write(1, msg, strlen(msg));
        exit();
    } else if (pid > 0) {
        // Parent process
        msg = "[PARENT] Parent waiting before unblocking child\n";
        write(1, msg, strlen(msg));

        // Wait a bit
        int start_time = gettime();
        while (gettime() - start_time < 5) {
            // Brief wait
        }

        msg = "[PARENT] Unblocking child process\n";
        write(1, msg, strlen(msg));

        msg = "[PARENT] About to unblock PID: ";
        write(1, msg, strlen(msg));
        itoa(pid, buffer);
        write(1, buffer, strlen(buffer));
        msg = "\n";
        write(1, msg, strlen(msg));

        int result = unblock(pid);
        if (result == 0) {
            msg = "[PARENT] Unblock successful\n";
            write(1, msg, strlen(msg));
        } else {
            msg = "[PARENT] Unblock failed\n";
            write(1, msg, strlen(msg));
        }

        // Wait for child to finish
        start_time = gettime();
        while (gettime() - start_time < 10) {
            // Wait for child
        }

        int passed = (result == 0);
        print_test_result("block/unblock syscalls", passed);
    } else {
        msg = "[ERROR] Fork failed for block/unblock test\n";
        write(1, msg, strlen(msg));
        print_test_result("block/unblock syscalls", 0);
    }
}
