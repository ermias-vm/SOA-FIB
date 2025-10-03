/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>
#include <io.h>
#include <mm.h>
#include <mm_address.h>
#include <sched.h>
#include <utils.h>
#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1
#define BUFFER_SIZE 256

char buffer_k[BUFFER_SIZE];

int check_fd(int fd, int permissions) {
    if (fd != 1) return -9;                    /*EBADF*/
    if (permissions != ESCRIPTURA) return -13; /*EACCES*/
    return 0;
}

int sys_ni_syscall() {
    return -38; /*ENOSYS*/
}

int sys_getpid() {
    return current()->PID;
}

int sys_fork() {
    int PID = -1;

    // creates the child process

    return PID;
}

int sys_write(int fd, char *buffer, int size) {

    int fd_error = check_fd(fd, ESCRIPTURA);
    if (fd_error) return fd_error;
    if (buffer == NULL) return -EFAULT;
    if (size < 0) return -EINVAL;

    int bytes_restantes = size;
    int written_bytes;

    while (bytes_restantes > BUFFER_SIZE) {
        copy_from_user(buffer, buffer_k, BUFFER_SIZE);
        written_bytes = sys_write_console(buffer_k, BUFFER_SIZE);
        bytes_restantes -= written_bytes;
        buffer += written_bytes;
    }

    if (bytes_restantes > 0) {
        copy_from_user(buffer, buffer_k, bytes_restantes);
        written_bytes = sys_write_console(buffer_k, bytes_restantes);
        bytes_restantes -= written_bytes;
    }

    return size - bytes_restantes;
}

void sys_exit() {
}
