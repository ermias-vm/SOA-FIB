################################
############ ZeOS #############
################################
########## Makefile ############
################################

# package dev86 is required
AS86 = as86 -0 -a
LD86 = ld86 -0

# Parallel compilation configuration
CORES ?= 1
MAKEFLAGS += -j$(CORES)


# Enhanced compilation flags for detailed error/warning reporting  
HOSTCFLAGS = -Wall -Wstrict-prototypes -Wextra -g
HOSTCC = gcc
CC = gcc
AS = as --32
LD = ld
OBJCOPY = objcopy -O binary -R .note -R .comment -S

INCLUDEDIR = include


# Enhanced CFLAGS for detailed warnings and debugging
# CFLAGS = -m32 -O2 -g -fno-omit-frame-pointer -ffreestanding -Wall -Wextra -Wpedantic -Wformat=2 -Wunused -Wcast-align -Wwrite-strings -Wlogical-op -Wmissing-declarations -Wredundant-decls -Wshadow -I$(INCLUDEDIR) -fno-PIC
# CFLAGS = -m32 -O2 -g -fno-omit-frame-pointer -ffreestanding -Wall -Wextra -Wpedantic -Wformat=2 -Wcast-align -Wwrite-strings  -Wshadow -I$(INCLUDEDIR) -fno-PIC

# Original CFLAGS
CFLAGS = -m32 -O2  -g -fno-omit-frame-pointer -ffreestanding -Wall -Wextra  -I$(INCLUDEDIR) -fno-PIC
# Original No optimizations
#CFLAGS = -m32 -O0  -g -fno-omit-frame-pointer -ffreestanding -Wall -Wextra  -I$(INCLUDEDIR) -fno-PIC
ASMFLAGS = -I$(INCLUDEDIR)
LDFLAGS = -g -melf_i386 -z noexecstack

SYSOBJ = \
	interrupt.o \
	entry.o \
	sys_call_table.o \
	io.o \
	sched.o \
	sys.o \
	mm.o \
	devices.o \
	utils.o \
	hardware.o \
	list.o \
	kernel_asm.o \
	keyboard.o \
	screen.o \
	kernel_helpers.o \

LIBZEOS = -L . -l zeos

#add to USROBJ any object files required to complete the user program
USROBJ = \
	libc.o \
	sys_call_wrappers.o \
	zeos_test.o \
	project_test.o \
	screen_samples.o \
	game_map.o \
	game_entities.o \
	game_input.o \
	game_render.o \


################################################################################
#                           COMPILATION TARGETS                               #
################################################################################

all: zeos.bin

zeos.bin: bootsect system build user
	$(OBJCOPY) system system.out
	$(OBJCOPY) user user.out
	./build bootsect system.out user.out > zeos.bin

build: build.c
	$(HOSTCC) $(HOSTCFLAGS) -o $@ $<

bootsect: bootsect.o
	$(LD86) -s -o $@ $<

bootsect.o: bootsect.s
	$(AS86) -o $@ $<

bootsect.s: bootsect.S
	$(CPP) $(ASMFLAGS) -traditional $< -o $@

entry.s: entry.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(ASMFLAGS) -o $@ $<

sys_call_table.s: sys_call_table.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(ASMFLAGS) -o $@ $<

sys_call_wrappers.s: sys_call_wrappers.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(ASMFLAGS) -o $@ $<

kernel_asm.s: kernel_asm.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(ASMFLAGS) -o $@ $<

user.o:user.c $(INCLUDEDIR)/libc.h $(INCLUDEDIR)/zeos_test.h

interrupt.o:interrupt.c $(INCLUDEDIR)/entry.h $(INCLUDEDIR)/hardware.h $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/io.h $(INCLUDEDIR)/kernel_helpers.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h $(INCLUDEDIR)/utils.h $(INCLUDEDIR)/zeos_interrupt.h

io.o:io.c $(INCLUDEDIR)/io.h $(INCLUDEDIR)/types.h

sched.o:sched.c $(INCLUDEDIR)/debug.h $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/io.h $(INCLUDEDIR)/libc.h $(INCLUDEDIR)/mm.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/utils.h

libc.o:libc.c $(INCLUDEDIR)/libc.h

zeos_test.o:zeos_test.c $(INCLUDEDIR)/errno.h $(INCLUDEDIR)/libc.h $(INCLUDEDIR)/zeos_test.h

project_test.o:project_test.c $(INCLUDEDIR)/libc.h $(INCLUDEDIR)/project_test.h $(INCLUDEDIR)/screen_samples.h

screen_samples.o:screen_samples.c $(INCLUDEDIR)/screen_samples.h $(INCLUDEDIR)/io.h

mm.o:mm.c $(INCLUDEDIR)/hardware.h $(INCLUDEDIR)/mm.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h

sys.o:sys.c $(INCLUDEDIR)/debug.h $(INCLUDEDIR)/devices.h $(INCLUDEDIR)/errno.h $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/io.h $(INCLUDEDIR)/kernel_helpers.h $(INCLUDEDIR)/libc.h $(INCLUDEDIR)/mm.h $(INCLUDEDIR)/mm_address.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/screen.h $(INCLUDEDIR)/sys.h $(INCLUDEDIR)/utils.h

utils.o:utils.c $(INCLUDEDIR)/io.h $(INCLUDEDIR)/mm_address.h $(INCLUDEDIR)/types.h $(INCLUDEDIR)/utils.h

hardware.o:hardware.c $(INCLUDEDIR)/types.h

list.o:list.c $(INCLUDEDIR)/list.h

devices.o:devices.c $(INCLUDEDIR)/io.h $(INCLUDEDIR)/list.h $(INCLUDEDIR)/utils.h

system.o:system.c $(INCLUDEDIR)/hardware.h system.lds $(SYSOBJ) $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/system.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/mm.h $(INCLUDEDIR)/io.h $(INCLUDEDIR)/mm_address.h 

keyboard.o: keyboard.c $(INCLUDEDIR)/keyboard.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/mm.h $(INCLUDEDIR)/io.h $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/errno.h $(INCLUDEDIR)/utils.h $(INCLUDEDIR)/segment.h

screen.o: screen.c $(INCLUDEDIR)/screen.h $(INCLUDEDIR)/io.h $(INCLUDEDIR)/utils.h $(INCLUDEDIR)/types.h

kernel_helpers.o: kernel_helpers.c $(INCLUDEDIR)/kernel_helpers.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/mm.h $(INCLUDEDIR)/errno.h

# Headers that will be created
$(INCLUDEDIR)/game.h: $(INCLUDEDIR)/game_types.h $(INCLUDEDIR)/game_config.h

game.o: game.c $(INCLUDEDIR)/game.h $(INCLUDEDIR)/libc.h

system: system.o system.lds $(SYSOBJ)
	$(LD) $(LDFLAGS) -T system.lds -o $@ $< $(SYSOBJ) $(LIBZEOS)

user: user.o user.lds $(USROBJ) 
	$(LD) $(LDFLAGS) -T user.lds -o $@ $< $(USROBJ)


################################################################################
#                            UTILITY TARGETS                                  #
################################################################################

# Remove all generated files (object files, binaries, temporary files)
clean:
	rm -f *.o *.s bochsout.txt parport.out system.out system bootsect zeos.bin user user.out *~ build

# Clean everything, rebuild the system, and start debugging session
restart:
	make clean && make gdb

# Clean everything, rebuild the system, and emul
reemul:
	make clean && make emul

# Create a timestamped backup of the ZeOS project in compressed format
backup: clean
	@mkdir -p ../backup/zeos_versions
	@mkdir -p ../backup/project_versions
	@TIMESTAMP=$$(date +%Y%m%d_%H%M%S) && \
	BACKUP_NAME="project_v$$TIMESTAMP.tar.gz" && \
	if [ -f ../backup/project_versions/$$BACKUP_NAME ]; then \
		echo "Error: File $$BACKUP_NAME already exists!"; \
		exit 1; \
	fi && \
	echo "Creating Project backup: $$BACKUP_NAME" && \
	cd .. && tar -czvf backup/project_versions/$$BACKUP_NAME project/

# Write the ZeOS binary image to floppy disk device (legacy target, requires floppy drive)
disk: zeos.bin
	dd if=zeos.bin of=/dev/fd0

# Start Bochs emulator to run ZeOS in a virtual machine
emul: zeos.bin
	/opt/bochs/bin/bochs -q -f .bochsrc

# Start Bochs with GDB stub and connect GDB debugger for kernel debugging
gdb: zeos.bin
	bochs -q -f .bochsrc_gdb &
	gdb -x .gdbcmd system

# Start Bochs emulator with debugging interface (no GDB connection)
emuldbg: zeos.bin
	bochs_nogdb -q -f .bochsrc

# Format all C and header files using clang-format and ensure proper line endings
format:
	@echo "Formatting all source files..."
	@modified_files=""; \
	echo "  - Formatting C files with clang-format..."; \
	for file in $$(find . -maxdepth 1 -name "*.c" -type f); do \
		if [ -f "$$file" ]; then \
			cp "$$file" "$$file.bak"; \
			clang-format -i "$$file" 2>/dev/null || true; \
			if ! cmp -s "$$file" "$$file.bak"; then \
				modified_files="$$modified_files$$file (clang-format)\n"; \
			fi; \
			rm -f "$$file.bak"; \
		fi; \
	done; \
	echo "  - Formatting header files with clang-format..."; \
	for file in $$(find include/ -name "*.h" -type f); do \
		if [ -f "$$file" ]; then \
			cp "$$file" "$$file.bak"; \
			clang-format -i "$$file" 2>/dev/null || true; \
			if ! cmp -s "$$file" "$$file.bak"; then \
				modified_files="$$modified_files$$file (clang-format)\n"; \
			fi; \
			rm -f "$$file.bak"; \
		fi; \
	done; \
	echo "  - Ensuring all source files end with newline..."; \
	for file in $$(find . -maxdepth 1 \( -name "*.c" -o -name "*.S" \) -type f) $$(find include/ -name "*.h" -type f); do \
		if [ -s "$$file" ] && [ "$$(tail -c1 "$$file" | wc -l)" -eq 0 ]; then \
			modified_files="$$modified_files$$file (added final newline)\n"; \
			echo >> "$$file"; \
		fi; \
	done; \
	echo "Formatting complete!"; \
	echo ""; \
	echo "Modified files:"; \
	if [ -n "$$modified_files" ]; then \
		printf "$$modified_files" | sed 's/^/  - /'; \
	else \
		echo "  No files were modified."; \
	fi

# Generate Doxygen documentation
doxygen:
	@if command -v doxygen >/dev/null 2>&1; then \
        echo "Generating Doxygen documentation..."; \
        cd ../docs/doxygen/project && doxygen Doxyfile; \
        echo "Documentation generated in docs/doxygen/project/output/html/"; \
	else \
        echo "Error: Doxygen is not installed. Please install it with 'sudo apt install doxygen' (Ubuntu/Debian) or equivalent for your system."; \
    fi

doxygen-pdf:
	@if [ -d "../docs/doxygen/project/output/latex" ]; then \
        cd ../docs/doxygen/project/output/latex && make; \
        echo "PDF generated: ../docs/doxygen/project/output/latex/refman.pdf"; \
	else \
        echo "Run 'make doxygen' first to generate LaTeX files."; \
    fi

game_map.o: game_map.c $(INCLUDEDIR)/game_map.h $(INCLUDEDIR)/game_types.h $(INCLUDEDIR)/game_config.h $(INCLUDEDIR)/libc.h

game_entities.o: game_entities.c $(INCLUDEDIR)/game_entities.h $(INCLUDEDIR)/game_types.h $(INCLUDEDIR)/game_config.h $(INCLUDEDIR)/game_map.h $(INCLUDEDIR)/libc.h

game_input.o: game_input.c $(INCLUDEDIR)/game_input.h $(INCLUDEDIR)/game_types.h $(INCLUDEDIR)/game_config.h $(INCLUDEDIR)/libc.h

game_render.o: game_render.c $(INCLUDEDIR)/game_render.h $(INCLUDEDIR)/game_types.h $(INCLUDEDIR)/libc.h