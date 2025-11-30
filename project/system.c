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

int (*usr_main)(void) = (void *)(PAG_LOG_INIT_CODE * PAGE_SIZE);
unsigned int *p_sys_size = (unsigned int *)KERNEL_START;
unsigned int *p_usr_size = (unsigned int *)KERNEL_START + 1;
unsigned int *p_rdtr = (unsigned int *)KERNEL_START + 2;

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
