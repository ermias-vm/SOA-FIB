################################
##############ZeOS #############
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
LDFLAGS = -g -melf_i386

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

LIBZEOS = -L . -l zeos

#add to USROBJ any object files required to complete the user program
USROBJ = \
	libc.o \
	suma.o \
	sys_call_wrappers.o \
	zeos_test.o

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

user.o:user.c $(INCLUDEDIR)/libc.h

interrupt.o:interrupt.c $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h

io.o:io.c $(INCLUDEDIR)/io.h

sched.o:sched.c $(INCLUDEDIR)/sched.h

libc.o:libc.c $(INCLUDEDIR)/libc.h

zeos_test.o:zeos_test.c $(INCLUDEDIR)/libc.h

mm.o:mm.c $(INCLUDEDIR)/types.h $(INCLUDEDIR)/mm.h

sys.o:sys.c $(INCLUDEDIR)/devices.h

utils.o:utils.c $(INCLUDEDIR)/utils.h

system.o:system.c $(INCLUDEDIR)/hardware.h system.lds $(SYSOBJ) $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/system.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/mm.h $(INCLUDEDIR)/io.h $(INCLUDEDIR)/mm_address.h 

system: system.o system.lds $(SYSOBJ)
	$(LD) $(LDFLAGS) -T system.lds -o $@ $< $(SYSOBJ) $(LIBZEOS)

user: user.o user.lds $(USROBJ) 
	$(LD) $(LDFLAGS) -T user.lds -o $@ $< $(USROBJ)

suma.o: suma.S $(INCLUDEDIR)/asm.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.s bochsout.txt parport.out system.out system bootsect zeos.bin user user.out *~ build

restart:
	make clean && make all && make gdb

backup: clean
	@mkdir -p ../backup
	@TIMESTAMP=$$(date +%Y%m%d_%H%M%S) && \
	BACKUP_NAME="zeos_v$$TIMESTAMP.tar.gz" && \
	if [ -f ../backup/$$BACKUP_NAME ]; then \
		echo "Error: File $$BACKUP_NAME already exists!"; \
		exit 1; \
	fi && \
	echo "Creating backup: $$BACKUP_NAME" && \
	cd .. && tar -czvf backup/$$BACKUP_NAME zeos/

disk: zeos.bin
	dd if=zeos.bin of=/dev/fd0

emul: zeos.bin
	/opt/bochs/bin/bochs -q -f .bochsrc

gdb: zeos.bin
	bochs -q -f .bochsrc_gdb &
	gdb -x .gdbcmd system

emuldbg: zeos.bin
	bochs_nogdb -q -f .bochsrc

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

format-check:
	@echo "Checking formatting..."
	@find . -maxdepth 1 -name "*.c" -exec clang-format --dry-run --Werror {} \; 2>/dev/null || true
	@find include/ -name "*.h" -exec clang-format --dry-run --Werror {} \; 2>/dev/null || true
