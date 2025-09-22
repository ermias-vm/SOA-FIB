# Bochs Internal Debugger Guide - ZeOS

Essential commands for debugging ZeOS using `make emuldbg`.

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
- `d n` - Delete breakpoint n
- `blist` - Show breakpoint status

### Information
- `r` / `info registers` - Show CPU registers
- `sreg` - Show segment registers
- `creg` - Show control registers
- `info gdt` - Show GDT table
- `info idt` - Show IDT table
- `info pic` - Show PIC status
- `info cpu` - Show CPU model
- `page addr` - Show page translation

### Memory Examination
- `x /nuf addr` - Examine memory (`x /10wx 0x7c00`)
- `xp /nuf addr` - Examine physical memory (`xp /4bx 0x100000`)
- `u addr` - Disassemble at address (`u 0x7c00`)
- `u /n addr` - Disassemble n instructions (`u /10 0x7c00`)
- `print expr` - Print expression (`print $eax`)
- `calc expr` - Calculate expression (`calc 0x7c00+16`)

**Memory format**: `n`=count, `u`=size(`b`yte/`w`ord/`g`iant), `f`=format(`x`=hex/`d`=dec/`c`=char/`s`=string)

### Memory Modification
- `set $reg = val` - Set register value (`set $eax = 0x1234`)
- `setpmem addr val` - Set physical memory (`setpmem 0x7c00 0x90`)
- `writemem file addr len` - Write memory to file

### Settings
- `set $disassemble_size = n` - Set default disasm size
- `set $auto_disassemble = 1` - Auto-disassemble on break

## Common ZeOS Debugging Workflow

```bash
# Boot debugging
<bochs:1> b 0x7c00          # Break at boot sector
<bochs:1> c                 # Continue to boot
<bochs:1> u /10 0x7c00      # Disassemble boot code
<bochs:1> s                 # Step through

# System call debugging
<bochs:1> b 0x80            # Break on interrupt 0x80
<bochs:1> r                 # Check registers
<bochs:1> x /4wx $esp       # Examine stack

# Memory management
<bochs:1> info gdt          # Check GDT
<bochs:1> sreg              # Check segments
<bochs:1> page 0x401000     # Check page translation

# General examination
<bochs:1> x /16wx 0x100000  # Examine kernel memory
<bochs:1> u $eip            # Disassemble current instruction
```

## Quick Reference

```
EXECUTION:    c  s  p  q
BREAKPOINTS:  b addr  lb  d n
MEMORY:       x /nuf addr  u addr  calc expr
REGISTERS:    r  sreg  creg
SYSTEM:       info gdt  info idt  page addr
MODIFY:       set $reg=val  setpmem addr val
```