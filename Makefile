# Root Makefile - Redirects all commands to specific project directory

# Specify the project directory (zeos or project)
PROJECT_DIR ?=zeos


# Directory that contains many subfolders
EXAM_DIR := lab_exams

# Default target
all:
	$(MAKE) -C $(PROJECT_DIR)

# Common ZeOS targets
clean:
	$(MAKE) -C $(PROJECT_DIR) clean

restart:
	$(MAKE) -C $(PROJECT_DIR) restart

reemul:
	$(MAKE) -C $(PROJECT_DIR) reemul

emul:
	$(MAKE) -C $(PROJECT_DIR) emul

gdb:
	$(MAKE) -C $(PROJECT_DIR) gdb

emuldbg:
	$(MAKE) -C $(PROJECT_DIR) emuldbg

format:
	$(MAKE) -C $(PROJECT_DIR) format

format-check:
	$(MAKE) -C $(PROJECT_DIR) format-check

backup:
	$(MAKE) -C $(PROJECT_DIR) backup

disk:
	$(MAKE) -C $(PROJECT_DIR) disk

doxygen:
	$(MAKE) -C $(PROJECT_DIR) doxygen

global_clean:
	@echo "=== Global clean ==="

	# Clean zeos and zeosBackup normally
	$(MAKE) -C zeos clean
	$(MAKE) -C zeosBackup clean

	# Recursively remove generated files inside lab_exams
	@echo "Cleaning lab_exams recursively..."
	@find lab_exams -type f \( \
		-name "*.o" -o \
		-name "*.s" -o \
		-name "*~" -o \
		-name "bochsout.txt" -o \
		-name "parport.out" -o \
		-name "system.out" -o \
		-name "system" -o \
		-name "user" -o \
		-name "user.out" -o \
		-name "zeos.bin" \
	\) -delete

	# Clean root directory
	@echo "Cleaning root directory..."
	@rm -f *~ *.o *.s bochsout.txt parport.out system.out system user user.out zeos.bin build

	@echo "=== Done ==="

# Redirect all other targets to the project directory
%:
	$(MAKE) -C $(PROJECT_DIR) $@

.PHONY: all clean restart emul reemul gdb emuldbg format format-check backup disk doxygen