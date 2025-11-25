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
    leal ThreadExit, %edx  # exit_routine
    
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
    
    movl $22, %eax          /* syscall number - adjust as needed */
    movl 8(%ebp), %ebx      /* func parameter */
    int $0x80               /* syscall interrupt */
    
    popl %ebx
    popl %ebp
    ret

