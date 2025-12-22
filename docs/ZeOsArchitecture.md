# ZeOS Internal Architecture

## Overview
ZeOS is a 32-bit teaching operating system for the Intel 80386 architecture that demonstrates fundamental OS concepts. This document provides detailed technical information about all system components, their implementation, and interactions within the ZeOS kernel.

## Table of Contents
1. [Boot System](#boot-system)
2. [Build System & Configuration](#build-system--configuration)
3. [Core System Components](#core-system-components)
4. [Memory Management](#memory-management)
5. [Process Management & Scheduling](#process-management--scheduling)
6. [Interrupt Handling](#interrupt-handling)
7. [System Calls](#system-calls)
8. [Device Management & I/O](#device-management--io)
9. [User Space Components](#user-space-components)
10. [Support Libraries & Utilities](#support-libraries--utilities)
11. [Testing Framework](#testing-framework)

## Boot System

### [`bootsect.S`](../zeos/bootsect.S)

The boot sector is the first code executed when ZeOS starts. This 512-byte assembly program handles the critical transition from BIOS real mode to protected mode and loads the kernel.

**Key Responsibilities:**
- **Self-relocation**: Moves itself from `0x07C00` to `0x90000` to make room for kernel loading
- **Kernel loading**: Reads kernel sectors from disk using BIOS interrupt `0x13` 
- **A20 line activation**: Enables access to memory above 1MB by activating the A20 address line
- **GDT setup**: Creates a basic Global Descriptor Table with kernel code/data segments
- **Protected mode transition**: Switches CPU from 16-bit real mode to 32-bit protected mode
- **Control transfer**: Jumps to kernel entry point at `0x10010`

**Technical Details:**
- Uses BIOS video services for "Loading.." message display
- Implements A20 line testing loop to ensure proper memory access
- Reprograms 8259 PIC controllers to remap hardware interrupts from `0x08-0x0F` to `0x20-0x2F`
- Contains embedded GDT with kernel code (`0x10`), kernel data (`0x18`), user code (`0x23`), and user data (`0x2B`) segments
- Loads kernel image starting at physical address `0x10000`
- User code is loaded at `0x100000` for data and `0x114000` for text sections

## Build System & Configuration

### [`Makefile`](../zeos/Makefile)

The build system orchestrates compilation, linking, and emulation of the ZeOS system through the Makefile.

**Build Process:**
- **Parallel compilation**: Automatically detects CPU cores and enables parallel builds when 4+ cores available
- **Cross-compilation**: Uses 32-bit compilation flags and linking for i386 architecture
- **Dependency tracking**: Manages header file dependencies and automatic rebuilding
- **Target creation**: Builds boot sector, kernel system image, and user programs separately

**Key Targets:**
- `all`: Builds complete `zeos.bin` image ready for emulation
- `emul`: Runs system in Bochs emulator with standard configuration
- `emuldbg`: Runs with Bochs internal debugger enabled
- `gdb`: Runs system with GDB remote debugging support
- `clean`: Removes all build artifacts and temporary files

**Compilation Flags:**
- `-m32`: Forces 32-bit compilation for x86 compatibility
- `-ffreestanding`: Indicates no standard library dependencies
- `-fno-PIC`: Disables position-independent code generation
- `-Wall -Wextra`: Enables detailed compiler warnings

### [`build.c`](../zeos/build.c)

A specialized utility program that concatenates boot sector, kernel, and user components into a single bootable image.

**Image Construction Process:**
1. **Header validation**: Verifies file formats and sizes of input components
2. **Boot sector processing**: Embeds kernel and user sizes at specific boot sector offsets
3. **Component concatenation**: Combines all parts with proper alignment and padding
4. **Size calculations**: Computes and stores component sizes for bootloader consumption

**Utility Functions:**
- [`die()`](../zeos/build.c#L65): Print error message and exit with error code
- [`minix_open()`](../zeos/build.c#L77): Open and validate MINIX filesystem image files
- [`usage()`](../zeos/build.c#L89): Display command-line usage information

**Output Format:**
- Boot sector (512 bytes) with embedded size information
- Kernel image with proper alignment for loading at `0x10000`
- User program positioned for loading at `0x100000`

### Configuration Files

**[`.bochsrc`](../zeos/.bochsrc)** and **[`.bochsrc_gdb`](../zeos/.bochsrc_gdb)**
- Bochs emulator configuration for standard and GDB debugging modes
- Configures memory size, boot device, display options, and debugging features
- Sets up disk image, ROM paths, and I/O device simulation

**[`.gdbcmd`](../zeos/.gdbcmd)**
- GDB initialization script for kernel debugging sessions
- Sets up symbol loading, breakpoints, and debugging convenience functions
- Configures remote debugging connection to Bochs GDB stub

### Linker Scripts

**[`system.lds`](../zeos/system.lds)** - Kernel Image Layout
- Places kernel at physical address `0x10000`
- Defines sections: `.text.main`, `.text`, `.rodata`, `.data`, `.bss`
- Aligns task_union structures to 4KB page boundaries
- Reserves space for embedded user code size information

**[`user.lds`](../zeos/user.lds)** - User Program Layout  
- Positions user data section at `0x100000`
- Places user text section at `0x114000`
- Defines user stack and data segment organization
- Provides proper alignment for memory management

## Core System Components

### [`system.c`](../zeos/system.c) and [`system.h`](../zeos/include/system.h)

The kernel's main entry point and core system initialization functionality.

**System Initialization Sequence:**
1. **Segment register setup**: Configures kernel DS, ES, FS, GS, SS segments and stack pointer
2. **Hardware initialization**: Sets up GDT, IDT, TSS for memory management and interrupts
3. **Memory management**: Initializes paging system and virtual memory structures
4. **Process initialization**: Creates idle task and initial user task (task1)
5. **User space setup**: Copies user program to target memory locations
6. **Mode transition**: Switches from kernel mode to user mode to start user execution

**Key Functions:**
- [`main()`](../zeos/system.c#L61): Primary kernel entry point called from bootloader
- [`set_seg_regs()`](../zeos/system.c#L45): Configures x86 segment registers for kernel operation
- Assembly integration with `entry.S` for low-level hardware operations

**Global Variables:**
- [`usr_main`](../zeos/system.c#L21): Pointer to user program entry point at `0x114000`
- [`p_sys_size`](../zeos/system.c#L22), [`p_usr_size`](../zeos/system.c#L23): Pointers to embedded size information
- `tss`: Task State Segment for hardware task switching
- `gdt`: Global Descriptor Table pointer for memory segmentation
- `gdt`: Global Descriptor Table pointer for memory segmentation

### [`entry.S`](../zeos/entry.S) and [`entry.h`](../zeos/include/entry.h)

Low-level assembly routines and entry point declarations for interrupts, exceptions, and system calls.

**entry.S - Assembly Implementation:**
- **Interrupt handlers**: Hardware interrupt entry points (keyboard, timer)
- **Exception handlers**: CPU exception handling (page faults, protection violations)  
- **System call entry**: SYSENTER-based fast system call mechanism
- **Assembly macros**: [`SAVE_ALL`](../zeos/entry.S#L36), [`RESTORE_ALL`](../zeos/entry.S#L52), and [`EOI`](../zeos/entry.S#L70) for register management

**entry.h - C Interface:**
- **Function declarations**: Extern declarations for assembly entry points
- **C header interface**: Provides prototypes for [`keyboard_handler()`](../zeos/include/entry.h#L39), [`clock_handler()`](../zeos/include/entry.h#L26), [`syscall_handler_sysenter()`](../zeos/include/entry.h#L18), and [`pageFault_handler()`](../zeos/include/entry.h#L47)
- **Include in kernel**: Required by interrupt.c and other kernel modules that reference entry points

**Register Management:**
- Implements [`SAVE_ALL`](../zeos/entry.S#L36) and [`RESTORE_ALL`](../zeos/entry.S#L52) macros for complete CPU state preservation
- Manages kernel/user stack switching during privilege level transitions
- Handles EFLAGS, segment register, and general-purpose register preservation

**Entry Point Functions:**
- **[`syscall_handler_sysenter()`](../zeos/entry.S#L76)**: Fast system call entry using SYSENTER/SYSEXIT
- **[`clock_handler()`](../zeos/entry.S#L103)**: Timer interrupt handler with immediate EOI
- **[`keyboard_handler()`](../zeos/entry.S#L110)**: Keyboard interrupt handler with delayed EOI  
- **[`pageFault_handler()`](../zeos/entry.S#L117)**: Page fault exception handler

**SYSENTER Implementation:**
- Fast system call entry using Intel's SYSENTER/SYSEXIT instructions
- Configured via MSR (Model Specific Register) programming
- Provides significant performance improvement over traditional `int 0x80` mechanism

### [`kernel_asm.S`](../zeos/kernel_asm.S)

Assembly language functions for critical kernel operations requiring direct hardware access.

**Key Functions:**
- [`task_switch()`](../zeos/kernel_asm.S#L24): High-level task switching interface
- [`switch_context()`](../zeos/kernel_asm.S#L44): Low-level ESP switching for context changes
- [`writeMSR()`](../zeos/kernel_asm.S#L12): Model Specific Register write operations for processor configuration
- Hardware-specific operations that cannot be implemented in C

**Context Switching Implementation:**
```assembly
task_switch:
    pushl %ebp
    movl %esp, %ebp
    pushl %edi
    pushl %esi  
    pushl %ebx
    pushl 8(%ebp)           # Push new task pointer
    call inner_task_switch  # Call C portion
    # Register restoration occurs in C code via switch_context
```

## Memory Management

### [`mm.c`](../zeos/mm.c) and [`mm.h`](../zeos/include/mm.h)

Memory management implementation handling paging, virtual memory, and physical frame allocation.

**Physical Memory Management:**
- **Frame allocation**: Bitmap-based tracking of 4KB physical memory frames
- **Free frame list**: Maintains list of available physical memory pages
- **Memory protection**: Enforces kernel/user space separation through paging

**Virtual Memory System:**
- **Page directories**: Per-task page directories for memory space isolation
- **Page tables**: Maps virtual addresses to physical frame numbers
- **Memory mapping**: Handles code, data, and stack segment mapping
- **Copy-on-write**: Framework for efficient memory sharing (future implementation)

**Key Data Structures:**
```c
extern Byte phys_mem[TOTAL_PAGES];                    // Physical frame bitmap
extern page_table_entry dir_pages[NR_TASKS][TOTAL_PAGES];    // Task page directories
extern page_table_entry pagusr_table[NR_TASKS][TOTAL_PAGES]; // Task page tables
```

**Memory Management Functions:**
- [`init_dir_pages()`](../zeos/mm.c#L37): Initialize page directories for all tasks
- [`init_table_pages()`](../zeos/mm.c#L49): Initialize page tables for all tasks
- [`set_user_pages()`](../zeos/mm.c#L68): Configure user virtual memory mappings
- [`set_cr3()`](../zeos/mm.c#L95): Set Control Register 3 with page directory address
- [`set_pe_flag()`](../zeos/mm.c#L108): Enable paging by setting PE flag in CR0
- [`init_mm()`](../zeos/mm.c#L114): Complete memory management system initialization
- [`setGdt()`](../zeos/mm.c#L125): Initialize Global Descriptor Table
- [`setTSS()`](../zeos/mm.c#L140): Initialize Task State Segment
- [`init_frames()`](../zeos/mm.c#L172): Initialize physical frame allocation system
- [`alloc_frame()`](../zeos/mm.c#L185): Allocate physical memory frame
- [`free_user_pages()`](../zeos/mm.c#L199): Free all user pages for a task
- [`free_frame()`](../zeos/mm.c#L209): Deallocate physical memory frame
- [`set_ss_pag()`](../zeos/mm.c#L213): Set page table entry for virtual to physical mapping
- [`del_ss_pag()`](../zeos/mm.c#L221): Delete page table entry and invalidate mapping
- [`get_frame()`](../zeos/mm.c#L225): Get physical frame number from page table entry

**Memory Management Macros:**
- [`read_cr0()`](../zeos/mm.c#L100): Read Control Register 0 (processor control flags)
- [`write_cr0()`](../zeos/mm.c#L106): Write to Control Register 0 (enable/disable paging)

### [`mm_address.h`](../zeos/include/mm_address.h)

Memory layout constants and address space definitions.

**Address Space Layout:**
- `KERNEL_START`: `0x10000` - Kernel load address
- [`PAG_LOG_INIT_CODE`](../zeos/include/mm_address.h#L22): User code virtual page number
- [`PAG_LOG_INIT_DATA`](../zeos/include/mm_address.h#L31): User data virtual page number  
- `TOTAL_PAGES`: Total number of pages in physical memory
- `NUM_PAG_KERNEL`, `NUM_PAG_CODE`, `NUM_PAG_DATA`: Section size definitions

**Address Translation Macros:**
- [`FRAME_INIT_CODE`](../zeos/include/mm_address.h#L25): Physical frame for user code initialization
- [`USER_FIRST_PAGE`](../zeos/include/mm_address.h#L49): First user virtual page number
- [`PH_PAGE()`](../zeos/include/mm_address.h#L52): Convert address to page number

**Paging Constants:**
- `PAGE_SIZE`: 4096 bytes (4KB pages)
- `ENTRY_DIR_PAGES`: Page directory entry index
- Page table entry flags and access permissions

### [`zeos_mm.h`](../zeos/include/zeos_mm.h)

Legacy memory management interface (deprecated for future multiprocess implementation).

**Current Functions:**
- [`monoprocess_init_addr_space()`](../zeos/include/zeos_mm.h#L18): Single address space initialization (declaration)
- Transitional interface that will be replaced with full process memory management

## Process Management & Scheduling

### [`sched.c`](../zeos/sched.c) and [`sched.h`](../zeos/include/sched.h)

Complete process management and scheduling system implementing round-robin scheduling with process hierarchies.

**Process Control Block (`task_struct`):**
```c
struct task_struct {
    int PID;                              // Process identifier
    page_table_entry *dir_pages_baseAddr; // Page directory base
    struct list_head list;                // Queue entry
    unsigned long kernel_esp;             // Kernel stack pointer
    int quantum;                          // Time slice
    enum state_t status;                  // Process state
    struct task_struct *parent;           // Parent process
    struct list_head children;            // Child process list
    struct list_head child_list;          // Entry in parent's children
    int pending_unblocks;                 // Synchronization counter
};
```

**Process States:**
- `ST_RUN`: Currently executing process
- `ST_READY`: Ready to run, waiting in ready queue
- `ST_BLOCKED`: Blocked waiting for event or synchronization

**Scheduling Queues:**
- `freequeue`: Available task_struct entries
- `readyqueue`: Processes ready for execution
- `blockedqueue`: Blocked processes waiting for events

**Global Scheduler Variables:**
- [`current_quantum`](../zeos/sched.c#L24): Current quantum ticks remaining for running process
- [`current_task`](../zeos/sched.c#L27): **Optimization**: Cached pointer to currently running task
- [`next_pid`](../zeos/sched.c#L21): Global PID counter (static, accessed via `get_next_pid()`)
- [`idle_task`](../zeos/sched.c#L29): Pointer to the idle task (PID 0)  
- [`init_task`](../zeos/sched.c#L30): Pointer to the initial user task (PID 1)

**Scheduling Algorithm:**
- **Round-robin**: Each process gets equal time quantum (100 ticks default)
- **Preemptive**: Timer interrupts trigger scheduler to switch contexts

**Process Management Macros:**
- [`KERNEL_ESP()`](../zeos/include/sched.h#L63): Calculate kernel stack pointer for task
- **Priority levels**: Idle task runs only when no other processes ready

**Key Functions:**
- [`list_head_to_task_struct()`](../zeos/sched.c#L33): Convert list_head pointer to containing task_struct
- [`get_DIR()`](../zeos/sched.c#L37): Get page directory for a task
- [`get_PT()`](../zeos/sched.c#L41): Get page table for a task
- [`allocate_DIR()`](../zeos/sched.c#L45): Allocate page directory for a task
- [`cpu_idle()`](../zeos/sched.c#L53): CPU idle function executed by the idle task
- [`init_idle()`](../zeos/sched.c#L60): Initialize the idle task
- [`init_task1()`](../zeos/sched.c#L80): Initialize the initial process task
- [`init_queues()`](../zeos/sched.c#L120): Initialize process queues
- [`init_sched()`](../zeos/sched.c#L130): Initialize scheduler data structures and queues
- [`current()`](../zeos/sched.c#L134): Get pointer to currently running process
- [`inner_task_switch()`](../zeos/sched.c#L141): Low-level task context switch implementation
- [`get_next_pid()`](../zeos/sched.c#L154): Get next available process ID
- [`get_quantum()`](../zeos/sched.c#L158): Get quantum value for a task
- [`set_quantum()`](../zeos/sched.c#L162): Set quantum value for a task
- [`update_sched_data_rr()`](../zeos/sched.c#L166): Update scheduling data for round-robin
- [`needs_sched_rr()`](../zeos/sched.c#L170): Check if scheduling is needed
- [`update_process_state_rr()`](../zeos/sched.c#L181): Update process state in round-robin scheduler
- [`sched_next_rr()`](../zeos/sched.c#L200): Round-robin process selection
- [`scheduler()`](../zeos/sched.c#L220): Main scheduling decision function
- [`printDebugInfoSched()`](../zeos/sched.c#L231): Print debug information before context switch

### [`list.c`](../zeos/list.c) and [`list.h`](../zeos/include/list.h)

Generic doubly-linked list implementation used throughout the kernel for queue management.

**List Operations:**
- [`INIT_LIST_HEAD()`](../zeos/list.c#L14): Initialize empty list head
- [`list_add()`](../zeos/list.c#L41): Insert element at head of list
- [`list_add_tail()`](../zeos/list.c#L53): Insert element at tail of list
- [`list_del()`](../zeos/list.c#L75): Remove element from list
- [`list_is_last()`](../zeos/list.c#L87): Check if element is last in list
- [`list_empty()`](../zeos/list.c#L96): Check if list is empty
- [`list_entry()`](../zeos/include/list.h#L111): Convert list_head pointer to containing structure
- [`list_for_each()`](../zeos/include/list.h#L119): Iterate over a list safely
- [`list_for_each_safe()`](../zeos/include/list.h#L127): Iterate over a list safe against removal
- [`list_first()`](../zeos/include/list.h#L134): Get first element of list

**Usage in Kernel:**
- Process scheduling queues (ready, blocked, free)
- Child process lists for process hierarchy
- General kernel data structure management

**Implementation Details:**
- Circular doubly-linked lists with sentinel head node
- Type-safe macros for structure member access
- Efficient O(1) insertion and deletion operations

## Interrupt Handling

### [`interrupt.c`](../zeos/interrupt.c) and [`interrupt.h`](../zeos/include/interrupt.h)

Interrupt handling system managing hardware interrupts, exceptions, and system calls.

**Interrupt Descriptor Table (IDT):**
- 256-entry table mapping interrupt vectors to handler functions
- Supports both interrupt gates (disable IF) and trap gates (preserve IF)
- Configurable privilege levels for user/kernel access control

**Hardware Interrupt Support:**
- **Keyboard interrupts**: IRQ 1 for keyboard input processing
- **Timer interrupts**: IRQ 0 for system clock and scheduling
- **PIC management**: 8259 Programmable Interrupt Controller configuration

**Exception Handling:**
- **Page fault handler**: Memory access violations and page-not-present
- **General protection faults**: Privilege level violations
- **Invalid opcode**: Illegal instruction handling
- **Debug exceptions**: Single-step and breakpoint support

**System Call Interface:**
- **SYSENTER mechanism**: Fast system call entry point
- **Parameter passing**: Uses registers EAX (syscall number), EBX, ECX, EDX (parameters)
- **Return values**: EAX contains return value or error code

**Key Functions:**
- [`setIdt()`](../zeos/interrupt.c#L79): Initialize complete IDT with all handlers
- [`setInterruptHandler()`](../zeos/interrupt.c#L37): Configure interrupt gate IDT entries
- [`setTrapHandler()`](../zeos/interrupt.c#L56): Configure trap gate IDT entries
- [`keyboard_routine()`](../zeos/interrupt.c#L98): C handler for keyboard interrupt processing
- [`clock_routine()`](../zeos/interrupt.c#L112): C handler for timer interrupt processing
- [`pageFault_routine()`](../zeos/interrupt.c#L118): C handler for page fault exception processing

**Global Variables:**
- `idt[IDT_ENTRIES]`: Interrupt Descriptor Table array
- `idtR`: IDT register structure for LIDT instruction
- `zeos_ticks`: Global system tick counter incremented by timer

## System Calls

### [`sys.c`](../zeos/sys.c) and [`sys.h`](../zeos/include/sys.h)

Kernel-side system call implementations providing process management, I/O, and system information services.

**Process Management System Calls:**
- **[`sys_fork()`](../zeos/sys.c#L44)**: Create new child process with copy-on-write semantics
- **[`sys_exit()`](../zeos/sys.c#L196)**: Terminate current process and clean up resources
- **[`sys_getpid()`](../zeos/sys.c#L36)**: Return current process identifier
- **[`sys_block()`](../zeos/sys.c#L237)**, **[`sys_unblock()`](../zeos/sys.c#L248)**: Process synchronization primitives

**I/O System Calls:**
- **[`sys_write()`](../zeos/sys.c#L166)**: Write data to file descriptor (currently console only)
- **File descriptor validation**: Ensures proper permissions and valid descriptors

**System Information:**
- **[`sys_gettime()`](../zeos/sys.c#L192)**: Return current system tick count
- **Error handling**: Proper errno setting for invalid operations

**Helper Functions:**
- [`check_fd()`](../zeos/sys.c#L26): Validate file descriptor and permissions
- [`sys_ni_syscall()`](../zeos/sys.c#L32): Not-implemented system call handler
- [`ret_from_fork()`](../zeos/sys.c#L40): Fork return path for child process

**Implementation Details:**
```c
int sys_fork() {
    // 1. Find free task_struct in task array
    // 2. Copy parent's page directory and user pages  
    // 3. Set up child's kernel stack with return context
    // 4. Configure child to return 0, parent returns child PID
    // 5. Add child to ready queue
}
```

**Stack Layout for Fork:**
The child process stack is carefully constructed to return 0 from fork():
```
[KERNEL_STACK_SIZE-19]: fake EBP (0)
[KERNEL_STACK_SIZE-18]: @ret_from_fork  
[KERNEL_STACK_SIZE-17]: @sysenter_return
[KERNEL_STACK_SIZE-5]:  Child's ESP0 for user mode
```

### [`sys_call_table.S`](../zeos/sys_call_table.S)

System call dispatch table mapping system call numbers to kernel functions.

**System Call Numbers:**
```assembly
sys_call_table:         # Defined at line 12 in sys_call_table.S
    .long sys_ni_syscall    # 0 - Not implemented
    .long sys_exit          # 1 - Process termination
    .long sys_fork          # 2 - Process creation  
    .long sys_ni_syscall    # 3 - Reserved
    .long sys_write         # 4 - Write to file descriptor
    .long sys_ni_syscall    # 5-9 - Reserved
    .long sys_gettime       # 10 - Get system time
    .long sys_ni_syscall    # 11 - Reserved
    .long sys_block         # 12 - Block current process
    .long sys_unblock       # 13 - Unblock process by PID
    .long sys_getpid        # 20 - Get process ID
```

**Global Symbol:**
- [`sys_call_table`](../zeos/sys_call_table.S#L12): System call dispatch table array

### [`sys_call_wrappers.S`](../zeos/sys_call_wrappers.S)

User-space system call wrappers providing C-callable interface to kernel services.

**Wrapper Implementation:**
- **Parameter passing**: Moves C function arguments to appropriate registers
- **SYSENTER invocation**: Uses fast system call mechanism  
- **Return handling**: Manages return values and errno setting
- **Error conversion**: Converts negative kernel return codes to errno

**User-Space System Call Wrappers:**
- [`write()`](../zeos/sys_call_wrappers.S#L14): Write data to file descriptor
- [`gettime()`](../zeos/sys_call_wrappers.S#L45): Get current system time
- [`getpid()`](../zeos/sys_call_wrappers.S#L70): Get process identifier
- [`fork()`](../zeos/sys_call_wrappers.S#L95): Create new child process
- [`exit()`](../zeos/sys_call_wrappers.S#L120): Terminate current process
- [`block()`](../zeos/sys_call_wrappers.S#L137): Block current process
- [`unblock()`](../zeos/sys_call_wrappers.S#L154): Unblock process by PID

**Generated Wrappers:**
```assembly
fork:
    movl $2, %eax          # System call number
    sysenter               # Fast system call
    cmpl $0, %eax          # Check for error
    jge fork_ok            # Jump if non-negative
    negl %eax              # Convert to positive
    movl %eax, errno       # Set errno
    movl $-1, %eax         # Return -1
fork_ok:
    ret
```

## Device Management & I/O

### [`io.c`](../zeos/io.c) and [`io.h`](../zeos/include/io.h)

Console I/O management and VGA text mode display driver.

**VGA Text Mode Driver:**
- **80x25 character display**: Standard VGA text mode support
- **Color attributes**: 16 foreground colors, 8 background colors  
- **Cursor management**: Hardware cursor positioning and visibility
- **Scrolling**: Automatic screen scrolling when buffer full

**Console Functions:**
- [`inb()`](../zeos/io.c#L37): Read byte from I/O port
- [`write_char_to_screen()`](../zeos/io.c#L44): Write character to specific screen position with color
- [`scroll_screen()`](../zeos/io.c#L51): Scroll screen content up by one line
- [`handle_newline()`](../zeos/io.c#L62): Process newline character and update cursor position
- [`printc()`](../zeos/io.c#L67): Print single character to console with color
- [`printc_xy()`](../zeos/io.c#L79): Print character at specific screen coordinates
- [`printk()`](../zeos/io.c#L90): Kernel printf-style formatted output (white color)
- [`printk_color()`](../zeos/io.c#L96): Kernel printf-style formatted output with specified color

**Color Management:**
```c
#define MAKE_COLOR(bg, fg) (((bg & 0xF) << 12) | ((fg & 0xF) << 8))
// Colors: BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GRAY, etc.
```

**Debug Output:**
- **Bochs E9 port**: Debug output via port 0xE9 for emulator logging
- **Conditional debugging**: Debug flags for different subsystems
- **Performance monitoring**: Timing and profiling output

### [`devices.c`](../zeos/devices.c) and [`devices.h`](../zeos/include/devices.h)

Device-dependent system call implementations and hardware abstraction layer.

**Device Abstractions:**
- **Console device**: Character output device abstraction
- **Keyboard device**: Input device handling (future implementation)
- **Timer device**: System clock and scheduling timer

**System Call Device Interface:**
- [`sys_write_console()`](../zeos/devices.c#L13): Console-specific write implementation
- Device file descriptor management
- Permission checking and validation

**Future Extensions:**
- Block device support for storage
- Network device interfaces  
- Additional character devices

### [`hardware.c`](../zeos/hardware.c) and [`hardware.h`](../zeos/include/hardware.h)

Low-level hardware abstraction and x86-specific operations.

**Processor Control:**
- **Interrupt control**: [`enable_int()`](../zeos/hardware.c#L95) for interrupt flag management
- **Descriptor table loading**: GDT and IDT register manipulation
- **Privilege level operations**: Ring 0/3 transitions and stack switching

**Memory Management Hardware:**
- **CR3 register**: Page directory base register management
- **TLB operations**: Translation lookaside buffer invalidation
- **Memory barrier**: Cache coherency and memory ordering

**Hardware Control Functions:**
- [`get_eflags()`](../zeos/hardware.c#L14): Read processor flags register
- [`set_eflags()`](../zeos/hardware.c#L24): Set processor flags register
- [`set_idt_reg()`](../zeos/hardware.c#L30): Load Interrupt Descriptor Table register
- [`set_gdt_reg()`](../zeos/hardware.c#L37): Load Global Descriptor Table register
- [`set_ldt_reg()`](../zeos/hardware.c#L44): Load Local Descriptor Table register
- [`set_task_reg()`](../zeos/hardware.c#L51): Load Task Register
- [`return_gate()`](../zeos/hardware.c#L58): Return from privileged mode to user mode
- [`enable_int()`](../zeos/hardware.c#L95): Enable hardware interrupts
- [`delay()`](../zeos/hardware.c#L106): Hardware-level timing delay

**I/O Port Operations:**
- **Port I/O**: inb(), outb() for hardware port access
- **PIC programming**: 8259 interrupt controller configuration
- **Timer programming**: 8253/8254 timer chip configuration

## User Space Components

### [`user.c`](../zeos/user.c)

Initial user process and system test execution environment.

**User Process Structure:**
- **Entry point**: [`main()`](../zeos/user.c#L13) function marked with special section attribute
- **Test execution**: Runs ZeOS test suite
- **System interaction**: Demonstrates system call usage and process behavior

**Test Integration:**
- Calls [`execute_zeos_tests()`](../zeos/zeos_test.c#L136) for system validation
- Process operation demonstrations (fork, block, unblock, exit)
- System call functionality verification

**Memory Layout:**
- **Text section**: Loaded at virtual address 0x114000  
- **Data section**: Positioned at virtual address 0x100000
- **Stack**: Grows downward from high virtual addresses
- **Heap**: Available for dynamic allocation (future implementation)

## Support Libraries & Utilities

### [`libc.c`](../zeos/libc.c) and [`libc.h`](../zeos/include/libc.h)

Standard C library implementation providing essential functions for user programs.

**String Operations:**
- [`strlen()`](../zeos/libc.c#L41): Calculate null-terminated string length
- [`itoa()`](../zeos/libc.c#L16): Convert integer to ASCII string representation
- [`perror()`](../zeos/libc.c#L51): Print error message to console

**System Call Wrappers:**
- [`write()`](../zeos/sys_call_wrappers.S#L14): User-space wrapper for sys_write system call
- [`fork()`](../zeos/sys_call_wrappers.S#L95): Process creation wrapper
- [`getpid()`](../zeos/sys_call_wrappers.S#L70): Process ID retrieval
- [`gettime()`](../zeos/sys_call_wrappers.S#L45): System time access
- [`block()`](../zeos/sys_call_wrappers.S#L137), [`unblock()`](../zeos/sys_call_wrappers.S#L154): Process synchronization

**Error Handling:**
- Global `errno` variable for error code storage
- Error code definitions matching POSIX standards
- Error message formatting and display

**Memory Operations:**
- Basic memory manipulation functions
- Buffer management utilities
- Safe memory access wrappers

### [`utils.c`](../zeos/utils.c) and [`utils.h`](../zeos/include/utils.h)

Kernel utility functions for memory operations and data handling.

**Memory Copy Operations:**
- [`copy_data()`](../zeos/utils.c#L14): Generic memory copying between kernel addresses
- [`copy_from_user()`](../zeos/utils.c#L30): Safe copying from user space to kernel space
- [`copy_to_user()`](../zeos/utils.c#L47): Safe copying from kernel space to user space

**User Space Validation:**
- [`access_ok()`](../zeos/utils.c#L72): Check if memory access is allowed for given operation type

**System Utilities:**
- [`get_ticks()`](../zeos/utils.c#L123): Retrieve current system tick count
- [`itoa_hex()`](../zeos/utils.c#L136): Convert integer to hexadecimal string representation
- [`print_splash_screen()`](../zeos/utils.c#L158): Display ZeOS startup splash screen with system information
- [`wait_ticks()`](../zeos/utils.c#L151): Implement busy-wait delay for specified number of timer ticks

**Utility Macros:**
- [`do_div()`](../zeos/utils.c#L106): 64-bit division macro for unsigned long values
- [`rdtsc()`](../zeos/utils.c#L121): Read Time Stamp Counter macro for high-precision timing

### [`types.h`](../zeos/include/types.h)

Fundamental data type definitions and system constants.

**Basic Types:**
```c
typedef unsigned char  Byte;     // 8-bit unsigned integer
typedef unsigned short Word;     // 16-bit unsigned integer  
typedef unsigned long  DWord;    // 32-bit unsigned integer
```

**Address Manipulation Macros:**
- [`highWord()`](../zeos/include/types.h#L24): Extract high 16 bits from 32-bit address
- [`lowWord()`](../zeos/include/types.h#L25): Extract low 16 bits from 32-bit address
- [`midByte()`](../zeos/include/types.h#L26): Extract middle byte from 32-bit address
- [`highByte()`](../zeos/include/types.h#L27): Extract highest byte from 32-bit address
- [`high4Bits()`](../zeos/include/types.h#L28): Extract high 4 bits from limit value

**System Structures:**
```c
typedef struct {
    Word limit;
    DWord base;
} __attribute__((packed)) Register;  // GDT/IDT register format

typedef struct {
    DWord entry;
} page_table_entry;                  // Page table entry format

typedef struct {
    // x86 Task State Segment structure
} TSS;
```

### [`asm.h`](../zeos/include/asm.h)

Assembly language integration macros and definitions.

**Function Declarations:**
```c
#define ENTRY(name) \
    .globl name; \
    name:
```

**Segment Definitions:**
- Kernel and user segment selector constants
- Privilege level definitions  
- Assembly-accessible constant definitions

### [`errno.h`](../zeos/include/errno.h)

Error code definitions matching POSIX standards.

**Standard Error Codes:**
```c
#define EPERM   1    // Operation not permitted
#define ENOENT  2    // No such file or directory
#define ESRCH   3    // No such process
#define EINTR   4    // Interrupted system call
#define EIO     5    // I/O error
#define EBADF   9    // Bad file descriptor
#define ENOMEM  12   // Out of memory
#define EACCES  13   // Permission denied
#define EFAULT  14   // Bad address
```

### [`debug.h`](../zeos/include/debug.h)

Debug configuration flags for controlling kernel debugging output.

**Debug Control Flags:**
```c
#define DEBUG_INFO_TASK_SWITCH    0  // Task switching debug messages
#define DEBUG_INFO_FORK           0  // Process fork debug messages  
#define DEBUG_INFO_EXIT           0  // Process exit debug messages
#define DEBUG_INFO_BLOCK          0  // Process blocking debug messages
#define DEBUG_INFO_UNBLOCK        0  // Process unblocking debug messages
```

**Integration:**
- Used in [`sched.c`](../zeos/sched.c) for task switching debug information
- Used in [`sys.c`](../zeos/sys.c) for system call debug tracing

## Testing Framework

### [`zeos_test.c`](../zeos/zeos_test.c) and [`zeos_test.h`](../zeos/include/zeos_test.h)

Test suite for validating ZeOS functionality and system call implementations.

**Test Categories:**
- **System call tests**: Write, gettime, getpid functionality
- **Process management tests**: Fork, exit, process hierarchy  
- **Synchronization tests**: Block, unblock operations
- **Memory tests**: Page fault handling, memory protection
- **Exception tests**: CPU exception handling verification

**Test Configuration:**
```c
#define WRITE_TEST           1  // Enable write system call tests
#define GETTIME_TEST         1  // Enable gettime system call tests  
#define GETPID_TEST          1  // Enable getpid system call tests
#define FORK_TEST            1  // Enable fork system call tests
#define EXIT_TEST            1  // Enable exit system call tests
#define BLOCK_UNBLOCK_TEST   1  // Enable synchronization tests
#define PAGEFAULT_TEST       0  // Disable page fault tests (optional)
```

**Test Utility Macros:**
- [`RESET_ERRNO()`](../zeos/include/zeos_test.h#L25): Reset errno variable to 0 for error testing

**Test Functions:**
- [`execute_zeos_tests()`](../zeos/zeos_test.c#L136): Main test execution coordinator
- [`test_basic_process_operations()`](../zeos/zeos_test.c#L23): Complete process lifecycle testing
- [`test_write_syscall()`](../zeos/zeos_test.c#L193): Write system call validation
- [`test_gettime_syscall()`](../zeos/zeos_test.c#L337): System time functionality testing
- [`test_getpid_syscall()`](../zeos/zeos_test.c#L366): Process ID retrieval testing
- [`test_fork_syscall()`](../zeos/zeos_test.c#L387): Process creation and hierarchy testing
- [`test_exit_syscall()`](../zeos/zeos_test.c#L580): Process termination testing
- [`test_block_unblock_syscalls()`](../zeos/zeos_test.c#L614): Synchronization primitives testing
- [`test_pagefault_exception()`](../zeos/zeos_test.c#L674): Memory exception handling testing
- [`print_test_header()`](../zeos/zeos_test.c#L716): Test section formatting
- [`print_test_result()`](../zeos/zeos_test.c#L724): Test result reporting
- [`print_final_summary()`](../zeos/zeos_test.c#L743): Complete test suite summary

**Test Infrastructure:**
- **Test statistics**: Automatic test counting and pass/fail tracking
- **Error reporting**: Detailed error messages with context information  
- **Buffer management**: Safe buffer operations for testing I/O
- **Process hierarchy validation**: Parent-child relationship verification

**Process Operation Testing:**
```c
void test_basic_process_operations(void) {
    // Creates process hierarchy: parent -> child1 -> child2
    // Tests fork, block, unblock, and exit operations
    // Validates proper process synchronization and cleanup
}
```

### Additional Support Files

**[`libzeos.a`](../zeos/libzeos.a)**
- Compiled kernel library containing core system functions
- Linked with kernel modules during system build process
- Contains compiled versions of scheduling, memory management, and I/O functions

**[`libauxjp.a`](../zeos/libauxjp.a)**  
- Auxiliary library provided with course materials
- Contains additional utility functions and support routines
- Used for extended functionality in laboratory exercises

### Memory Layout Summary

```
Virtual Address Space Layout:
0x00000000 ─────────────────────────────────────────────┐
            BIOS vectors, interrupt table, EBDA         │
0x00007C00 ── BIOS loads boot sector here               │
0x00090000 ── Bootloader relocates itself here          │
0x00010000 ── Kernel image loaded at 64 KiB             │
0x00100000 ── User data section at 1 MiB                │
0x00114000 ── User text section                         │
0xC0000000 ── Kernel virtual memory space               │
0xFFC00000 ── Page tables and directories               │
0xFFFFFFFF ── Top of virtual address space              │
```
