#include <libc.h>

char buff[24];

int pid;
char msg_aligned[] __attribute__((aligned(0x1000))) = "\nBAKA BAKA is aligned\n";
char msg_aligned2[] __attribute__((aligned(0x1000))) = "\nBARKUS BARKUS is aligned\n";
int __attribute__((__section__(".text.main"))) main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so
     * it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    int bytesTowriteAligned = strlen(msg_aligned);
    int ret_aligned = write(1, msg_aligned, bytesTowriteAligned);

    int bytesTowriteAligned2 = strlen(msg_aligned2);
    int ret_aligned2 = write(1, msg_aligned2, bytesTowriteAligned2);
    //
    // char *msg = "\n\nBAKA BAKA is not aligned\n";
    // int bytesTowrite = strlen(msg);
    // int ret = write(1, msg, bytesTowrite);

    // write(1, "bytes to write: ", 16);
    // itoa(bytesTowrite, buff);
    // write(1, buff, strlen(buff));
    //
    // write(1, "\nbytes written: ", 16);
    // itoa(ret, buff);
    // write(1, buff, strlen(buff));

    while (1) {
    }
}
