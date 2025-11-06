#include <libc.h>
#include <zeos_test.h>
char buff[256];

int pid;

extern int addAsm(int par1, int par2);

int add(int par1, int par2) {
    return par1 + par2;
}

__attribute__((__section__(".text.main"))) int main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a
     * privileged one, and so it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    // Execute test suite
    // execute_zeos_tests();
    pid = fork();

    if (pid == 0) {
        // Child process
        write(1, "First Child process created with fork(), my pid: ", 50);
        itoa(getpid(), buff);
        write(1, buff, strlen(buff));
        write(1, "\n", 1);
    } else if (pid > 0) {
        // Parent process
        exit(); // Parent is task1, cannot exit
        write(1, "Parent process created a child with fork()\n", 43);
        itoa(pid, buff);
        write(1, "Child PID: ", 11);
        write(1, buff, strlen(buff));
        write(1, "\n", 1);

        int pid2= fork();

        if (pid2 > 0) {
            // First child process
            write(1, "Parent process created another child with fork()\n", 48);
        } else if (pid2 == 0) {
            // Second child process
            write(1, "Second child process created with fork()\n", 42);
            write(1, "Second child exiting...\n", 24);
            exit();
        }

    } else {
        // Fork failed
        write(1, "Fork failed\n", 12);
    }

    while (1) {
        for (volatile int i = 0; i < 10000000; i++);
        int myPid = getpid();
        write(1, "\nProcess PID: ", 14);
        itoa(myPid, buff);
        write(1, buff, strlen(buff));
        write(1, " - ", 3); 
        write(1, "Working...\n", 11);
    }
}
