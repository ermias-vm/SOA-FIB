#include <libc.h>

char buff[24];

int pid;

int __attribute__((__section__(".text.main"))) main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so
     * it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    char *msg = "\n\nBAKA BAKA is back\n";
    int bytesTowrite = strlen(msg);
    int ret = write(1, msg, bytesTowrite);

    write(1, "bytes to write: ", 16);
    itoa(bytesTowrite, buff);
    write(1, buff, strlen(buff));

    write(1, "\nbytes written: ", 16);
    itoa(ret, buff);
    write(1, buff, strlen(buff));

    while (1) {
    }
}
