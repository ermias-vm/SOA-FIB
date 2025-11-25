/**
 * @file libc.c
 * @brief Standard C library functions for ZeOS.
 *
 * This file provides basic C library functions including string
 * manipulation, memory operations, and utility functions for
 * both kernel and user space code.
 */

#include <errno.h>
#include <libc.h>
#include <types.h>

int errno;

void itoa(int a, char *b) {
    int i, i1;
    char c;

    if (a == 0) {
        b[0] = '0';
        b[1] = 0;
        return;
    }

    i = 0;
    while (a > 0) {
        b[i] = (a % 10) + '0';
        a = a / 10;
        i++;
    }

    for (i1 = 0; i1 < i / 2; i1++) {
        c = b[i1];
        b[i1] = b[i - i1 - 1];
        b[i - i1 - 1] = c;
    }
    b[i] = 0;
}

int strlen(char *a) {
    int i;

    i = 0;

    while (a[i] != 0) i++;

    return i;
}

void perror() {
    char buff[12];
    char *msg;

    switch (errno) {
    case ENOSYS:
        msg = "Syscall not implemented\n";
        break;
    case EFAULT:
        msg = "Bad address\n";
        break;
    case EINVAL:
        msg = "Invalid argument\n";
        break;
    case EACCES:
        msg = "Permission denied\n";
        break;
    case EBADF:
        msg = "Bad file number\n";
        break;
    default:
        itoa(errno, buff);
        msg = "Message for error ";
        write(1, msg, strlen(msg));
        write(1, buff, strlen(buff));
        msg = " not found\n";
        write(1, msg, strlen(msg));
        return;
    }

    write(1, msg, strlen(msg));

    return;
}

/* Keyboard wrapper function */
void __kbd_wrapper(void (*func)(char, int), char key, int pressed) {
    func(key, pressed);
    __asm__ __volatile__("int $0x2b" ::: "memory");
}

/* Assembly wrapper for kernel to call */
__asm__(
    ".global __kbd_wrapper_asm\n"
    "__kbd_wrapper_asm:\n"
    "    pushl %ebp\n"
    "    movl %esp, %ebp\n"
    "    pushl 16(%ebp)\n"      
    "    pushl 12(%ebp)\n"      
    "    calll *8(%ebp)\n"      
    "    addl $8, %esp\n"       
    "    int $0x2b\n"           
    "    popl %ebp\n"
    "    ret\n"
);

/* Declaration - implementation is in sys_call_wrappers.S */
extern int KeyboardEvent(void (*func)(char key, int pressed));
