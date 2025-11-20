/**
 * @file system.h
 * @brief Core system definitions and interfaces for ZeOS.
 *
 * This header defines fundamental system structures, global variables,
 * and core system initialization functions for the ZeOS kernel.
 */

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <types.h>

/* Task State Segment structure used for hardware task switching */
extern TSS tss;

/* Global Descriptor Table pointer for memory segmentation */
extern Descriptor *gdt;

/**
 * @brief Set segment registers during kernel initialization.
 *
 * This function configures the x86 segment registers (DS, ES, FS, GS, SS)
 * and stack pointer (ESP) for kernel mode operation. It must be inline
 * because it directly modifies the stack pointer register.
 *
 * @param data_sel Data segment selector value (typically __KERNEL_DS)
 * @param stack_sel Stack segment selector value (typically __KERNEL_DS)
 * @param esp Initial stack pointer value, adjusted to avoid task 1 stack
 *
 * @note This function:
 *       - Clears the direction flag (CLD) for gcc compatibility
 *       - Sets all segment registers to the same data selector
 *       - Adjusts ESP to prevent overwriting task 1's stack area
 *       - Must be called during early kernel initialization in real mode
 */
inline void set_seg_regs(Word data_sel, Word stack_sel, DWord esp);

#endif /* __SYSTEM_H__ */
