/**
 * @file mm_address.h
 * @brief Memory address layout and page management constants for ZeOS.
 *
 * This header defines memory layout constants, page sizes, and address
 * calculations for memory management in the ZeOS kernel.
 */

#ifndef __MM_ADDRESS_H__
#define __MM_ADDRESS_H__

/* Page directory entry index for page directory pages */
#define ENTRY_DIR_PAGES 0

/* Total number of pages in the system (4MB / 4KB = 1024) */
#define TOTAL_PAGES 1024

/* Number of pages reserved for kernel use */
#define NUM_PAG_KERNEL 256

/* Logical page number where user code starts */
#define PAG_LOG_INIT_CODE (PAG_LOG_INIT_DATA + NUM_PAG_DATA)

/* Physical frame number where user code is loaded */
#define FRAME_INIT_CODE (PH_USER_START >> 12)

/* Number of pages for user code segment */
#define NUM_PAG_CODE 8

/* Logical page number where user data starts */
#define PAG_LOG_INIT_DATA (L_USER_START >> 12)

/* Number of pages for user data segment */
#define NUM_PAG_DATA 20

/* Size of a page in bytes (4KB) */
#define PAGE_SIZE 0x1000

/* Physical address where kernel starts (64KB) */
#define KERNEL_START 0x10000

/* Logical address where user space starts (1MB) */
#define L_USER_START 0x100000

/* Physical address where user code starts (1MB) */
#define PH_USER_START 0x100000

/* Initial ESP for user processes (top of data segment - 16 bytes) */
#define USER_ESP L_USER_START + (NUM_PAG_DATA)*0x1000 - 16

/* First page number in user space */
#define USER_FIRST_PAGE (L_USER_START >> 12)

/* Convert virtual address to page number */
#define PH_PAGE(x) (x >> 12)

#endif /* __MM_ADDRESS_H__ */
