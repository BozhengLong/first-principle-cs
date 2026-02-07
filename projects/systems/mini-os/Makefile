# Makefile for mini-os

# Compiler and tools
CC = gcc
AS = gas
LD = ld
OBJCOPY = objcopy
QEMU = qemu-system-i386

# Compiler flags
CFLAGS = -fno-pic -static -fno-builtin -fno-strict-aliasing -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer
CFLAGS += -fno-stack-protector -Wno-main-return-type
ASFLAGS = -m32 -gdwarf-2 -Wa,-divide
LDFLAGS = -m elf_i386

# Directories
KERNEL_DIR = kernel
USER_DIR = user
TEST_DIR = tests
BUILD_DIR = build

# Kernel source files
KERNEL_SRCS = \
	$(KERNEL_DIR)/main.c \
	$(KERNEL_DIR)/lib/string.c \
	$(KERNEL_DIR)/lib/printf.c \
	$(KERNEL_DIR)/lib/spinlock.c \
	$(KERNEL_DIR)/proc/proc.c \
	$(KERNEL_DIR)/proc/cpu.c \
	$(KERNEL_DIR)/mm/vm.c \
	$(KERNEL_DIR)/mm/kalloc.c \
	$(KERNEL_DIR)/trap/trap.c \
	$(KERNEL_DIR)/trap/syscall.c

KERNEL_ASMS = \
	$(KERNEL_DIR)/boot/boot.S \
	$(KERNEL_DIR)/proc/swtch.S \
	$(KERNEL_DIR)/trap/trapasm.S \
	$(KERNEL_DIR)/trap/vectors.S

# Object files
KERNEL_OBJS = $(KERNEL_SRCS:.c=.o) $(KERNEL_ASMS:.S=.o)

# Default target
all: kernel.bin

# Build kernel
kernel.bin: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) -T kernel.ld -o kernel.elf $(KERNEL_OBJS)
	$(OBJCOPY) -S -O binary kernel.elf kernel.bin

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -I$(KERNEL_DIR) -c $< -o $@

# Compile assembly files
%.o: %.S
	$(CC) $(ASFLAGS) -I$(KERNEL_DIR) -c $< -o $@

# Run in QEMU
qemu: kernel.bin
	$(QEMU) -kernel kernel.bin -serial mon:stdio

# Run with GDB
qemu-gdb: kernel.bin
	$(QEMU) -kernel kernel.bin -serial mon:stdio -s -S

# Run tests
test:
	@echo "Running tests..."
	@cd $(TEST_DIR) && make test

# Clean build artifacts
clean:
	rm -f $(KERNEL_OBJS) kernel.elf kernel.bin
	rm -f $(KERNEL_DIR)/**/*.o $(KERNEL_DIR)/**/*.d
	rm -f $(USER_DIR)/**/*.o $(USER_DIR)/**/*.d
	rm -f $(TEST_DIR)/**/*.o $(TEST_DIR)/**/*.d

# Help
help:
	@echo "mini-os Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build kernel (default)"
	@echo "  qemu      - Run kernel in QEMU"
	@echo "  qemu-gdb  - Run kernel in QEMU with GDB"
	@echo "  test      - Run test suite"
	@echo "  clean     - Remove build artifacts"
	@echo "  help      - Show this help message"

.PHONY: all qemu qemu-gdb test clean help
