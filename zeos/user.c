#include <libc.h>
char buff[24];

int pid;

extern int addAsm(int par1, int par2);

int add(int par1, int par2) {
    return par1 + par2;
}

test_page_fault() {
    volatile char *p = (volatile char *)0;
    *p = 'x';
}

__attribute__((__section__(".text.main"))) int main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a
     * privileged one, and so it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    write(1, "HOLA", 4);
    write(1, "HOLA", 4);

    while (1) {
    }
}
