/**
 * @file user.c
 * @brief User space initialization and test program for ZeOS.
 *
 * This file contains the initial user process code that runs
 * system tests and demonstrates ZeOS functionality.
 */

#include <libc.h>
#include <zeos_test.h>
char buff[256];

__attribute__((__section__(".text.main"))) int main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a
     * privileged one, and so it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    /* Execute basic process operations test (fork, block, unblock, exit) */
    // test_basic_process_operations();

    /* Execute ZeOS test suite, configure test to execute in zeos_test.h*/
    execute_zeos_tests();

    write_current_pid();
    char *msg = "After execute_zeos_tests(), only task 1 should be running...\n";
    write(1, msg, strlen(msg));

    while (1) {
        // Infinite loop to keep system running
    }
}
