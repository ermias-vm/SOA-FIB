#ifndef UTILS_H
#define UTILS_H

void copy_data(void *start, void *dest, int size);
int copy_from_user(void *start, void *dest, int size);
int copy_to_user(void *start, void *dest, int size);

#define VERIFY_READ 0
#define VERIFY_WRITE 1
int access_ok(int type, const void *addr, unsigned long size);

#define min(a, b) (a < b ? a : b)

unsigned long get_ticks(void);

void itoa_hex(unsigned int num, char *buffer);

void print_splash_screen(void);
void wait_ticks(int ticks_to_wait);
#endif // UTILS_H
