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
    jge wr_no_error

    negl %eax
    movl %eax, errno
    movl -1, %eax
wr_no_error:
    popl %ebp
    ret