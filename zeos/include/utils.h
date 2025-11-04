#ifndef UTILS_H
#define UTILS_H

/**
 * @brief Copy data between memory locations.
 *
 * This function copies data from one memory location to another.
 * @param start Source memory address.
 * @param dest Destination memory address.
 * @param size Number of bytes to copy.
 */
void copy_data(void *start, void *dest, int size);

/**
 * @brief Copy data from user space to kernel space.
 *
 * This function safely copies data from user space to kernel space
 * with proper access checking.
 * @param start Source address in user space.
 * @param dest Destination address in kernel space.
 * @param size Number of bytes to copy.
 * @return Number of bytes copied, or negative error code.
 */
int copy_from_user(void *start, void *dest, int size);

/**
 * @brief Copy data from kernel space to user space.
 *
 * This function safely copies data from kernel space to user space
 * with proper access checking.
 * @param start Source address in kernel space.
 * @param dest Destination address in user space.
 * @param size Number of bytes to copy.
 * @return Number of bytes copied, or negative error code.
 */
int copy_to_user(void *start, void *dest, int size);

#define VERIFY_READ 0
#define VERIFY_WRITE 1
/**
 * @brief Check if memory access is allowed.
 *
 * This function verifies if the specified memory range is accessible
 * for the given operation type (read or write).
 * @param type Type of access (VERIFY_READ or VERIFY_WRITE).
 * @param addr Starting address to check.
 * @param size Size of the memory region to check.
 * @return 1 if access is allowed, 0 otherwise.
 */
int access_ok(int type, const void *addr, unsigned long size);

#define min(a, b) (a < b ? a : b)

/**
 * @brief Get current system ticks.
 *
 * This function returns the current system timer ticks.
 * @return Current system ticks.
 */
unsigned long get_ticks(void);

/**
 * @brief Convert integer to hexadecimal string.
 *
 * This function converts an unsigned integer to its hexadecimal
 * string representation.
 * @param num Unsigned integer to convert.
 * @param buffer Buffer to store the hexadecimal string.
 */
void itoa_hex(unsigned int num, char *buffer);

/**
 * @brief Print system splash screen.
 *
 * This function prints the ZeOS startup splash screen with system information.
 */
void print_splash_screen(void);

/**
 * @brief Wait for specified number of ticks.
 *
 * This function implements a busy-wait delay for the specified number
 * of timer ticks.
 * @param ticks_to_wait Number of ticks to wait.
 */
void wait_ticks(int ticks_to_wait);
#endif // UTILS_H
