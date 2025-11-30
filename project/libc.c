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

/* Global errno variable for error handling */
int errno;

/* Internal buffer for printf formatting */
static char printf_buffer[PRINTF_BUFFER_SIZE];

/****************************************/
/**    String Functions                **/
/****************************************/

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

int strlen(const char *a) {
    int i;

    i = 0;

    while (a[i] != 0) i++;

    return i;
}

/****************************************/
/**    Error Handling Functions        **/
/****************************************/

void perror(void) {
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

/****************************************/
/**    I/O Functions                   **/
/****************************************/

void prints(const char *fmt, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, fmt);

    char *buf = printf_buffer;
    int buf_idx = 0;
    char num_buf[16];

    while (*fmt && buf_idx < PRINTF_BUFFER_SIZE - 1) {
        if (*fmt == '%' && *(fmt + 1)) {
            fmt++;
            switch (*fmt) {
            case 'd': {
                int val = __builtin_va_arg(args, int);
                int is_neg = 0;
                unsigned int uval;
                if (val < 0) {
                    is_neg = 1;
                    uval = -(unsigned int)val;
                } else {
                    uval = (unsigned int)val;
                }
                /* Convert to string */
                int i = 0;
                if (uval == 0) {
                    num_buf[i++] = '0';
                } else {
                    while (uval > 0 && i < 15) {
                        num_buf[i++] = (uval % 10) + '0';
                        uval /= 10;
                    }
                }
                if (is_neg && buf_idx < PRINTF_BUFFER_SIZE - 1) {
                    buf[buf_idx++] = '-';
                }
                /* Reverse and copy */
                while (i > 0 && buf_idx < PRINTF_BUFFER_SIZE - 1) {
                    buf[buf_idx++] = num_buf[--i];
                }
                break;
            }
            case 'u': {
                unsigned int uval = __builtin_va_arg(args, unsigned int);
                /* Convert to string */
                int i = 0;
                if (uval == 0) {
                    num_buf[i++] = '0';
                } else {
                    while (uval > 0 && i < 15) {
                        num_buf[i++] = (uval % 10) + '0';
                        uval /= 10;
                    }
                }
                /* Reverse and copy */
                while (i > 0 && buf_idx < PRINTF_BUFFER_SIZE - 1) {
                    buf[buf_idx++] = num_buf[--i];
                }
                break;
            }
            case 's': {
                char *str = __builtin_va_arg(args, char *);
                while (*str && buf_idx < PRINTF_BUFFER_SIZE - 1) {
                    buf[buf_idx++] = *str++;
                }
                break;
            }
            case 'c': {
                char c = (char)__builtin_va_arg(args, int);
                buf[buf_idx++] = c;
                break;
            }
            case '%':
                buf[buf_idx++] = '%';
                break;
            default:
                buf[buf_idx++] = '%';
                if (buf_idx < PRINTF_BUFFER_SIZE - 1) {
                    buf[buf_idx++] = *fmt;
                }
                break;
            }
            fmt++;
        } else {
            buf[buf_idx++] = *fmt++;
        }
    }

    __builtin_va_end(args);

    if (buf_idx > 0) {
        write(1, buf, buf_idx);
    }
}

int clear_screen_buffer(int fd) {
    char clear_buffer[80 * 25 * 2];

    /* Fill buffer with spaces and default color */
    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        clear_buffer[i] = ' ';      /* Space character */
        clear_buffer[i + 1] = 0x07; /* Light gray on black */
    }

    return write(fd, clear_buffer, sizeof(clear_buffer));
}
