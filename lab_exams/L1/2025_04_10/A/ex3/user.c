#include <libc.h>

char buff[24];

int pid;
// Declarar un buffer grande alineado a página para asegurar alineamiento
char big_buff[4096 * 2] __attribute__((aligned(0x1000))) = {0}; // 8192 bytes, alineado a 0x1000

int __attribute__((__section__(".text.main"))) main(void) {
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so
     * it will raise an exception */
    /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

    // Apuntar directamente a 16 bytes antes del final de la primera página de big_buff
    char *test_buff = big_buff + (0x1000 - 16);

    char *test_string = "0123456789ABCDEF0123456789ABCDEF";

    for (int i = 0; i < 5000; i++) {
        test_buff[i] = test_string[i % 32];
    }

    // Mensaje de inicio
    write(1, "Starting cross-page test\n", 23);

    // Escribir 32 bytes desde test_buff
    int ret = write(1, test_buff, 5000);

    // Mostrar bytes escritos
    write(1, "\nBytes written: ", 15);
    itoa(ret, buff);
    write(1, buff, strlen(buff));
    write(1, "\nTest completed.\n", 17);

    while (1) {
    }
}
