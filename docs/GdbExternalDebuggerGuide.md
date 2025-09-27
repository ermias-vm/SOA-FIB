# GDB External Debugger Guide for ZeOS

## Overview
This guide explains how to use GDB to debug the ZeOS operating system. ZeOS uses Bochs emulator with GDB stub support for kernel debugging, allowing you to debug both kernel space and user space code.

## Getting Started with ZeOS

### Starting Debug Session
To start debugging ZeOS, use the provided Makefile target:
```bash
make gdb
```

This command does the following:
1. Starts Bochs with GDB stub enabled (port 1234)
2. Launches GDB with the system binary
3. Automatically connects to Bochs via `target remote localhost:1234`
4. Loads user space symbols at address 0x114000
5. Sets up initial display to show current instruction

### Basic GDB Commands for ZeOS
- `quit` or `q` - Exit GDB (will also stop Bochs)
- `help` - Display help information
- `continue` or `c` - Continue execution
- `Ctrl+C` - Break execution (interrupt running system)

### ZeOS Debug Environment
The debug setup automatically:
- Connects to Bochs GDB stub on localhost:1234
- Loads kernel symbols from `system` binary
- Loads user space symbols from `user` binary at 0x114000
- Displays current instruction pointer with `display/i $eip`

## Breakpoints and Execution Control in ZeOS

### Setting Breakpoints
- `break <function>` or `b <function>` - Set breakpoint at function
- `break <file:line>` - Set breakpoint at specific line  
- `break *<address>` - Set breakpoint at memory address
- `info breakpoints` - List all breakpoints
- `delete <number>` - Delete breakpoint by number

### ZeOS-Specific Breakpoint Examples
```bash
# Break at system initialization
(gdb) break main

# Break at interrupt handler
(gdb) break keyboard_handler

# Break at specific memory address (kernel entry point)
(gdb) break *0x100000
```

### Execution Control
- `continue` or `c` - Continue execution
- `step` or `s` - Step into functions (works across kernel/user boundary)
- `next` or `n` - Step over functions
- `finish` - Run until current function returns
- `stepi` or `si` - Step one assembly instruction
- `nexti` or `ni` - Step over one assembly instruction

## Examining Data in ZeOS

### Variables and Memory
- `print <variable>` or `p <variable>` - Print variable value
- `x/<format> <address>` - Examine memory
- `info registers` - Display register contents
- `backtrace` or `bt` - Show call stack

### ZeOS-Specific Data Examination Examples
```bash
# Examine current task structure
(gdb) print *current()
(gdb) print task[0]

# Examine process information
(gdb) print current()->PID
(gdb) print current()->dir_pages_baseAddr

# Examine kernel stack
(gdb) print-stack

# Examine GDT and system structures
(gdb) print gdt[0]
(gdb) print gdt[1]
(gdb) print tss

# Examine memory management structures
(gdb) x/10x phys_mem
(gdb) print free_frames

# Check interrupt vector
(gdb) x/10x idt
```

### Memory Formats for ZeOS
- `x/i` - Instructions (useful for examining code)
- `x/x` - Hexadecimal (default for addresses)
- `x/d` - Decimal (for counters, PIDs)
- `x/s` - String (for debugging text)
- `x/10i $eip` - Show next 10 instructions from current position

### Register Examination Examples
```bash
# Show all CPU registers
(gdb) info registers

# Show specific registers
(gdb) print $eip    # Current instruction pointer
(gdb) print $esp    # Stack pointer
(gdb) print $ebp    # Base pointer
(gdb) print $eax    # Accumulator
(gdb) print $cs     # Code segment
(gdb) print $ds     # Data segment
(gdb) print $ss     # Stack segment

# Show segment registers (important for OS debugging)
(gdb) info registers eflags
(gdb) print $eflags
```

## Advanced ZeOS Debugging Features

### Watchpoints for OS Debugging
- `watch <variable>` - Break when variable changes
- `rwatch <variable>` - Break when variable is read  
- `awatch <variable>` - Break when variable is accessed

### ZeOS Watchpoint Examples
```bash
# Watch for changes in current task
(gdb) watch current()->PID

# Watch for stack pointer changes
(gdb) watch $esp

# Watch for memory allocation changes
(gdb) watch free_frames

# Watch for specific memory address writes
(gdb) watch *0x114000

# Watch for changes in task array
(gdb) watch task[1].task.PID
```

### Memory Address Debugging
```bash
# Set breakpoint at user space entry
(gdb) break *0x114000

# Examine page tables
(gdb) x/256x 0x100000   # Examine page directory

# Examine kernel memory layout
(gdb) x/20i 0x100000    # Kernel code start
(gdb) x/10x 0x110000    # Kernel data area

# Examine user space
(gdb) x/10i 0x114000    # User code start
```

### Custom ZeOS GDB Commands
The `.gdbcmd` file defines these custom commands:

#### print-stack / pila
```bash
(gdb) print-stack   # or (gdb) pila
```
Shows the top values from the stack (ESP + offsets)

#### Symbol Loading
The setup automatically loads symbols:
- Kernel symbols from `system` binary
- User symbols from `user` binary at 0x114000

## Common ZeOS Debugging Workflow

### Initial Setup
1. Build ZeOS with debug symbols: `make clean && make`
2. Start debugging session: `make gdb`
3. GDB automatically connects to Bochs and loads symbols

### Typical Debugging Session
```bash
# 1. Set initial breakpoints
(gdb) break main
(gdb) break init_task1
(gdb) break task_switch

# 2. Continue to see system boot
(gdb) continue

# 3. When breakpoint hits, examine state
(gdb) info registers
(gdb) backtrace
(gdb) print current()->PID

# 4. Step through critical sections
(gdb) next
(gdb) step

# 5. Examine memory and data structures
(gdb) print task[0]
(gdb) print-stack
```

### Debugging Specific ZeOS Components

#### Process Management
```bash
# Debug task switching
(gdb) break task_switch
(gdb) continue
(gdb) print *current()
(gdb) info registers esp ebp

# Debug scheduler
(gdb) break sched_next_rr
(gdb) continue
(gdb) backtrace
```

#### Memory Management
```bash
# Debug page allocation
(gdb) break alloc_frame
(gdb) break free_frame
(gdb) continue
(gdb) print free_frames
(gdb) x/10x phys_mem
```

#### Interrupt Handling
```bash
# Debug system calls
(gdb) break system_call_handler
(gdb) continue
(gdb) print $eax    # System call number
(gdb) print $ebx    # First parameter
```

#### User Space Debugging
```bash
# Break at user space entry
(gdb) break *0x114000
(gdb) continue

# Step through user code
(gdb) stepi
(gdb) x/5i $eip    # Show next instructions
```
