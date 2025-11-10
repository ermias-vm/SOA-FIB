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


    ################# STACK LAYOUT EXAMPLES #################


    # PARENT PROCESS KERNEL STACK (Complete clock interrupt to switch_context flow):
    # 
    # Lower addresses (stack grows UP)
    # +------------------+ 
    # |      ......      |
    # +------------------+
    # | @ret_scheduler   | <- return address (where switch_context returns)
    # +------------------+ <- ESP points here before switch_context
    # | &parent->k_esp   | <- esp + 4 (address to save parent's ESP)
    # +------------------+
    # | child->k_esp     | <- esp + 8 (child's ESP = fake EBP address)
    # +------------------+
    # | scheduler locals | <- Local vars from scheduler(), sched_next_rr()
    # +------------------+
    # | saved EBP        | <- pushl %ebp (clock_routine activation frame)
    # +------------------+
    # | @ret_clock_rout  | <- return address back to clock_routine
    # +------------------+
    # | saved EBP        | <- pushl %ebp (clock_routine activation frame)
    # +------------------+
    # | @ret_clock_hand  | <- return address back to clock_handler
    # +------------------+
    # | CTX SW (11 regs) | <- Software context: EBX,ECX,EDX,ESI,EDI,EBP,EAX,DS,ES,FS,GS
    # +------------------+
    # | CTX HW (5 regs)  | <- Hardware context: EIP,CS,EFLAGS,ESP,SS (saved by CPU)
    # +------------------+
    # Higher addresses


    # PARENT PROCESS KERNEL STACK (Complete fork() to switch_context flow):
    # 
    # Lower addresses (stack grows UP)
    # +------------------+ 
    # | task_switch regs |
    # | (EBX, ESI, EDI)  |
    # +------------------+
    # | inner_task_switch|
    # | local vars       |
    # +------------------+
    # | @ret_scheduler   | <- return address (where switch_context returns)
    # +------------------+ <- ESP points here before switch_context
    # | &parent->k_esp   | <- esp + 4 (address to save parent's ESP)
    # +------------------+
    # | child->k_esp     | <- esp + 8 (child's ESP = fake EBP address)
    # +------------------+
    # | @sysenter_return | <- Return address for sysenter handler
    # +------------------+
    # | CTX SW (11 regs) | <- Software context: EBX,ECX,EDX,ESI,EDI,EBP,EAX,DS,ES,FS,GS
    # +------------------+
    # | CTX HW (5 regs)  | <- Hardware context: EIP,CS,EFLAGS,ESP,SS (saved by CPU)
    # +------------------+
    # Higher addresses


    # CHILD PROCESS STACK (prepared by sys_fork):
    #
    # Lower addresses
    # +------------------+ <- child->kernel_esp points here  
    # | 0 (fake EBP)     | <- Will be popped into EBP by switch_context
    # +------------------+
    # | @ret_from_fork   | <- Return address for first child execution
    # +------------------+
    # | @sysenter_return | <- Return address for sysenter handler (Copied from parent)
    # +------------------+
    # | CTX SW (11 regs) | <- Software context: (Copied from parent)
    # +------------------+
    # | CTX HW (5 regs)  | <- Hardware context: (Copied from parent)
    # +------------------+
    # Higher addresses