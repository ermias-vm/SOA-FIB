# Root Makefile - Redirects all commands to specific project directory

PROJECT_DIR = zeos

# Default target
all:
	$(MAKE) -C $(PROJECT_DIR)

# Common ZeOS targets
clean:
	$(MAKE) -C $(PROJECT_DIR) clean

restart:
	$(MAKE) -C $(PROJECT_DIR) restart

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

# Redirect all other targets to the project directory
%:
	$(MAKE) -C $(PROJECT_DIR) $@

.PHONY: all clean restart emul gdb emuldbg format format-check backup disk