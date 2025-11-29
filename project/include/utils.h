/**
 * @file utils.h
 * @brief Utility function interfaces and helper definitions for ZeOS.
 *
 * This header defines utility functions, memory operations,
 * and general-purpose helper routines for the ZeOS kernel.
 */

#ifndef UTILS_H
#define UTILS_H

/** Number of cycles per tick */
#define CYCLESPERTICK 109000

/** Read access verification type */
#define VERIFY_READ 0

/** Write access verification type */
#define VERIFY_WRITE 1

/**
 * @brief Minimum value macro.
 *
 * This macro returns the minimum of two values.
 * @param a First value.
 * @param b Second value.
 */
#define min(a, b) (a < b ? a : b)

/**
 * @brief 64-bit division macro.
 *
 * This macro performs a 64-bit division and stores the result in the low and high registers.
 * @param n Dividend.
 * @param base Divisor.
 */
#define do_div(n, base)                                                                            \
    ({                                                                                             \
        unsigned long __upper, __low, __high, __mod, __base;                                       \
        __base = (base);                                                                           \
        asm("" : "=a"(__low), "=d"(__high) : "A"(n));                                              \
        __upper = __high;                                                                          \
        if (__high) {                                                                              \
            __upper = __high % (__base);                                                           \
            __high = __high / (__base);                                                            \
        }                                                                                          \
        asm("divl %2" : "=a"(__low), "=d"(__mod) : "rm"(__base), "0"(__low), "1"(__upper));        \
        asm("" : "=A"(n) : "a"(__low), "d"(__high));                                               \
        __mod;                                                                                     \
    })

/**
 * @brief Read Time Stamp Counter.
 *
 * This macro reads the Time Stamp Counter register and stores the result in the low and high
 * registers.
 * @param low Low register.
 * @param high High register.
 */
#define rdtsc(low, high) __asm__ __volatile__("rdtsc" : "=a"(low), "=d"(high))

/**
 * @brief Copy data between memory locations.
 *
 * This function copies data from one memory location to another.
 *
 * @param start Source memory address.
 * @param dest Destination memory address.
 * @param size Number of bytes to copy.
 */
extern void copy_data(void *start, void *dest, int size);

/**
 * @brief Copy data from user space to kernel space.
 *
 * This function safely copies data from user space to kernel space
 * with proper access checking.
 *
 * @param start Source address in user space.
 * @param dest Destination address in kernel space.
 * @param size Number of bytes to copy.
 * @return Number of bytes copied, or negative error code.
 */
extern int copy_from_user(void *start, void *dest, int size);

/**
 * @brief Copy data from kernel space to user space.
 *
 * This function safely copies data from kernel space to user space
 * with proper access checking.
 *
 * @param start Source address in kernel space.
 * @param dest Destination address in user space.
 * @param size Number of bytes to copy.
 * @return Number of bytes copied, or negative error code.
 */
extern int copy_to_user(void *start, void *dest, int size);

/**
 * @brief Check if memory access is allowed.
 *
 * This function verifies if the specified memory range is accessible
 * for the given operation type (read or write).
 *
 * @param type Type of access (VERIFY_READ or VERIFY_WRITE).
 * @param addr Starting address to check.
 * @param size Size of the memory region to check.
 * @return 1 if access is allowed, 0 otherwise.
 */
extern int access_ok(int type, const void *addr, unsigned long size);

/**
 * @brief Get current system ticks.
 *
 * This function returns the current system timer ticks.
 *
 * @return Current system ticks.
 */
extern unsigned long get_ticks(void);

/**
 * @brief Wait for specified number of ticks.
 *
 * This function implements a busy-wait delay for the specified number
 * of timer ticks.
 *
 * @param ticks_to_wait Number of ticks to wait.
 */
extern void wait_ticks(int ticks_to_wait);

/**
 * @brief Convert integer to hexadecimal string.
 *
 * This function converts an unsigned integer to its hexadecimal
 * string representation.
 *
 * @param num Unsigned integer to convert.
 * @param buffer Buffer to store the hexadecimal string.
 */
extern void itoa_hex(unsigned int num, char *buffer);

/**
 * @brief Convert unsigned integer to ASCII string.
 *
 * This function converts an unsigned integer to its decimal string representation.
 * Handles the full range of unsigned int including values > INT_MAX.
 *
 * @param a Unsigned integer to convert.
 * @param b Buffer to store the decimal string.
 */
extern void utoa(unsigned int a, char *b);

/**
 * @brief Print system splash screen.
 *
 * This function prints the ZeOS startup splash screen with system information.
 */
extern void print_splash_screen(void);

#endif /* UTILS_H */
