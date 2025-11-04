#ifndef DEVICES_H__
#define DEVICES_H__

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
#endif /* DEVICES_H__*/
