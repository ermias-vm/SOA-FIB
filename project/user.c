/**
 * @file user.c
 * @brief User space initialization and test program for ZeOS.
 *
 * This file contains the initial user process code that runs
 * system tests and demonstrates ZeOS functionality.
 */

#include <libc.h>
#include <project_test.h>
#include <zeos_test.h>

__attribute__((__section__(".text.main"))) int main(void) {

#if RUN_TESTS
    execute_project_tests();
#endif
    write_current_pid();
    char *msg = "After tests, only task 1 should be running...\n";
    write(1, msg, strlen(msg));

    while (1) {
        // Infinite loop to keep system running
    }
}
