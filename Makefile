# =====================
# Configuration
# =====================

ASM := nasm
CC  := gcc

CFLAGS   := -Wall -Wextra -g
CPPFLAGS := -I$(TOOLS_DIR)/fat
LDFLAGS  :=
LDLIBS   :=

SRC_DIR   := src
BUILD_DIR := build
TOOLS_DIR := tools

BOOTLOADER := $(BUILD_DIR)/bootloader.bin
KERNEL     := $(BUILD_DIR)/kernel.bin
IMAGE      := $(BUILD_DIR)/main_floppy.img
FAT_TOOL   := $(BUILD_DIR)/tools/fat


# =====================
# Default target
# =====================

.PHONY: all
all: image tools


# =====================
# Disk image
# =====================

.PHONY: image

image: $(IMAGE)

$(IMAGE): $(BOOTLOADER) $(KERNEL)
	mkdir -p $(BUILD_DIR)
	dd if=/dev/zero of=$@ bs=512 count=2880
	mkfs.fat -F 12 -n "DUMMY" $@
	dd if=$(BOOTLOADER) of=$@ conv=notrunc
	mcopy -i $@ $(KERNEL) "::kernel"


# =====================
# Bootloader
# =====================

.PHONY: bootloader

bootloader: $(BOOTLOADER)

$(BOOTLOADER): $(SRC_DIR)/bootloader/boot.asm
	mkdir -p $(BUILD_DIR)
	$(ASM) $< -f bin -o $@


# =====================
# Kernel
# =====================

.PHONY: kernel

kernel: $(KERNEL)

$(KERNEL): $(SRC_DIR)/kernel/main.asm
	mkdir -p $(BUILD_DIR)
	$(ASM) $< -f bin -o $@


# =====================
# Host tools
# =====================

.PHONY: tools

tools: $(FAT_TOOL)

$(FAT_TOOL): $(TOOLS_DIR)/fat/fat.c $(TOOLS_DIR)/fat/fat.h
	mkdir -p $(BUILD_DIR)/tools
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@


# =====================
# Development helpers
# =====================

.PHONY: run

run: image
	qemu-system-i386 -fda $(IMAGE)


# =====================
# Cleanup
# =====================

.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)
