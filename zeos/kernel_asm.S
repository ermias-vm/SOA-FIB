/*
 * kernel_asm.S - Assembly routines for kernel operations
 */

#include <asm.h>

 ENTRY(writeMSR)
    pushl %ebp
    movl %esp, %ebp
    movl 0x8(%ebp), %ecx
    movl $0, %edx
    movl 0xc(%ebp), %eax
    wrmsr
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
    popl %ebp
    ret

ENTRY(switch_context)
    movl 4(%ebp), %eax          # eax = current_esp
    movl %esp, (%eax)           # Save current esp
    movl 8(%ebp), %esp          # esp = new_esp
    popl %ebp
    ret
