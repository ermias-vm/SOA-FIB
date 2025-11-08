/**
 * @file kernel_asm.S
 * @brief Low-level assembly routines for kernel operations in ZeOS.
 * 
 * This file contains critical assembly functions for memory management,
 * CPU control operations, and hardware-specific kernel functionality.
 */

#include <asm.h>
#include <segment.h>

ENTRY(writeMSR)
    pushl %ebp
    movl %esp, %ebp
    movl 0x8(%ebp), %ecx
    movl $0, %edx
    movl 0xc(%ebp), %eax
    wrmsr
    movl %ebp, %esp
    popl %ebp
    ret


ENTRY(task_switch)
    pushl %ebp
    movl %esp, %ebp

    pushl %edi
    pushl %esi
    pushl %ebx

    pushl 8(%ebp)               # Push new task pointer
    call inner_task_switch
    addl $4, %esp

    popl %ebx
    popl %esi
    popl %edi
    movl %ebp, %esp
    popl %ebp
    ret


ENTRY(switch_context)
    movl 4(%esp), %eax          # eax =  &current()->kernel_esp
    movl %ebp, (%eax)           # Save current esp, *eax = %ebp
    movl 8(%esp), %esp          # esp = new_esp
    popl %ebp
    ret

    # STACK LAYOUT:
    # +------------------+
    # | return address   | <- esp + 0
    # +------------------+
    # | &current()->esp  | <- esp + 4
    # +------------------+
    # | new_esp          | <- esp + 8
    # +------------------+
    # | %ebx             |
    # +------------------+
    # | %esi             |
    # +------------------+
    # | %edi             |
    # +------------------+
    # | %ebp             |
    # +------------------+              
