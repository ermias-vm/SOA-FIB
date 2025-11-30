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

/** Entry point to user-space program (PAG_LOG_INIT_CODE * PAGE_SIZE) */
extern int (*usr_main)(void);

/** Pointer to kernel size in bytes, set by bootloader at KERNEL_START */
extern unsigned int *p_sys_size;

/** Pointer to user program size in bytes, set by bootloader at KERNEL_START+4 */
extern unsigned int *p_usr_size;

/** Pointer to initrd address at KERNEL_START+8 (reserved for future use) */
extern unsigned int *p_rdtr;

/**
 * @brief Main entry point to ZeOS Operating System.
 *
 * This function initializes the kernel by setting up hardware, memory
 * management, scheduling, and then transitions to user mode execution.
 * It is placed in a special linker section (.text.main) to ensure
 * correct positioning in the final binary.
 *
 * @return Never returns (transitions to user mode via return_gate).
 */
int main(void);

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
static inline void set_seg_regs(Word data_sel, Word stack_sel, DWord esp) {
    esp = esp - 5 * sizeof(DWord); /* Avoid overwriting task 1 stack */
    __asm__ __volatile__("cld\n\t"
                         "mov %0,%%ds\n\t"
                         "mov %0,%%es\n\t"
                         "mov %0,%%fs\n\t"
                         "mov %0,%%gs\n\t"
                         "mov %1,%%ss\n\t"
                         "mov %2,%%esp"
                         : /* no output */
                         : "r"(data_sel), "r"(stack_sel), "g"(esp));
}

#endif /* __SYSTEM_H__ */
