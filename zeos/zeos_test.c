#include <errno.h>
#include <libc.h>

#define BUFFER_SIZE 256
#define LARGE_BUFFER_SIZE 300
// clang-format off
#define WRITE_TEST              1
#define GETTIME_TEST            1
#define PAGEFAULT_TEST          0
// clang-format on

char buffer[BUFFER_SIZE];
char large_buffer[LARGE_BUFFER_SIZE]; // For testing large writes

static int tests_run = 0;
static int tests_passed = 0;
static int subtests_run = 0;
static int subtests_passed = 0;
extern int errno;

void print_test_header(char *test_name);
void print_test_result(char *test_name, int passed);
void print_final_summary(void);

void test_write_syscall(void);
void test_gettime_syscall(void);
void test_pagefault_exception(void);

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
#endif

#if GETTIME_TEST == 1
    test_gettime_syscall();
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

    msg = " passed\n";
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

    msg = "\n";
    write(1, msg, strlen(msg));

    // Test result - ticks should be non-negative and second >= first
    int passed = (ticks1 >= 0 && ticks2 >= 0 && ticks2 >= ticks1);
    print_test_result("gettime() syscall", passed);
}

void test_pagefault_exception(void) {
    print_test_header("PAGE FAULT EXCEPTION");
    char *msg;

    msg = "[TEST] Testing Page Fault Exception...\n";
    write(1, msg, strlen(msg));

    msg = "[TEST] WARNING: This will cause a page fault!\n";
    write(1, msg, strlen(msg));

    msg = "[TEST] About to access NULL pointer...\n";
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
        msg = "[RESULT] ";
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

    msg = "\n";
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