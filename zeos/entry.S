/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include <errno.h>

/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %ebx    \                        **/  // first parameter
/**   4(%esp) - %ecx     |                       **/  // second parameter
/**   8(%esp) - %edx     |                       **/  // third parameter
/**   C(%esp) - %esi     | Register saved        **/  // fourth parameter
/**  10(%esp) - %edi     |  by 'save'            **/  // fifth parameter
/**  14(%esp) - %ebp     |                       **/  // sixth parameter
/**  18(%esp) - %eax     |                       **/  // result of the system call
/**  1C(%esp) - %ds      |                       **/  // data segment
/**  20(%esp) - %es      |                       **/  // extra segment
/**  24(%esp) - %fs      |                       **/  // file segment
/**  28(%esp) - %gs     /                        **/  // global segment
/**  2C(%esp) - %eip    \                        **/  // user space instruction pointer
/**  30(%esp) - %cs      |                       **/  // user space code segment
/**  34(%esp) - %eflags  |  Return context saved **/  // user space eflags
/**  38(%esp) - %oldesp  |   by the processor.   **/  // user space stack pointer
/**  3C(%esp) - %oldss  /                        **/  // user space stack segment
/**                                              **/
/**************************************************/

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
    movl %edx, %ds;           \
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
    push $__USER_DS
    pushl %EBP
    pushfl
    push $__USER_CS
    pushl 4(%EBP)
    SAVE_ALL
    cmpl $0, %EAX
    jl sysenter_err
    cmpl $MAX_SYSCALL, %EAX
    jg sysenter_err
    call *sys_call_table(, %EAX, 0x04)
    
    movl %EAX, 0x18(%ESP)
    RESTORE_ALL
    movl (%ESP), %EDX
    movl 12(%ESP), %ECX
    sti
    sysexit

sysenter_err:
    movl $-ENOSYS, %EAX



ENTRY(clock_handler)
    SAVE_ALL
    EOI                     # EOI before call 
    call clock_routine
    RESTORE_ALL
    iret


ENTRY(keyboard_handler) 
    SAVE_ALL
    call keyboard_routine
    EOI                     # EOI after call        
    RESTORE_ALL
    iret


 ENTRY(writeMSR)
    pushl %ebp
    movl %esp, %ebp
    movl 0x8(%ebp), %ecx
    movl $0, %edx
    movl 0xc(%ebp), %eax
    wrmsr
    popl %ebp
    ret


ENTRY(pageFault_handler)
    SAVE_ALL
    pushl 44(%esp)            # Push EIP
    call pageFault_routine
    addl $4, %esp
    RESTORE_ALL
    iret
