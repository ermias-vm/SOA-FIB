################################
##############ZeOS #############
################################
########## Makefile ############
################################

# package dev86 is required
AS86	= as86 -0 -a
LD86	= ld86 -0

HOSTCFLAGS = -Wall -Wstrict-prototypes -g
HOSTCC 	= gcc
CC      = gcc -m32
AS      = as --32
LD      = ld -melf_i386
OBJCOPY = objcopy -O binary -R .note -R .comment -S

INCLUDEDIR = include

# Define here flags to compile the tests if needed
JP =

CFLAGS = -O2  -g $(JP) -fno-omit-frame-pointer -ffreestanding -Wall -I$(INCLUDEDIR)
ASMFLAGS = -I$(INCLUDEDIR)
SYSLDFLAGS = -T system.lds
USRLDFLAGS = -T user.lds
LINKFLAGS = -g

SYSOBJ = interrupt.o entry.o sys_call_table.o io.o sched.o sys.o mm.o devices.o utils.o hardware.o list.o p_stats.o kernel-utils.o

LIBZEOS = -L . -l zeos -l auxjp

#add to USROBJ the object files required to complete the user program
USROBJ = libc.o user-utils.o # libjp.a

all:zeos.bin

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

bootsect.s: bootsect.S Makefile
	$(CPP) $(ASMFLAGS) -traditional $< -o $@

entry.s: entry.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(ASMFLAGS) -o $@ $<

user-utils.s: user-utils.S $(INCLUDEDIR)/asm.h
	$(CPP) $(ASMFLAGS) -o $@ $<

kernel-utils.s: kernel-utils.S $(INCLUDEDIR)/asm.h
	$(CPP) $(ASMFLAGS) -o $@ $<

sys_call_table.s: sys_call_table.S $(INCLUDEDIR)/asm.h $(INCLUDEDIR)/segment.h
	$(CPP) $(ASMFLAGS) -o $@ $<

user.o:user.c $(INCLUDEDIR)/libc.h

interrupt.o:interrupt.c $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h

io.o:io.c $(INCLUDEDIR)/io.h

sched.o:sched.c $(INCLUDEDIR)/sched.h

libc.o:libc.c $(INCLUDEDIR)/libc.h

mm.o:mm.c $(INCLUDEDIR)/types.h $(INCLUDEDIR)/mm.h

sys.o:sys.c $(INCLUDEDIR)/devices.h

utils.o:utils.c $(INCLUDEDIR)/utils.h

p_stats.o:p_stats.c $(INCLUDEDIR)/utils.h

system.o:system.c $(INCLUDEDIR)/hardware.h system.lds $(SYSOBJ) $(INCLUDEDIR)/segment.h $(INCLUDEDIR)/types.h $(INCLUDEDIR)/interrupt.h $(INCLUDEDIR)/system.h $(INCLUDEDIR)/sched.h $(INCLUDEDIR)/mm.h $(INCLUDEDIR)/io.h $(INCLUDEDIR)/mm_address.h 


system: system.o system.lds $(SYSOBJ)
	$(LD) $(LINKFLAGS) $(SYSLDFLAGS) -o $@ $< $(SYSOBJ) $(LIBZEOS) 

user: user.o user.lds $(USROBJ) 
	$(LD) $(LINKFLAGS) $(USRLDFLAGS) -o $@ $< $(USROBJ)


clean:
	rm -f *.o *.s bochsout.txt parport.out system.out system bootsect zeos.bin user user.out *~ build 

disk: zeos.bin
	dd if=zeos.bin of=/dev/fd0

emul: zeos.bin
	bochs -q -f .bochsrc

gdb: zeos.bin
	bochs -q -f .bochsrc_gdb &
	gdb -x .gdbcmd system

emuldbg: zeos.bin
	bochs_nogdb -q -f .bochsrc

reemul:
	make clean && make emul

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
