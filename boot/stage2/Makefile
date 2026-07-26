ASM      ?= nasm
CC      ?= gcc
LD      ?= ld
OBJCOPY ?= objcopy

BUILD ?= ../../_build

CFLAGS = -m16 \
       -ffreestanding \
       -fno-pic \
       -fno-stack-protector \
       -fno-asynchronous-unwind-tables \
       -Wall

all: $(BUILD)/stage2.bin

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/entry.o: entry.asm | $(BUILD)
	$(ASM) -f elf32 entry.asm -o $@

$(BUILD)/stage2.o: stage2.c | $(BUILD)
	$(CC) $(CFLAGS) -c stage2.c -o $@

$(BUILD)/stage2.elf: $(BUILD)/entry.o $(BUILD)/stage2.o linker.ld
	$(LD) \
	    -m elf_i386 \
	    -T linker.ld \
	    -o $@ \
	    $(BUILD)/entry.o $(BUILD)/stage2.o

$(BUILD)/stage2.bin: $(BUILD)/stage2.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf $(BUILD)
