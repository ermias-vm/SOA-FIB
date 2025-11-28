/**
 * @file entry.S
 * @brief Low-level interrupt and system call entry points for ZeOS.
 * 
 * This file contains assembly routines for interrupt handling, system call
 * processing, context switching, and user-kernel mode transitions.
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>

/*********************************************************/
/**** Save & Restore *************************************/
/**                                                     **/
/** When we change to privilege level 0 (kernel)        **/
/** (through an interrupt, a system call, an            **/
/** exception ...) we must save the state of the        **/
/** currently running task (save).                      **/
/**                                                     **/
/** Stack layout in 'systemCall':                       **/
/**     0x00(%esp) - %ebx     \                         **/  // first parameter
/**     0x04(%esp) - %ecx     |                         **/  // second parameter
/**     0x08(%esp) - %edx     |                         **/  // third parameter
/**     0x0c(%esp) - %esi     | Register saved          **/  // fourth parameter
/**     0x10(%esp) - %edi     |  by 'save'              **/  // fifth parameter
/**     0x14(%esp) - %ebp     |                         **/  // sixth parameter
/**     0x18(%esp) - %eax     |                         **/  // result of the system call
/**     0x1c(%esp) - %ds      |                         **/  // data segment
/**     0x20(%esp) - %es      |                         **/  // extra segment
/**     0x24(%esp) - %fs      |                         **/  // file segment
/**     0x28(%esp) - %gs      |                         **/  // global segment
/**     0x2C(%esp) - %eip     |                         **/  // user space instruction pointer
/**     0x30(%esp) - %cs      |                         **/  // user space code segment
/**     0x34(%esp) - %eflags  |  Return context saved   **/  // user space eflags
/**     0x38(%esp) - %oldesp  |   by the processor.     **/  // user space stack pointer
/**     0x3C(%esp) - %oldss   |                         **/  // user space stack segment
/**                                                     **/
/*********************************************************/

#define SAVE_ALL \
    pushl %gs; \
    pushl %fs; \
    pushl %es; \
    pushl %ds; \
    pushl %eax; \
    pushl %ebp; \
    pushl %edi; \
    pushl %esi; \
    pushl %edx; \
    pushl %ecx; \
    pushl %ebx; \
    movl $__KERNEL_DS, %edx;    \
    movl %edx, %ds;             \
    movl %edx, %es

#define RESTORE_ALL \
    popl %ebx; \
    popl %ecx; \
    popl %edx; \
    popl %esi; \
    popl %edi; \
    popl %ebp; \
    popl %eax; \
    popl %ds; \
    popl %es; \
    popl %fs; \
    popl %gs; \

#define EOI \
    movb $0x20, %al; \
    outb %al, $0x20;



ENTRY(syscall_handler_sysenter)
    # sysenter does not save hardware context automatically as INT
    push $__USER_DS             # user SS (stack segment)
    pushl %ebp                  # user ESP (EBP contains user ESP)
    pushfl                      # save EFLAGS manually
    push $__USER_CS             # user CS (code segment)
    pushl 4(%ebp)               # user EIP (return address)

    SAVE_ALL                    # save software context (all registers)

    cmpl $0, %eax
    jl sysenter_err
    cmpl $MAX_SYSCALL, %eax
    jg sysenter_err
    call *sys_call_table(, %eax, 0x04)
    jmp sysenter_return

sysenter_err:
    movl $-ENOSYS, %eax

sysenter_return:
    movl %eax, 0x18(%esp)       # store return value in saved context
    RESTORE_ALL                 # restore software context (all registers)
    movl (%esp), %edx           # edx = eip user 
    movl 0x0c(%esp), %ecx       # ecx = oldesp user
    sti
    sysexit                         


ENTRY(clock_handler)
    SAVE_ALL
    EOI                         # EOI before call 
    call clock_routine
    RESTORE_ALL
    iret

ENTRY(keyboard_handler) 
    SAVE_ALL
    call keyboard_routine
    EOI                         # EOI after call        
    RESTORE_ALL
    iret

ENTRY(pageFault_handler)
    SAVE_ALL
    movl %cr2, %eax
    pushl %eax                  # Faulting address
    pushl 0x34(%esp)            # Adjusted offset -> user EIP
    call pageFault_routine
    addl $8, %esp
    RESTORE_ALL
    addl $4, %esp               # Discard error code
    iret


ENTRY(kbd_irq_entry)
    SAVE_ALL
    call kbd_irq_handler
    EOI
    RESTORE_ALL
    iret


ENTRY(kbd_resume_entry)
    SAVE_ALL
    call kbd_resume_handler
    RESTORE_ALL
    iret

