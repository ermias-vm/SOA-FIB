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
/**   0(%esp) - %ebx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %edx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
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
      popl %ebx;   \
      popl %ecx;   \
      popl %edx;   \
      popl %esi;   \
      popl %edi;   \
      popl %ebp;   \
      popl %eax;   \
      popl %ds;    \
      popl %es;    \
      popl %fs;    \
      popl %gs;

#define EOI \
      movb $0x20, %al; \
      outb %al, $0x20;

ENTRY(clock_handler)
      SAVE_ALL
      pushl %eax;
      call user_to_system;
      popl %eax;
      EOI
      call clock_routine;
      pushl %eax;
      call system_to_user;
      popl %eax;
      RESTORE_ALL
      iret;

ENTRY(keyboard_handler)
      SAVE_ALL
      pushl %eax;
      call user_to_system;
      popl %eax;
      EOI
      call keyboard_routine;
      pushl %eax;
      call system_to_user;
      popl %eax;
      RESTORE_ALL
      iret;

ENTRY(system_call_handler)
      push $__USER_DS             # user SS (stack segment)
      pushl %ebp                  # user ESP (EBP contains user ESP)
      pushfl                      # save EFLAGS manually
      push $__USER_CS             # user CS (code segment)
      pushl 4(%ebp)               # user EIP (return address)
      
	SAVE_ALL	// What about arg 6?
	cmpl $0, %eax
	jl sysenter_err
	cmpl $MAX_SYSCALL, %eax
	jg sysenter_err         # (mod) 2.b se gestionan parametros en el sys_write_wrap -> (mmod) 2.d
      # movl 0x8(%ebp), %ebx  // Primer parámetro (ej. fd) - Comentado, se hace en sys_write_wrap
      # movl 0xC(%ebp), %ecx  // Segundo parámetro (ej. buffer) - Comentado, se hace en sys_write_wrap
      # movl 0x10(%ebp), %edx // Tercer parámetro (ej. size) - Comentado, se hace en sys_write_wrap
	call *sys_call_table(, %eax, 0x04)
	jmp sysenter_fin
sysenter_err:
	movl $-ENOSYS, %eax
sysenter_fin:
	movl %eax, 0x18(%esp)
	RESTORE_ALL
	sti
	sysexit

# (mod) 2.d
ENTRY(sys_write_wrap)
	pushl 0x10(%ebp)  // size
	pushl 0xC(%ebp)   // buffer
	pushl 0x8(%ebp)   // fd
	call sys_write
	addl $12, %esp
	ret
