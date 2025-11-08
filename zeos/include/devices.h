/**
 * @file devices.h
 * @brief Device driver interface definitions for ZeOS.
 *
 * This header defines device driver functions and interfaces
 * for hardware device access and management in ZeOS.
 */

#ifndef __DEVICES_H__
#define __DEVICES_H__

/**
 * @brief Write data to console device.
 *
 * This function writes a buffer of characters directly to the console device.
 * Used by the write system call for console output.
 * @param buffer Character buffer to write to console.
 * @param size Number of bytes to write.
 * @return Number of bytes written to console.
 */
int sys_write_console(char *buffer, int size);

#endif /* __DEVICES_H__ */
