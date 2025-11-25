/**
 * @file sys_call_table.S
 * @brief System call dispatch table for ZeOS.
 * 
 * This file defines the system call table that maps system call
 * numbers to their corresponding kernel implementation functions.
 */

#include <asm.h>
#include <segment.h>

ENTRY (sys_call_table)
    .long sys_ni_syscall	# 0
    .long sys_exit          # 1 (ok)
    .long sys_fork          # 2 (ok)
    .long sys_ni_syscall    # 3
    .long sys_write			# 4 (ok)
    .long sys_exit_thread	# 5 ()
    .long sys_create_thread	# 6 ()
    .long sys_ni_syscall	# 7
    .long sys_ni_syscall	# 8
    .long sys_ni_syscall	# 9
    .long sys_gettime	    # 10 (ok)
    .long sys_ni_syscall	# 11
    .long sys_block	        # 12 (ok)
    .long sys_unblock	    # 13 (ok)
    .long sys_ni_syscall	# 14
    .long sys_ni_syscall	# 15
    .long sys_ni_syscall	# 16
    .long sys_ni_syscall	# 17
    .long sys_ni_syscall	# 18
    .long sys_ni_syscall	# 19
    .long sys_getpid        # 20 (ok)
    .long sys_gettid        # 21 (ok)
    .long sys_KeyboardEvent_wrapper    # 22 KeyboardEvent

.globl MAX_SYSCALL
MAX_SYSCALL = (. - sys_call_table)/4 
