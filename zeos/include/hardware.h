/*
 * hardware.h - Rutines hardware per manegar els accesos a baix nivell
 */

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include <types.h>

/**
 * @brief Get current EFLAGS register value
 *
 * Assembly routine that returns the current value of the EFLAGS register,
 * which contains processor status and control flags.
 *
 * @return Current EFLAGS register value
 */
DWord get_eflags(void);

/**
 * @brief Set EFLAGS register value
 *
 * Assembly routine that sets the EFLAGS register to enable/disable
 * processor features and flags.
 */
void set_eflags(void);

/**
 * @brief Load Interrupt Descriptor Table Register
 *
 * Assembly routine that loads the IDTR register with the address
 * and size of the Interrupt Descriptor Table.
 *
 * @param idt Pointer to IDT register structure containing base and limit
 */
void set_idt_reg(Register *idt);

/**
 * @brief Load Global Descriptor Table Register
 *
 * Assembly routine that loads the GDTR register with the address
 * and size of the Global Descriptor Table.
 *
 * @param gdt Pointer to GDT register structure containing base and limit
 */
void set_gdt_reg(Register *gdt);

/**
 * @brief Load Local Descriptor Table Register
 *
 * Assembly routine that loads the LDTR register with the selector
 * for the Local Descriptor Table.
 *
 * @param ldt LDT selector value
 */
void set_ldt_reg(Selector ldt);

/**
 * @brief Load Task Register
 *
 * Assembly routine that loads the TR register with the selector
 * for the Task State Segment.
 *
 * @param tr TSS selector value
 */
void set_task_reg(Selector tr);

/**
 * @brief Return to user mode through a gate
 *
 * Assembly routine that performs a transition from kernel mode to user mode
 * by setting up the appropriate segment selectors and stack, then executing
 * an IRET instruction.
 *
 * @param ds Data segment selector for user mode
 * @param ss Stack segment selector for user mode
 * @param esp Stack pointer for user mode
 * @param cs Code segment selector for user mode
 * @param eip Instruction pointer for user mode
 */
void return_gate(Word ds, Word ss, DWord esp, Word cs, DWord eip);

/**
 * @brief Enable hardware interrupts
 *
 * Configures the Programmable Interrupt Controller (PIC) mask register
 * to enable specific hardware interrupts. The 8259 PIC uses register 0x21
 * to control which interrupts are enabled/disabled.
 *
 * Register 0x21 bit mapping:
 * - bit 0: Timer interrupt
 * - bit 1: Keyboard interrupt
 * - bit 2: PIC cascading
 * - bit 3: 2nd Serial Port
 * - bit 4: 1st Serial Port
 * - bit 5: Reserved
 * - bit 6: Floppy disk
 * - bit 7: Reserved
 *
 * Note: 0 = interrupt enabled, 1 = interrupt disabled
 */
void enable_int(void);

/**
 * @brief Introduce a short delay
 *
 * Assembly routine that introduces a brief delay, typically used
 * for timing-sensitive hardware operations or debouncing.
 */
void delay(void);
#endif /* __HARDWARE_H__ */
