# Bochs 2.6.7 Installation and Configuration on Debian 12

This guide explains how to download, install, and configure Bochs 2.6.7 with debugging capabilities. You need to compile the Bochs source code to use the debugging facilities, as debugger options are disabled by default in the standard binary package.

## Part 1: Bochs Installation

Since we need the exact version 2.6.7 (not the one from Debian 12 repositories), we must download and compile the source code manually.

### Official Links
- [Bochs Official Page](http://bochs.sourceforge.net/)
- [Direct Download 2.6.7](https://sourceforge.net/projects/bochs/files/bochs/2.6.7/bochs-2.6.7.tar.gz/download)

### Step 1: Navigate to Project Directory and Install Dependencies

```bash
sudo apt update
```

```bash
sudo apt install build-essential
```

```bash
sudo apt install bin86
```

```bash
sudo apt install libx11-dev libgtk2.0-dev libreadline-dev libxpm-dev
```

### Step 2: Download Source Code

```bash
wget https://sourceforge.net/projects/bochs/files/bochs/2.6.7/bochs-2.6.7.tar.gz/download -O bochs-2.6.7.tar.gz
```

### Step 3: Extract Source Code

```bash
tar -xvzf bochs-2.6.7.tar.gz
```

```bash
cd bochs-2.6.7
```

## Part 2: Bochs Configuration and Compilation

You can choose between two debugging options. You can install both versions in different directories.


### **Option A**: Compile with Internal Debugger Support

This version includes Bochs' built-in debugger with basic debugging features.

#### Configure:
```bash
./configure --enable-debugger --enable-disasm --enable-x86-debugger --enable-readline --with-x --prefix=/opt/bochs
```

#### Compile:
```bash
make -j$(($(nproc)-2))
```

#### Install:
```bash
sudo make install
```

### **Option B**: Compile with External GDB Debugger Support

This version allows you to use external GDB for advanced debugging.

**Note:** If switching from Option A, clean first with `make clean`

#### Configure:
```bash
./configure --enable-gdb-stub --with-x --prefix=/opt/bochs_gdb
```

#### Compile:
```bash
make -j$(($(nproc)-2))
```

#### Install:
```bash
sudo make install
```

### Verify Installation

Check the installation of the internal debugger version:
```bash
/opt/bochs/bin/bochs -v
```

Check the installation of the GDB version:
```bash
/opt/bochs_gdb/bin/bochs -v
```
### Create System-wide Symbolic Links (Optional)

To make the Bochs commands available system-wide for the Makefile targets, create symbolic links:

```bash
# Link for GDB debugging version (used by 'make gdb')
sudo ln -s /opt/bochs_gdb/bin/bochs /usr/local/bin/bochs

# Link for internal debugger version (used by 'make emuldbg') 
sudo ln -s /opt/bochs/bin/bochs /usr/local/bin/bochs_nogdb

which bochs          # Should show: /usr/local/bin/bochs
which bochs_nogdb    # Should show: /usr/local/bin/bochs_nogdb
```

## Part 3: Testing ZeOS

Once Bochs is installed, you can compile and test your ZeOS operating system.

### Navigate to ZeOS Directory

```bash
cd zeos
```

### Compile ZeOS

```bash
make
```

### Run ZeOS with Different Debugger Options

#### Option 1: Run with Bochs Internal Debugger

```bash
make emuldbg
```

- A new window will appear showing your ZeOS output
- The debugger prompt will show the current address and wait for commands
- Press `c` and ENTER to continue execution
- Press `Ctrl+C` to finish and exit the debugger

#### Option 2: Run with External GDB Debugger

```bash
make gdb
```

- Bochs will start in the background with GDB stub enabled
- GDB will automatically attach and load debugging symbols
- Use standard GDB commands for debugging
- Press `Ctrl+C` in GDB to interrupt execution
- Use `quit` to exit GDB and terminate Bochs

#### Option 3: Run Normal Execution (No Debugger)

```bash
make emul
```

- Runs ZeOS in normal mode without debugging capabilities
- Faster execution for testing basic functionality
- Press `Ctrl+C` to exit Bochs