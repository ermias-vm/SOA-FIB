#include <libc.h>
char buff[24];

int pid;

extern int addAsm(int par1, int par2);

int add(int par1, int par2) {
    return par1 + par2;
}

void test_write() {
    write(1, "\n\nTesting write() syscall...\n", 29);
    write(1, "Write working correctly\n", 24);
    write(1, "write() test: PASSED", 20);
}

void test_gettime() {
    write(1, "\n\nTesting gettime() syscall...\n", 31);
    itoa(gettime(), buff);
    write(1, "Ticks: ", 7);
    write(1, buff, strlen(buff));
    write(1, "\ngettime() test: PASSED", 23);
}

void test_pagefault() {
    write(1, "\n\nTesting Page Fault Exception...\n", 34);
    char *p = 0;
    *p = 'x';
    write(1, "Page Fault Exception test: FAILED\n", 36);
}

__attribute__((__section__(".text.main"))) int main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a
     * privileged one, and so it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    volatile int sum = add(2, 3);
    volatile int sumAsm = addAsm(999, 110);

    buff[0] = sum;
    buff[1] = sumAsm;

    test_write();
    test_gettime();
    test_pagefault();

    while (1) {
        int ticks = gettime();
        itoa(ticks, buff);
        write(1, "Ticks: ", 7);
        write(1, buff, strlen(buff));
        write(1, "\n", 1);
        for (int i = 0; i < 100000000; i++);
    }
}
