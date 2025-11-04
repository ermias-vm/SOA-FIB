#ifndef __ZEOS_INTERRUPT_H__
#define __ZEOS_INTERRUPT_H__

#include <sched.h>

/**
 * @brief Display system clock.
 *
 * This function displays the current system clock on the screen.
 */
void zeos_show_clock();

// Exception handlers
/**
 * @brief Handle divide by zero exception.
 *
 * This function handles division by zero exceptions (interrupt 0).
 */
void divide_error_routine();

/**
 * @brief Handle debug exception.
 *
 * This function handles debug exceptions (interrupt 1).
 */
void debug_routine();

/**
 * @brief Handle non-maskable interrupt.
 *
 * This function handles non-maskable interrupts (interrupt 2).
 */
void nm1_routine();

/**
 * @brief Handle breakpoint exception.
 *
 * This function handles breakpoint exceptions (interrupt 3).
 */
void breakpoint_routine();

/**
 * @brief Handle overflow exception.
 *
 * This function handles overflow exceptions (interrupt 4).
 */
void overflow_routine();

/**
 * @brief Handle bounds check exception.
 *
 * This function handles bounds check exceptions (interrupt 5).
 */
void bounds_check_routine();

/**
 * @brief Handle invalid opcode exception.
 *
 * This function handles invalid opcode exceptions (interrupt 6).
 */
void invalid_opcode_routine();

/**
 * @brief Handle device not available exception.
 *
 * This function handles device not available exceptions (interrupt 7).
 */
void device_not_available_routine();

/**
 * @brief Handle double fault exception.
 *
 * This function handles double fault exceptions (interrupt 8).
 */
void double_fault_routine();

/**
 * @brief Handle coprocessor segment overrun exception.
 *
 * This function handles coprocessor segment overrun exceptions (interrupt 9).
 */
void coprocessor_segment_overrun_routine();

/**
 * @brief Handle invalid TSS exception.
 *
 * This function handles invalid TSS exceptions (interrupt 10).
 */
void invalid_tss_routine();

/**
 * @brief Handle segment not present exception.
 *
 * This function handles segment not present exceptions (interrupt 11).
 */
void segment_not_present_routine();

/**
 * @brief Handle stack exception.
 *
 * This function handles stack exceptions (interrupt 12).
 */
void stack_exception_routine();

/**
 * @brief Handle general protection fault.
 *
 * This function handles general protection faults (interrupt 13).
 */
void general_protection_routine();

/**
 * @brief Handle page fault exception.
 *
 * This function handles page fault exceptions (interrupt 14).
 */
void page_fault_routine();

/**
 * @brief Handle Intel reserved exception.
 *
 * This function handles Intel reserved exceptions (interrupt 15).
 */
void intel_reserved_routine();

/**
 * @brief Handle floating point error exception.
 *
 * This function handles floating point error exceptions (interrupt 16).
 */
void floating_point_error_routine();

/**
 * @brief Handle alignment check exception.
 *
 * This function handles alignment check exceptions (interrupt 17).
 */
void alignment_check_routine();

// Hardware interrupt handlers
/**
 * @brief Handle keyboard interrupt.
 *
 * This function handles keyboard hardware interrupts.
 */
void keyboard_routine();

/**
 * @brief Handle timer/clock interrupt.
 *
 * This function handles timer/clock hardware interrupts for system scheduling.
 */
void clock_routine();

/**
 * @brief Set up interrupt handlers.
 *
 * This function initializes and sets up all interrupt and exception handlers
 * in the interrupt descriptor table.
 */
void set_handlers();

#endif /* __ZEOS_INTERRUPT_H__ */
