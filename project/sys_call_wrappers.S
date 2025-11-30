/**
 * @file sys_call_wrappers.S
 * @brief System call wrapper functions for ZeOS user-kernel interface.
 * 
 * This file contains optimized assembly wrappers for system calls using
 * SYSENTER/SYSEXIT instructions for fast user-kernel mode transitions.
 */

#include <asm.h>

# caller-saved registers: eax, ecx, edx
# callee-saved registers: ebp, edi, esi, ebx

ENTRY(write)
    pushl %ebp
    movl %esp,%ebp
    pushl %ebx

    movl 0x08(%ebp), %ebx 
    movl 0x0c(%ebp), %ecx
    movl 0x10(%ebp), %edx
    movl $4, %eax
    
    pushl $wr_return
    pushl %ebp
    movl %esp,%ebp
    sysenter
    
wr_return:
    popl %ebp
    addl $4, %esp
    popl %ebx
    popl %ebp
    test %eax, %eax
    js wr_error
    ret
    
wr_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret


ENTRY(gettime)
    pushl %ebp
    movl %esp, %ebp
    movl $10, %eax

    pushl $gt_return
    pushl %ebp
    movl %esp,%ebp
    sysenter

gt_return:
    popl %ebp
    addl $4, %esp
    popl %ebp
    test %eax, %eax
    js gt_error
    ret

gt_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret


ENTRY(getpid)
    pushl %ebp
    movl %esp, %ebp
    movl $20, %eax
    
    pushl $gp_return
    pushl %ebp
    movl %esp, %ebp
    sysenter

gp_return:
    popl %ebp
    addl $4, %esp
    popl %ebp
    test %eax, %eax
    js gp_error
    ret
 
gp_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret


ENTRY(fork)
    pushl %ebp
    movl %esp, %ebp
    movl $2, %eax
    
    pushl $fork_return
    pushl %ebp
    movl %esp, %ebp
    sysenter

fork_return:
    popl %ebp
    addl $4, %esp
    popl %ebp
    test %eax, %eax
    js fork_error
    ret
 
fork_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret


ENTRY(exit)
    pushl %ebp
    movl %esp, %ebp
    movl $1, %eax
    
    pushl $exit_return
    pushl %ebp
    movl %esp, %ebp
    sysenter

exit_return:
    popl %ebp
    addl $4, %esp
    popl %ebp
    ret


ENTRY(block)
    pushl %ebp
    movl %esp, %ebp
    movl $12, %eax
    
    pushl $block_return
    pushl %ebp
    movl %esp, %ebp
    sysenter

block_return:
    popl %ebp
    addl $4, %esp
    popl %ebp
    test %eax, %eax
    js block_error
    ret

block_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret


ENTRY(unblock)
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    movl $13, %eax
    
    movl 0x08(%ebp), %ebx 
    pushl $unblock_return
    pushl %ebp
    movl %esp, %ebp
    sysenter

unblock_return:
    popl %ebp
    addl $4, %esp
    popl %ebx
    popl %ebp
    test %eax, %eax
    js unblock_error
    ret

unblock_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret





ENTRY(ThreadCreate)
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx
    
    movl $6, %eax
    movl 0x08(%ebp), %ebx  # function pointer
    movl 0x0c(%ebp), %ecx  # parameter
    leal thread_entry_wrapper, %edx  # wrapper function address
    
    pushl $tc_return
    pushl %ebp
    movl %esp, %ebp
    sysenter

tc_return:
    popl %ebp
    addl $4, %esp
    popl %ebx
    popl %ebp
    test %eax, %eax
    js tc_error
    ret

tc_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret


ENTRY(ThreadExit)
    pushl %ebp
    movl %esp, %ebp
    movl $5, %eax
    
    pushl $te_return
    pushl %ebp
    movl %esp, %ebp
    sysenter

te_return:
    popl %ebp
    addl $4, %esp
    popl %ebp
    ret


ENTRY(gettid)
    pushl %ebp
    movl %esp, %ebp
    movl $21, %eax
    
    pushl $gtid_return
    pushl %ebp
    movl %esp, %ebp
    sysenter

gtid_return:
    popl %ebp
    addl $4, %esp
    popl %ebp
    test %eax, %eax
    js gtid_error
    ret
 
gtid_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret

ENTRY(KeyboardEvent)
    pushl %ebp
    movl %esp, %ebp
    pushl %ebx

    movl $22, %eax
    movl 8(%ebp), %ebx          # func parameter
    leal kbd_wrapper, %ecx      # wrapper function address

    pushl $ke_return
    pushl %ebp
    movl %esp, %ebp
    sysenter

ke_return:
    popl %ebp
    addl $4, %esp
    popl %ebx
    popl %ebp
    test %eax, %eax
    js ke_error
    ret

ke_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret


ENTRY(WaitForTick)
    pushl %ebp
    movl %esp, %ebp
    movl $23, %eax

    pushl $wft_return
    pushl %ebp
    movl %esp, %ebp
    sysenter

wft_return:
    popl %ebp
    addl $4, %esp
    popl %ebp
    test %eax, %eax
    js wft_error
    ret

wft_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret


ENTRY(thread_entry_wrapper)
    movl 4(%esp), %eax      # %eax = function pointer
    movl 8(%esp), %ecx      # %ecx = parameter
    pushl %ecx
    call *%eax
    addl $4, %esp
    call ThreadExit         # ALWAYS call ThreadExit when function returns


ENTRY(kbd_wrapper)
    pushl 12(%esp)          # Push pressed (3rd original param becomes 2nd)
    pushl 12(%esp)          # Push key (2nd original param, now at offset 12 due to push)
    movl 12(%esp), %eax     # Get handler address (1st original param, now at offset 12)
    call *%eax              # Call user handler
    addl $8, %esp           # Clean up parameters

    int $0x2b               # Trigger resume handler

