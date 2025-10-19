/*
 * entry.S - Entry point to system mode from user mode
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
    push $__USER_DS
    pushl %ebp
    pushfl
    push $__USER_CS
    pushl 4(%ebp)
    SAVE_ALL

    cmpl $0, %eax
    jl sysenter_err
    cmpl $MAX_SYSCALL, %eax
    jg sysenter_err
    call *sys_call_table(, %eax, 0x04)
    jmp sysenter_return

sysenter_err:
    movl $-ENOSYS, %eax

sysenter_return:
    movl %eax, 0x18(%esp)
    RESTORE_ALL
    movl (%esp), %edx       # edx = eip user 
    movl 0x0c(%esp), %ecx   # ecx = oldesp user
    sti
    sysexit                         



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
    pushl 0x30(%esp)            # Push EIP
    call pageFault_routine
    RESTORE_ALL                 # This line is never reached
    iret