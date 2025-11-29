/**
 * @file system.c
 * @brief Core system initialization and main kernel entry point for ZeOS.
 *
 * This file contains the main kernel initialization sequence including
 * hardware setup, memory management, interrupt configuration, and
 * the transition to user mode execution.
 */

#include <hardware.h>
#include <interrupt.h>
#include <io.h>
#include <keyboard.h>
#include <mm.h>
#include <sched.h>
#include <segment.h>
#include <system.h>
#include <types.h>
#include <utils.h>
// #include <zeos_mm.h> /* DEPRECATED: TO BE DELETED WHEN PROCESS MANAGEMENT IS FULLY IMPLEMENTED */

int (*usr_main)(void) = (void *)(PAG_LOG_INIT_CODE * PAGE_SIZE);
unsigned int *p_sys_size = (unsigned int *)KERNEL_START;
unsigned int *p_usr_size = (unsigned int *)KERNEL_START + 1;
unsigned int *p_rdtr = (unsigned int *)KERNEL_START + 2;

/**************************
 ** setSegmentRegisters ***
 **************************
 * Set properly all the registers, used
 * at initialization code.
 *   DS, ES, FS, GS <- DS
 *   SS:ESP <- DS:DATA_SEGMENT_SIZE
 *         (the stacks grows towards 0)
 *
 * cld -> gcc2 wants DF (Direction Flag (eFlags.df))
 *        always clear.
 */

/*
 * This function MUST be 'inline' because it modifies the %esp
 */
inline void set_seg_regs(Word data_sel, Word stack_sel, DWord esp) {
    esp = esp - 5 * sizeof(DWord); /* To avoid overwriting task 1 */
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

/*
 *   Main entry point to ZEOS Operating System
 */
__attribute__((__section__(".text.main"))) int main(void) {

    set_eflags();

    /* Define the kernel segment registers  and a stack to execute the 'main'
     * code */
    // It is necessary to use a global static array for the stack, because the
    // compiler will know its final memory location. Otherwise it will try to
    // use the 'ds' register to access the address... but we are not ready for
    // that yet (we are still in real mode).
    set_seg_regs(__KERNEL_DS, __KERNEL_DS, (DWord)&tasks[4]);

    /*** DO *NOT* ADD ANY CODE IN THIS ROUTINE BEFORE THIS POINT ***/

    print_splash_screen();

    /* Initialize hardware data */
    setGdt(); /* Definition of the memory segments table */
    setIdt(); /* Definition of the interrupt vector */
    setTSS(); /* Definition of the TSS */

    /* Initialize Memory */
    init_mm();

    /* Initialize Scheduling */
    init_sched();

    /* Initialize idle task data */
    init_idle();
    /* Initialize task 1 data */
    init_task1();

    /* Keyboard support is initialized per-task in init_task1/init_idle */

    /* Move user code/data now (after the page table initialization) */
    copy_data((void *)KERNEL_START + *p_sys_size, (void *)L_USER_START, *p_usr_size);

    printk("Entering user mode...\n\n");

    enable_int();
    /*
     * We return from a 'theorical' call to a 'call gate' to reduce our
     * privileges and going to execute 'magically' at 'usr_main'...
     */
    return_gate(__USER_DS, __USER_DS, USER_ESP, __USER_CS, (DWord)usr_main);

    /* The execution never arrives to this point */
    return 0;
}
