# ZeOS Operating System

## Overview
ZeOS is a Unix-like operating system built for educational purposes. It implements process management, memory management, fast system calls via SYSENTER, interrupt handling, and includes a test framework. The system is designed for x86 architecture and runs on the Bochs emulator with debugging support.

## Project Structure

- **[`Makefile`](Makefile)** - Root Makefile that redirects commands to zeos/ directory
- **[`README.md`](README.md)** - Project documentation
- **[`ConfigZeos.md`](ConfigZeos.md)** - Configuration details for ZeOS setup
- **[`zeos/`](zeos/)** - Main ZeOS source code and build directory
- **[`docs/`](docs/)** - Documentation and guides
- **[`bochs-2.6.7/`](bochs-2.6.7/)** - Bochs emulator source and binaries
- **`backup/`** - Automatically created backup directory containing:
  - **`zeos_versions/`** - Timestamped .tar.gz backups of the ZeOS directory
  - **`project_versions/`** - Timestamped .tar.gz backups of the complete project


## Development Environment
- **Emulator**: Bochs 2.6.7 with internal debugger
- **Architecture**: x86 (32-bit)
- **Languages**: C, x86 Assembly
- **Build System**: GNU Make

## Course Information
**Subject**: Sistemas Operatius Avançats (SOA)  
**Institution**: Facultat d'Informàtica de Barcelona (FIB) - UPC  
**Academic Year**: 2025-2026

                           
## 👥 Credits 

- [Marc De Rialp](https://github.com/Derri725)
- [Ermias Valls](https://github.com/ermias-vm)