# Makefile for kacchiOS (Xinu-style)

# Compiler and tools
CC = gcc
AS = as
LD = ld
AR = ar

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = $(SRC_DIR)

# Compiler flags (adjust for your target architecture)
CFLAGS = -Wall -Wextra -std=c99 -ffreestanding -nostdlib -nostdinc \
         -fno-builtin -fno-stack-protector -m32 -I$(INCLUDE_DIR)

# Assembler flags
ASFLAGS = --32

# Linker flags
LDFLAGS = -m elf_i386 -nostdlib

# Source files
C_SOURCES = $(SRC_DIR)/memory.c \
            $(SRC_DIR)/process.c \
            $(SRC_DIR)/scheduler.c \
            $(SRC_DIR)/io.c \
            $(SRC_DIR)/main.c

ASM_SOURCES = $(SRC_DIR)/ctxsw.S

# Object files
C_OBJECTS = $(C_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
ASM_OBJECTS = $(ASM_SOURCES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%.o)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# Target executable
TARGET = kacchiOS

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link object files
$(TARGET): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Build complete: $(TARGET)"

# Compile C source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled: $<"

# Assemble assembly files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.S
	$(AS) $(ASFLAGS) $< -o $@
	@echo "Assembled: $<"

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned build artifacts"

# Rebuild from scratch
rebuild: clean all

# Run in QEMU (adjust for your architecture)
run: $(TARGET)
	qemu-system-i386 -kernel $(TARGET) -serial stdio

# Run with debugging
debug: $(TARGET)
	qemu-system-i386 -kernel $(TARGET) -serial stdio -s -S

# Show help
help:
	@echo "Available targets:"
	@echo "  all      - Build the OS (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  rebuild  - Clean and build"
	@echo "  run      - Run in QEMU"
	@echo "  debug    - Run in QEMU with GDB support"
	@echo "  help     - Show this help"

# Phony targets
.PHONY: all clean rebuild run debug help

# Dependencies (optional, for incremental builds)
$(BUILD_DIR)/memory.o: $(SRC_DIR)/memory.h $(SRC_DIR)/process.h
$(BUILD_DIR)/process.o: $(SRC_DIR)/process.h $(SRC_DIR)/memory.h $(SRC_DIR)/scheduler.h
$(BUILD_DIR)/scheduler.o: $(SRC_DIR)/scheduler.h $(SRC_DIR)/process.h
$(BUILD_DIR)/main.o: $(SRC_DIR)/process.h $(SRC_DIR)/scheduler.h $(SRC_DIR)/memory.h
