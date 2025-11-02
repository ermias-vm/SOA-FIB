/*
 * interrupt.c -
 */
#include <hardware.h>
#include <interrupt.h>
#include <io.h>
#include <sched.h>
#include <segment.h>
#include <types.h>
#include <utils.h>
#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register idtR;

int zeos_ticks = 0;

char char_map[] = {'\0', '\0', '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0',  '\'',
                   '\0', '\0', '\0', 'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',  'o',  'p',
                   '`',  '+',  '\0', '\0', 'a',  's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',
                   '\0', '\0', '\0', '\0', '\0', 'z',  'x',  'c',  'v',  'b',  'n',  'm',  ',',
                   '.',  '-',  '\0', '*',  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '7',  '8',  '9',  '-',  '4',  '5',  '6',
                   '+',  '1',  '2',  '3',  '0',  '\0', '\0', '\0', '<',  '\0', '\0', '\0', '\0',
                   '\0', '\0', '\0', '\0', '\0', '\0', '\0'};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL) {
    /***********************************************************************/
    /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
    /* ***************************                                         */
    /* flags = x xx 0x110 000 ?????                                        */
    /*         |  |  |                                                     */
    /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
    /*         |   \ DPL = Num. higher PL from which it is accessible      */
    /*          \ P = Segment Present bit                                  */
    /***********************************************************************/
    Word flags = (Word)(maxAccessibleFromPL << 13);
    flags |= 0x8E00; /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

    idt[vector].lowOffset = lowWord((DWord)handler);
    idt[vector].segmentSelector = __KERNEL_CS;
    idt[vector].flags = flags;
    idt[vector].highOffset = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL) {
    /***********************************************************************/
    /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
    /* ********************                                                */
    /* flags = x xx 0x111 000 ?????                                        */
    /*         |  |  |                                                     */
    /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
    /*         |   \ DPL = Num. higher PL from which it is accessible      */
    /*          \ P = Segment Present bit                                  */
    /***********************************************************************/
    Word flags = (Word)(maxAccessibleFromPL << 13);

    // flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
    /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
       the system calls will be thread-safe. */
    flags |= 0x8E00; /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

    idt[vector].lowOffset = lowWord((DWord)handler);
    idt[vector].segmentSelector = __KERNEL_CS;
    idt[vector].flags = flags;
    idt[vector].highOffset = highWord((DWord)handler);
}

void setIdt() {
    /* Program interrups/exception service routines */
    idtR.base = (DWord)idt;
    idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;

    set_handlers();

    /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
    setInterruptHandler(32, clock_handler, 0);
    setInterruptHandler(33, keyboard_handler, 0);
    setInterruptHandler(14, pageFault_handler, 0);

    writeMSR(0x174, __KERNEL_CS);
    writeMSR(0x175, INITIAL_ESP);
    writeMSR(0x176, (unsigned long)syscall_handler_sysenter);

    set_idt_reg(&idtR);
}

void keyboard_routine() {
    unsigned char key = inb(0x60); // Read the value from port 0x60 (keyboard port)
    // Bit 7 indicates if the key was pressed (MAKE = 0) or released (BREAK = 1)
    int isBreak = key & 0x80; // 0x80 & 00000000 = 0 // Make  <--> 0x80 & 10000000 = 1 // Break
    if (!isBreak) {
        char pressedKey = char_map[key & 0x7F]; // Get the character from char_map
        if (pressedKey == '\0') {
            pressedKey = 'C';
        }

        testTaskSwitch(pressedKey); // Test de cambio de contexto
        printc_xy(0, 0, pressedKey, INFO_COLOR);
    }
}

void clock_routine(void) {
    zeos_ticks++;
    zeos_show_clock();
}

void pageFault_routine(unsigned int eip) {
    char buffer_eip[11];

    itoa_hex(eip, buffer_eip);

    printk_color("\n===============================================\n", ERROR_COLOR);
    printk_color("           PAGE FAULT EXCEPTION               \n", ERROR_COLOR);
    printk_color("===============================================\n", ERROR_COLOR);
    printk_color("\n  Error at EIP:   ", WARNING_COLOR);
    printk_color(buffer_eip, MAKE_COLOR(BLACK, WHITE));
    printk_color("\n\n  The process tried to access an invalid\n", INFO_COLOR);
    printk_color("  memory address and will be terminated.\n", INFO_COLOR);
    printk_color("===============================================\n", ERROR_COLOR);
    while (1) {
    }
}

void testTaskSwitch(char key) {
    if (key == '0') {
        printk_color("\n[BEFORE_TASK_SWITCH] Switching to idle task\n", INFO_COLOR);
        task_switch((union task_union *)idle_task);
        printk_color("\n[AFTER_TASK_SWITCH] Switched to idle task\n",
                     INFO_COLOR); // Should not reach here
    }

    else if (key == '1') {
        printk_color("\n[BEFORE_TASK_SWITCH] Switching to init task\n", INFO_COLOR);
        task_switch((union task_union *)init_task);
        printk_color("\n[AFTER_TASK_SWITCH] Switched to init task\n",
                     INFO_COLOR); // Should not reach here
    }
}
