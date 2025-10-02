#include <libc.h>
char buff[24];

int pid;

extern int addAsm(int par1, int par2);

int add(int par1, int par2) {
    return par1 + par2;
}

__attribute__ ((__section__(".text.main"))) 
int main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    volatile int sum = add(2, 3);
    volatile int sumAsm = addAsm(999, 110);
    
    buff[0] = sum;
    buff[1] = sumAsm;
    while(1) { 

    }
}
