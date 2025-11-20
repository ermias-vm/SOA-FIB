/**
 * @mainpage ZeOS Internal Architecture Documentation
 *
 * @section intro Introduction
 *
 * ZeOS is a 32-bit teaching operating system for the Intel 80386 architecture that demonstrates
 * fundamental OS concepts. This documentation provides detailed technical information about all
 * system components, their implementation, and interactions within the ZeOS kernel. The system is
 * designed to run on emulated x86 hardware using Bochs and provides a complete environment for
 * learning operating system internals.
 *
 * @section overview System Overview
 *
 * ZeOS implements a monolithic kernel architecture with the following key components:
 * - **Boot System**: Assembly bootstrap code handling real-to-protected mode transition
 * - **Memory Management**: Virtual memory system with paging and frame allocation
 * - **Process Management**: Round-robin scheduler with process hierarchies and synchronization
 * - **Interrupt Handling**: Complete IDT setup with hardware interrupt processing
 * - **System Calls**: Fast SYSENTER-based kernel interface with user-space wrappers
 * - **I/O System**: VGA text mode console driver and device abstraction layer
 *
 * @section boot_system Boot System
 *
 * @subsection bootsect Boot Sector (bootsect.S)
 *
 * The boot sector is the first code executed when ZeOS starts. This 512-byte assembly program
 * handles the critical transition from BIOS real mode to protected mode and loads the kernel.
 *
 * **Key Responsibilities:**
 * - Self-relocation from `0x07C00` to `0x90000` to make room for kernel loading
 * - Kernel loading using BIOS interrupt `0x13` to read sectors from disk
 * - A20 line activation to enable access to memory above 1MB
 * - GDT setup with kernel code/data segments and user segments
 * - Protected mode transition switching CPU from 16-bit real mode to 32-bit protected mode
 * - Control transfer to kernel entry point at `0x10010`
 *
 * **Technical Implementation:**
 * ```cpp
 * // Boot sector relocates itself and loads kernel
 * start:
 *     movw $0x9000, %ax
 *     movw %ax, %es
 *     movw $0x100, %cx      // 256 words = 512 bytes
 *     xorw %si, %si
 *     xorw %di, %di
 *     rep movsw
 *
 *     ljmp $0x9000, $go     // Jump to relocated code
 * ```
 *
 * @subsection build_system Build System
 *
 * @subsubsection makefile Build Configuration (Makefile)
 *
 * The build system orchestrates compilation, linking, and emulation through GNU Make.
 *
 * **Key Features:**
 * - Parallel compilation with automatic CPU core detection
 * - Cross-compilation for i386 architecture with `-m32` flag
 * - Dependency tracking for header files and automatic rebuilding
 * - Multiple build targets: `all`, `emul`, `emuldbg`, `gdb`, `clean`
 *
 * **Compilation Flags:**
 * - `-m32`: Forces 32-bit compilation for x86 compatibility
 * - `-ffreestanding`: No standard library dependencies
 * - `-fno-PIC`: Disables position-independent code generation
 * - `-Wall -Wextra`: Comprehensive compiler warnings
 *
 * @subsection core_components Core System Components
 *
 * @subsubsection system_core Kernel Entry (system.c)
 *
 * The kernel's main entry point handles complete system initialization sequence:
 * 1. Segment register configuration for kernel operation
 * 2. Hardware initialization (GDT, IDT, TSS setup)
 * 3. Memory management system initialization with paging
 * 4. Process scheduler and task structures setup
 * 5. User program copying to target memory locations
 * 6. Privilege level transition to user mode execution
 *
 * **Global Variables:**
 * - `usr_main`: Pointer to user program entry point at `0x114000`
 * - `p_sys_size`, `p_usr_size`: Embedded size information pointers
 * - `tss`: Task State Segment for hardware task switching
 * - `gdt`: Global Descriptor Table for memory segmentation
 *
 * @section memory_management Memory Management
 *
 * @subsection paging_system Virtual Memory System (mm.c)
 *
 * ZeOS implements a complete virtual memory system with paging support.
 *
 * **Physical Memory Management:**
 * - Frame allocation using bitmap tracking of 4KB physical memory frames
 * - Free frame list maintenance for available physical memory pages
 * - Memory protection enforcing kernel/user space separation
 *
 * **Virtual Memory Implementation:**
 * - Per-task page directories for memory space isolation
 * - Page tables mapping virtual addresses to physical frame numbers
 * - Memory mapping for code, data, and stack segments
 * - Framework for copy-on-write memory sharing
 *
 * **Key Data Structures:**
 * ```cpp
 * extern Byte phys_mem[TOTAL_PAGES];                    // Physical frame bitmap
 * extern page_table_entry dir_pages[NR_TASKS][TOTAL_PAGES];    // Task page directories
 * extern page_table_entry pagusr_table[NR_TASKS][TOTAL_PAGES]; // Task page tables
 * ```
 *
 * **Page Table Entry Structure:**
 * ```cpp
 * typedef struct {
 *     unsigned frame:20;      // Physical frame number (20 bits)
 *     unsigned reserved:3;    // Reserved bits
 *     unsigned accessed:1;    // Accessed flag
 *     unsigned dirty:1;       // Dirty flag
 *     unsigned r_w:1;         // Read/write permission
 *     unsigned user:1;        // User/supervisor level
 *     unsigned present:1;     // Present in memory
 * } page_table_entry;
 * ```
 *
 * @section process_management Process Management & Scheduling
 *
 * @subsection scheduler Round-Robin Scheduler (sched.c)
 *
 * ZeOS implements a complete process management system with round-robin scheduling.
 *
 * **Process Control Block:**
 * ```cpp
 * struct task_struct {
 *     int PID;                              // Process identifier
 *     page_table_entry *dir_pages_baseAddr; // Page directory base
 *     struct list_head list;                // Queue entry
 *     unsigned long kernel_esp;             // Kernel stack pointer
 *     int quantum;                          // Time slice (100 ticks default)
 *     enum state_t status;                  // Process state
 *     struct task_struct *parent;           // Parent process
 *     struct list_head children;            // Child process list
 *     struct list_head child_list;          // Entry in parent's children
 *     int pending_unblocks;                 // Synchronization counter
 * };
 * ```
 *
 * **Process States:**
 * - `ST_RUN`: Currently executing process
 * - `ST_READY`: Ready to run, waiting in ready queue
 * - `ST_BLOCKED`: Blocked waiting for event or synchronization
 *
 * **Scheduling Queues:**
 * - `freequeue`: Available task_struct entries
 * - `readyqueue`: Processes ready for execution
 * - `blockedqueue`: Blocked processes waiting for events
 *
 * **Scheduling Algorithm:**
 * - Round-robin with equal time quantum for all processes
 * - Preemptive scheduling triggered by timer interrupts
 * - Priority-based idle task execution when no other processes ready
 *
 * @subsection process_creation Process Creation (sys.c)
 *
 * Process creation through `sys_fork()` implements copy-on-write semantics:
 * 1. Allocate free task_struct from task array
 * 2. Copy parent's page directory and user pages
 * 3. Set up child's kernel stack with return context
 * 4. Configure child to return 0, parent returns child PID
 * 5. Add child to ready queue for execution
 *
 * **Fork Implementation:**
 * ```cpp
 * int sys_fork() {
 *     // Find free task_struct in task array
 *     // Copy parent's page directory and user pages
 *     // Set up child's kernel stack with return context
 *     // Configure child to return 0, parent returns child PID
 *     // Add child to ready queue
 * }
 * ```
 *
 * @section interrupt_handling Interrupt Handling
 *
 * @subsection idt_setup Interrupt Descriptor Table (interrupt.c)
 *
 * ZeOS implements a complete interrupt handling system with IDT management.
 *
 * **Interrupt Descriptor Table:**
 * - 256-entry table mapping interrupt vectors to handler functions
 * - Support for interrupt gates (disable IF) and trap gates (preserve IF)
 * - Configurable privilege levels for user/kernel access control
 *
 * **Hardware Interrupt Support:**
 * - **Timer interrupts** (IRQ 0): System clock and process scheduling
 * - **Keyboard interrupts** (IRQ 1): Input device processing
 * - **PIC management**: 8259 Programmable Interrupt Controller configuration
 *
 * **Exception Handling:**
 * - Page fault handler for memory access violations
 * - General protection faults for privilege violations
 * - Invalid opcode and debug exception handling
 *
 * @subsection system_calls System Call Interface
 *
 * ZeOS provides fast system call entry using Intel's SYSENTER/SYSEXIT mechanism.
 *
 * **System Call Numbers (sys_call_table.S):**
 * ```cpp
 * sys_call_table:
 *     .long sys_ni_syscall    // 0 - Not implemented
 *     .long sys_exit          // 1 - Process termination
 *     .long sys_fork          // 2 - Process creation
 *     .long sys_write         // 4 - Write to file descriptor
 *     .long sys_gettime       // 10 - Get system time
 *     .long sys_block         // 12 - Block current process
 *     .long sys_unblock       // 13 - Unblock process by PID
 *     .long sys_getpid        // 20 - Get process ID
 * ```
 *
 * **User-Space Wrappers (sys_call_wrappers.S):**
 * - Parameter passing through registers (EAX, EBX, ECX, EDX)
 * - SYSENTER invocation for fast kernel entry
 * - Return value and errno handling
 *
 * @section io_system I/O System
 *
 * @subsection vga_driver VGA Text Mode Driver (io.c)
 *
 * ZeOS implements a complete VGA text mode console driver.
 *
 * **Display Features:**
 * - 80x25 character display with color attributes
 * - 16 foreground colors and 8 background colors
 * - Hardware cursor positioning and visibility control
 * - Automatic screen scrolling when buffer full
 *
 * **Color Management:**
 * ```cpp
 * #define MAKE_COLOR(bg, fg) (((bg & 0xF) << 12) | ((fg & 0xF) << 8))
 * // Colors: BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHT_GRAY, etc.
 * ```
 *
 * **Console Operations:**
 * - `printk()`: Kernel printf-style formatted output
 * - `printk_color()`: Colored kernel output with specified attributes
 * - `write_char_to_screen()`: Direct character placement with color
 * - `scroll_screen()`: Screen content scrolling and line management
 *
 * @section user_space User Space Components
 *
 * @subsection user_program Initial User Process (user.c)
 *
 * The initial user process serves as the system test execution environment.
 *
 * **Memory Layout:**
 * - **Text section**: Loaded at virtual address `0x114000`
 * - **Data section**: Positioned at virtual address `0x100000`
 * - **Stack**: Grows downward from high virtual addresses
 * - **Entry point**: `main()` function with special section attribute
 *
 * **Test Integration:**
 * - Executes complete ZeOS test suite via `execute_zeos_tests()`
 * - Demonstrates system call usage and process behavior
 * - Validates process operations (fork, block, unblock, exit)
 *
 * @section support_libraries Support Libraries & Utilities
 *
 * @subsection libc Standard C Library (libc.c)
 *
 * Essential C library functions for user programs.
 *
 * **String Operations:**
 * - `strlen()`: Null-terminated string length calculation
 * - `itoa()`: Integer to ASCII string conversion
 * - `perror()`: Error message printing to console
 *
 * **System Call Wrappers:**
 * - `write()`: File descriptor write operations
 * - `fork()`: Process creation interface
 * - `getpid()`: Process ID retrieval
 * - `gettime()`: System time access
 * - `block()`, `unblock()`: Process synchronization primitives
 *
 * @subsection utils Kernel Utilities (utils.c)
 *
 * Kernel utility functions for memory operations and system services.
 *
 * **Memory Operations:**
 * - `copy_data()`: Generic memory copying between kernel addresses
 * - `copy_from_user()`: Safe copying from user space to kernel space
 * - `copy_to_user()`: Safe copying from kernel space to user space
 * - `access_ok()`: User space memory access validation
 *
 * **System Utilities:**
 * - `get_ticks()`: Current system tick count retrieval
 * - `print_splash_screen()`: ZeOS startup information display
 * - `wait_ticks()`: Busy-wait delay implementation
 *
 * @section testing_framework Testing Framework
 *
 * @subsection zeos_tests Test Suite (zeos_test.c)
 *
 * Comprehensive test suite validating ZeOS functionality.
 *
 * **Test Categories:**
 * - **System call tests**: Write, gettime, getpid functionality validation
 * - **Process management tests**: Fork, exit, process hierarchy verification
 * - **Synchronization tests**: Block/unblock operations testing
 * - **Memory tests**: Page fault handling and memory protection
 * - **Exception tests**: CPU exception handling verification
 *
 * **Test Infrastructure:**
 * - Automatic test counting and pass/fail tracking
 * - Detailed error reporting with context information
 * - Safe buffer operations for I/O testing
 * - Process hierarchy validation and cleanup verification
 *
 * @section memory_layout Memory Layout
 *
 * **Virtual Address Space:**
 * ```
 * 0x00000000 ─────────────────────────────────────────────┐
 *             BIOS vectors, interrupt table, EBDA         │
 * 0x00007C00 ── BIOS loads boot sector here               │
 * 0x00090000 ── Bootloader relocates itself here          │
 * 0x00010000 ── Kernel image loaded at 64 KiB             │
 * 0x00100000 ── User data section at 1 MiB                │
 * 0x00114000 ── User text section                         │
 * 0xC0000000 ── Kernel virtual memory space               │
 * 0xFFC00000 ── Page tables and directories               │
 * 0xFFFFFFFF ── Top of virtual address space              │
 * ```
 *
 * **Physical Memory Layout:**
 * ```
 * 0x00000000 ── BIOS and real mode code
 * 0x00010000 ── Kernel code and data (64KB)
 * 0x00100000 ── User data segment (1MB)
 * 0x00200000 ── Page tables and directories
 * 0x01000000 ── End of physical memory (16MB)
 * ```
 *
 * @section build_and_run Building and Running
 *
 * @subsection prerequisites Prerequisites
 * - GCC cross-compiler for i386 architecture
 * - GNU Make build system
 * - Bochs x86 emulator
 * - GDB debugger (optional, for debugging)
 *
 * @subsection build_steps Build Process
 * ```bash
 * # Build complete system
 * make all
 *
 * # Run in Bochs emulator
 * make emul
 *
 * # Run with debugging support
 * make emuldbg
 *
 * # Run with GDB remote debugging
 * make gdb
 *
 * # Clean build artifacts
 * make clean
 * ```
 *
 * @subsection debugging Debugging Support
 * - **Bochs internal debugger**: `make emuldbg` for built-in debugging
 * - **GDB remote debugging**: `make gdb` with `.gdbcmd` configuration
 * - **Debug output**: Port 0xE9 output for emulator logging
 * - **Conditional debugging**: Configurable debug flags in `debug.h`
 *
 * @section documentation_structure Documentation Structure
 *
 * This Doxygen documentation provides:
 * - **Function interfaces**: Detailed function signatures and parameters
 * - **Data structures**: Complete structure definitions and field descriptions
 * - **System architecture**: Design decisions and component interactions
 * - **Code organization**: Module structure and file relationships
 * - **Implementation details**: Technical implementation specifics
 *
 * @section authors Authors
 *
 * Developed as part of the Operating Systems course at FIB (Facultat d'Informàtica de Barcelona).
 * Implementation demonstrates fundamental OS concepts including process scheduling, memory
 * management, interrupt handling, and system call interfaces.
 *
 * @section license License
 *
 * This project is developed for educational purposes as part of the Operating Systems curriculum.
 * The implementation serves as a learning tool for understanding operating system internals and
 * low-level system programming concepts.
 */
