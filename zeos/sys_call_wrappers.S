#include <asm.h>

ENTRY(write)
    pushl %ebp
    movl %esp,%ebp
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
    cmpl $0, %eax 	
    jl wr_error
    popl %ebp
    ret
    
wr_error:
    negl %eax
    movl %eax, errno
    movl -1, %eax


ENTRY(gettime)
    pushl %ebp
    movl %esp, %ebp
    pushl %esi
    movl $10, %eax
    pushl %edx
    pushl %ecx
    leal gt_return, %esi
    pushl %esi
    pushl %ebp
    movl %esp, %ebp
    sysenter

gt_return:
    popl %ebp
    popl %esi
    popl %edx
    popl %ecx
    cmpl $0, %eax
    jl gt_error
    popl %esi
    movl %ebp, %esp
    popl %ebp
    ret

gt_error:
    negl %eax
    movl %eax, errno
    movl -1, %eax
