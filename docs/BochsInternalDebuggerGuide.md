# Bochs Internal Debugger Guide - ZeOS

Essential commands for debugging ZeOS using `make emuldbg`.

**⚠️ Important**: Bochs debugger syntax differs from GDB:
- No `$` prefix for registers: use `eax` not `$eax`
- Different memory format syntax
- Built-in system information commands

## Quick Start

```bash
make emuldbg
```

Debugger prompt: `<bochs:1>`

## Commands

### Execution Control
- `c` - Continue execution
- `s [n]` - Step n instructions (default: 1)
- `p` - Step over calls
- `q` / `quit` - Quit debugger
- `Ctrl+C` - Break execution

### Breakpoints
- `b addr` - Set breakpoint at address (`b 0x7c00`)
- `vb seg:off` - Set virtual breakpoint (`vb 0x10:0x1000`)
- `lb` - List all breakpoints  
- `d n` - Delete breakpoint number n (`d 1`)
- `bpe n` / `bpd n` - Enable/disable breakpoint n

### Information
- `r` / `regs` - Show CPU registers
- `sreg` - Show segment registers  
- `creg` - Show control registers (CR0, CR2, CR3, CR4)
- `dreg` - Show debug registers
- `info gdt [n]` - Show GDT table (optionally entry n)
- `info idt [n]` - Show IDT table (optionally entry n)  
- `info tss` - Show TSS information
- `info cpu` - Show CPU model and features
- `page addr` - Show page translation for linear address

### Memory Examination
- `x /nuf addr` - Examine memory (`x /10wx 0x7c00`)
- `xp /nuf addr` - Examine physical memory (`xp /4bx 0x100000`)
- `u addr [addr2]` - Disassemble at address (`u 0x7c00`)
- `u /n addr` - Disassemble n instructions (`u /10 0x7c00`)  
- `dump_cpu` - Show CPU state
- `calc expr` - Calculate expression (`calc 0x7c00+16`)
- `?` - Calculator shorthand (`? 0x7c00+16`)

**Memory format**: `n`=count, `u`=size(`b`yte/`w`ord/`g`iant), `f`=format(`x`=hex/`d`=dec/`c`=char/`s`=string)

### Memory Modification
- `set $reg = val` - Set register value (`set $eax = 0x1234`)
- `setpmem addr size val` - Set physical memory (`setpmem 0x7c00 1 0x90`)
- `crc addr len` - Calculate CRC of memory region
- `restore file addr` - Load file to memory address
- `save file addr len` - Save memory region to file

### Settings
- `set $disassemble_size = n` - Set default disasm size
- `set $auto_disassemble = 1` - Auto-disassemble on break
- `trace on/off` - Enable/disable instruction tracing
- `record on/off` - Enable/disable execution recording

## Common ZeOS Debugging Workflow

```bash
# Boot debugging
<bochs:1> b 0x7c00          # Break at boot sector
<bochs:1> c                 # Continue to boot
<bochs:1> u /10 0x7c00      # Disassemble boot code
<bochs:1> s                 # Step through

# System call debugging (ZeOS uses INT 0x80)
<bochs:1> b system_call_handler  # Break at syscall handler
<bochs:1> r                 # Check registers
<bochs:1> x /4wx esp        # Examine stack (no $ in Bochs)

# Memory management
<bochs:1> info gdt          # Check GDT
<bochs:1> sreg              # Check segments
<bochs:1> page 0x401000     # Check page translation

# General examination
<bochs:1> x /16wx 0x100000  # Examine kernel memory
<bochs:1> u eip             # Disassemble current instruction (no $ in Bochs)
<bochs:1> dump_cpu          # Show complete CPU state
```

## Quick Reference

```
EXECUTION:    c  s  p  q
BREAKPOINTS:  b addr  lb  d n  bpe n  bpd n
MEMORY:       x /nuf addr  u addr  calc expr  ? expr
REGISTERS:    r  sreg  creg  dreg  dump_cpu
SYSTEM:       info gdt  info idt  info tss  page addr
MODIFY:       set $reg=val  setpmem addr size val
TRACE:        trace on/off  record on/off
```