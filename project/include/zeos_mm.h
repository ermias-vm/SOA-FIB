/**
 * @file zeos_mm.h
 * @brief ZeOS-specific memory management extensions for ZeOS.
 *
 * This header defines ZeOS-specific memory management functions
 * and address space initialization routines.
 */

#ifndef __ZEOS_MM_H__
#define __ZEOS_MM_H__

/**
 * @brief Initialize address space for monoprocess operation.
 *
 * This function initializes the memory address space for single-process
 * operation mode in ZeOS.
 */
void monoprocess_init_addr_space(void);

#endif /* __ZEOS_MM_H__ */
