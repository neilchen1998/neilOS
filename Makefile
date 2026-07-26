# Configuration
ASM := nasm
CC  := gcc

export ASM CC

CFLAGS   := -Wall -Wextra -g
LDFLAGS  :=
LDLIBS   :=

SRC_DIR   := src
BUILD_DIR := _build
TOOLS_DIR := tools

BOOTLOADER := $(BUILD_DIR)/bootloader.bin
KERNEL     := $(BUILD_DIR)/kernel.bin
IMAGE      := $(BUILD_DIR)/neilOS.img


# Default target
.PHONY: all image

all: image

image: $(IMAGE)


# Disk image
$(IMAGE): $(BOOTLOADER) $(KERNEL)
	@mkdir -p $(BUILD_DIR)

	@echo "Creating floppy image..."
	dd if=/dev/zero of=$@ bs=512 count=2880

	@echo "Formatting FAT12..."
	mkfs.fat -F 12 -n "NEILOS" $@

	@echo "Installing stage1..."
	dd if=_build/stage1.bin of=$@ bs=512 count=1 conv=notrunc

	@echo "Installing stage2..."
	mcopy -i $@ _build/stage2.bin "::stage2"

	@echo "Installing kernel..."
	mcopy -i $@ $(KERNEL) "::kernel"


# Bootloader
.PHONY: boot stage1 stage2

boot: $(BOOTLOADER)

STAGE1_BIN := _build/stage1.bin
STAGE2_BIN := _build/stage2.bin

$(BOOTLOADER): stage1 stage2
	@mkdir -p $(BUILD_DIR)

stage1:
	$(MAKE) -C boot/stage1

stage2:
	$(MAKE) -C boot/stage2


# Kernel
.PHONY: kernel

kernel: $(KERNEL)

$(KERNEL): kernel/main.asm
	@mkdir -p $(BUILD_DIR)
	$(ASM) $< -f bin -o $@


# Development helpers
.PHONY: run debug

run: image
	qemu-system-i386 -fda $(IMAGE)

debug: image
	qemu-system-i386 -fda $(IMAGE) \
	-drive format=raw,file=$(BUILD_DIR)/os.img -s -S


# Cleanup
.PHONY: clean

clean:
	rm -rf $(BUILD_DIR)
	$(MAKE) -C boot/stage1 clean
	$(MAKE) -C boot/stage2 clean
	$(MAKE) -C kernel clean
