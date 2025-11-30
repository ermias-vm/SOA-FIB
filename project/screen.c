/**
 * @file screen.c
 * @brief Screen frame buffer implementation for ZeOS.
 *
 * This file implements direct screen buffer writing for the write(10, ...)
 * system call. Multiple copy methods are available, selectable at compile time.
 */

#include <screen.h>
#include <types.h>
#include <utils.h>

#if SCREEN_COPY_METHOD == SCREEN_METHOD_WORD_LOOP /* Method A: Word-by-word C loop */
int sys_write_screen(char *buffer, int size) {
    /* Limit to screen buffer size */
    if (size > SCREEN_BUFFER_SIZE) size = SCREEN_BUFFER_SIZE;

    Word *screen = (Word *)VIDEO_MEMORY_BASE;
    int words = size / 2; /* 4000 / 2 = 2000 words */

    for (int i = 0; i < words; i++) {
        /* Combine two bytes into one Word (16 bits) */
        screen[i] = ((Word)buffer[i * 2]) | ((Word)buffer[i * 2 + 1] << 8);
    }

    return size;
}
#elif SCREEN_COPY_METHOD == SCREEN_METHOD_COPY_FROM_USER /* Method B: Using copy_from_user */
int sys_write_screen(char *buffer, int size) {
    /* Limit to screen buffer size */
    if (size > SCREEN_BUFFER_SIZE) size = SCREEN_BUFFER_SIZE;

    /* Copy from user space directly to video memory */
    copy_from_user(buffer, (char *)VIDEO_MEMORY_BASE, size);

    return size;
}
#elif SCREEN_COPY_METHOD == SCREEN_METHOD_REP_MOVSL      /* Method C: Using REP MOVSL (Assembly) */
int sys_write_screen(char *buffer, int size) {
    /* Limit to screen buffer size */
    if (size > SCREEN_BUFFER_SIZE) size = SCREEN_BUFFER_SIZE;

    int dwords = size / 4;
    int remaining = size % 4;

    /* Use REP MOVSL for bulk copy (4 bytes at a time) */
    unsigned int src = (unsigned int)buffer;
    unsigned int dst = (unsigned int)VIDEO_MEMORY_BASE;
    __asm__ __volatile__("cld\n\t"   /* Clear direction flag (forward) */
                         "rep movsl" /* Copy ECX dwords from DS:ESI to ES:EDI */
                         : "+S"(src), "+D"(dst)
                         : "c"(dwords)
                         : "memory");

    /* Copy remaining bytes (0-3) if size not divisible by 4 */
    if (remaining > 0) {
        char *dest = (char *)VIDEO_MEMORY_BASE + (dwords * 4);
        char *src_rem = buffer + (dwords * 4);
        for (int i = 0; i < remaining; i++) {
            dest[i] = src_rem[i];
        }
    }

    return size;
}

#else
#error "Invalid SCREEN_COPY_METHOD defined"
#endif
