# Configuration
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


# Default target
all: image


# Disk image
image: boot kernel

$(IMAGE): $(BOOTLOADER) $(KERNEL)
	mkdir -p $(BUILD_DIR)
	dd if=/dev/zero of=$@ bs=512 count=2880
	mkfs.fat -F 12 -n "DUMMY" $@
	dd if=$(BOOTLOADER) of=$@ conv=notrunc
	mcopy -i $@ $(KERNEL) "::kernel"


# Bootloader
boot: stage1

stage1:
	$(MAKE) -C boot/stage1

stage2:
	$(MAKE) -C boot/stage2


# Kernel
kernel: $(KERNEL)

$(KERNEL): kernel/main.asm
	mkdir -p $(BUILD_DIR)
	$(ASM) $< -f bin -o $@


# Development helpers
.PHONY: run

run: image
	qemu-system-i386 -fda $(IMAGE)

debug: image
	qemu-system-i386 -fda $(IMAGE) \
	-drive format=raw,file=$(BUILD_DIR)/os.img -s -S


# Cleanup
.PHONY: all image boot stage1 stage2 kernel tools run debug clean

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C boot/stage1 clean
	$(MAKE) -C boot/stage2 clean
	$(MAKE) -C kernel clean
