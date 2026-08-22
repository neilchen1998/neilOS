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

$(STAGE2_DIR)/disk.o: boot/stage2/disk.asm
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
		-Iboot/stage2/lib \
		-Wall \
		-c $< \
		-o $@

# Lib archive
STAGE2_LIB_SRC := $(wildcard boot/stage2/lib/*.c)
STAGE2_LIB_OBJ := $(patsubst %.c,$(STAGE2_DIR)/%.o,$(notdir $(STAGE2_LIB_SRC)))

$(STAGE2_DIR)/lib.a: $(STAGE2_LIB_SRC)
	@mkdir -p $(dir $@)

	$(foreach src,$(STAGE2_LIB_SRC), \
		$(CC) \
			-m16 \
			-ffreestanding \
			-fno-pic \
			-fno-stack-protector \
			-fno-asynchronous-unwind-tables \
			-ffunction-sections \
			-fdata-sections \
			-Iboot/stage2/lib \
			-Wall \
			-c $(src) \
			-o $(STAGE2_DIR)/$(notdir $(src:.c=.o)); \
	)

	ar rcs $@ $(STAGE2_LIB_OBJ)


$(STAGE2_DIR)/stage2.elf: $(STAGE2_DIR)/entry.o $(STAGE2_DIR)/stage2.o $(STAGE2_DIR)/disk.o $(STAGE2_DIR)/lib.a boot/stage2/linker.ld

	$(LD) \
		-m elf_i386 \
		-T boot/stage2/linker.ld \
		-o $@ \
		$(STAGE2_DIR)/disk.o \
		$(STAGE2_DIR)/entry.o \
		$(STAGE2_DIR)/stage2.o \
		$(STAGE2_DIR)/lib.a


$(STAGE2_BIN): $(STAGE2_DIR)/stage2.elf
	$(OBJCOPY) -O binary $< $@

	@size=$$(stat -c%s $@); \
	if [ $$size -gt 8192 ]; then \
		echo "Stage 2 is too large ($$size bytes): Stage 1 only loads 16 sectors"; \
		exit 1; \
	fi


# Kernel
KERNEL_DIR := $(BUILD)/kernel

KERNEL_LD := kernel/linker.ld

KERNEL_C_SOURCES := kernel/kernel.c \
	kernel/arch/x86/idt.c \
	kernel/arch/x86/pic.c \
	kernel/drivers/keyboard/keyboard.c \
	kernel/drivers/timer/pit.c \
	kernel/drivers/video/vga.c \
	kernel/mm/kmalloc.c \
	kernel/mm/paging.c \
	kernel/mm/physical.c \
	kernel/scheduler/scheduler.c

KERNEL_ASM_SOURCES := kernel/main.asm kernel/arch/x86/isr.asm

KERNEL_C_OBJS := $(patsubst kernel/%.c,$(KERNEL_DIR)/%.o,$(KERNEL_C_SOURCES))
KERNEL_ASM_OBJS := $(patsubst kernel/%.asm,$(KERNEL_DIR)/%.o,$(KERNEL_ASM_SOURCES))

KERNEL_OBJS := $(KERNEL_C_OBJS) $(KERNEL_ASM_OBJS)

KERNEL_ELF := $(KERNEL_DIR)/kernel.elf
KERNEL_BIN := $(KERNEL_DIR)/kernel.bin

KERNEL_CFLAGS := -m32 -ffreestanding -fno-pie -fno-stack-protector -Wall -Ikernel

KERNEL_LDFLAGS := -m elf_i386 -T $(KERNEL_LD)

$(KERNEL_DIR)/%.o: kernel/%.c
	@mkdir -p $(@D)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

$(KERNEL_DIR)/%.o: kernel/%.asm
	@mkdir -p $(@D)
	$(ASM) -f elf32 $< -o $@

$(KERNEL_ELF): $(KERNEL_OBJS) $(KERNEL_LD)
	@mkdir -p $(@D)
	$(LD) $(KERNEL_LDFLAGS) -o $@ $(KERNEL_OBJS)

$(KERNEL_BIN): $(KERNEL_ELF)
	@mkdir -p $(@D)
	$(OBJCOPY) -O binary $< $@


# Development
run: image
	qemu-system-i386 -fda $(IMAGE) -serial stdio

debug: image
	qemu-system-i386 -fda $(IMAGE) -s -S


# Clean
clean:
	rm -rf $(BUILD)
