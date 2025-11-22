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

char buff[256];

__attribute__((__section__(".text.main"))) int main(void) {
    /* Execute ZeOS test suite, configure test to execute in zeos_test.h*/
    // execute_zeos_tests();

    /* Execute project thread tests */
    execute_project_tests();

    write_current_pid();
    char *msg = "After tests, only task 1 should be running...\n";
    write(1, msg, strlen(msg));

    while (1) {
        // Infinite loop to keep system running
    }
}
