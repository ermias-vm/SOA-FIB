# ZeOS - Advanced Operating Systems (SOA) - FIB

Advanced Operating Systems project for developing a custom kernel (ZeOS), implemented in C and x86 assembly.

## Project Overview
This project involves creating a basic operating system kernel with:
- Process management and scheduling
- Memory management 
- System calls implementation
- Interrupt handling
- Hardware abstraction layer

## Project Structure
- `ConfigZeos.md`:  **[Bochs Configuration Guide](ConfigZeos.md)**
- `zeos/`: Contains the ZeOS kernel source code and build files
  - `include/`: Header files and system definitions
  - `Makefile`: Build system configuration
  - Various `.c` and `.S` files: Kernel implementation
- `docs/`: Project documentation and specifications
  - `Zeos.pdf`: - [ZeOS Project Specification](docs/Zeos.pdf)

- `bochs-2.6.7/`: Bochs emulator source code (for compilation)




## Development Environment
- **Emulator**: Bochs 2.6.7 with internal debugger
- **Architecture**: x86 (32-bit)
- **Languages**: C, x86 Assembly
- **Build System**: GNU Make


## Course Information
**Subject**: Sistemas Operatius Avançats (SOA)  
**Institution**: Facultat d'Informàtica de Barcelona (FIB) - UPC  
**Academic Year**: 2025-2026