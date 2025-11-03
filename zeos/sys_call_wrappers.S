#include <asm.h>

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
    cmpl $0, %eax
    jl wr_error
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
    cmpl $0, %eax
    jl gt_error
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
    cmpl $0, %eax
    jl gp_error
    ret
 
gp_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax
    ret