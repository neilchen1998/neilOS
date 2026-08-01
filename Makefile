ASM      := nasm
CC       := gcc
LD       := ld
OBJCOPY  := objcopy

BUILD := _build

# Binary files
STAGE1_BIN := $(BUILD)/boot/stage1/stage1.bin
STAGE2_BIN := $(BUILD)/boot/stage2/stage2.bin
KERNEL_BIN := $(BUILD)/kernel/kernel.bin
IMAGE := $(BUILD)/neilOS.img

# All
.PHONY: all image clean run debug
all: image


# Disk image
image: $(IMAGE)
$(IMAGE): $(STAGE1_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	@mkdir -p $(BUILD)

	@echo "Creating floppy image"
	dd if=/dev/zero of=$@ bs=512 count=2880

	@echo "Formatting FAT12"
	mkfs.fat -F 12 -n "NEILOS" $@

	@echo "Installing stage1"
	dd if=$(STAGE1_BIN) of=$@ bs=512 count=1 conv=notrunc

	@echo "Installing stage2"
	dd if=$(STAGE2_BIN) of=$@ bs=512 seek=2864 conv=notrunc

	@echo "Installing kernel"
	mcopy -i $@ $(KERNEL_BIN) "::kernel"


# Stage 1
$(STAGE1_BIN): boot/stage1/boot.asm
	@mkdir -p $(dir $@)

	$(ASM) \
		-f bin \
		$< \
		-o $@

	@size=$$(stat -c%s $@); \
	if [ $$size -ne 512 ]; then \
		echo "Stage1 must be exactly 512 bytes"; \
		exit 1; \
	fi


# Stage 2
STAGE2_DIR := $(BUILD)/boot/stage2

$(STAGE2_DIR)/entry.o: boot/stage2/entry.asm
	@mkdir -p $(dir $@)

	$(ASM) -f elf32 $< -o $@

$(STAGE2_DIR)/stage2.o: boot/stage2/stage2.c
	@mkdir -p $(dir $@)

	$(CC) \
		-m16 \
		-ffreestanding \
		-fno-pic \
		-fno-stack-protector \
		-fno-asynchronous-unwind-tables \
		-Wall \
		-c $< \
		-o $@


$(STAGE2_DIR)/stage2.elf: $(STAGE2_DIR)/entry.o $(STAGE2_DIR)/stage2.o boot/stage2/linker.ld

	$(LD) \
		-m elf_i386 \
		-T boot/stage2/linker.ld \
		-o $@ \
		$(STAGE2_DIR)/entry.o \
		$(STAGE2_DIR)/stage2.o


$(STAGE2_BIN): $(STAGE2_DIR)/stage2.elf
	$(OBJCOPY) -O binary $< $@


# Kernel
$(KERNEL_BIN): kernel/main.asm
	@mkdir -p $(dir $@)

	$(ASM) -f bin $< -o $@


# Development
run: image
	qemu-system-i386 -fda $(IMAGE)

debug: image
	qemu-system-i386 -fda $(IMAGE) -s -S


# Clean
clean:
	rm -rf $(BUILD)
