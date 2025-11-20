/**
 * @file asm.h
 * @brief Assembly language macros and definitions for ZeOS.
 *
 * This header provides macros and definitions for assembly
 * files, including function entry/exit macros and constants.
 */

#ifndef __ASM_H__
#define __ASM_H__

#define ENTRY(name)                                                                                \
    .globl name;                                                                                   \
    .type name, @function;                                                                         \
    .align 0;                                                                                      \
    name:

#endif /* __ASM_H__ */
