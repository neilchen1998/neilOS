ASM      ?= nasm
ASMFLAGS ?= -f bin

SOURCE   = boot.asm
OUTPUT   = ../../build/stage1.bin

all: $(OUTPUT) check

$(OUTPUT): $(SOURCE)
	@mkdir -p ../../build
	$(ASM) $(ASMFLAGS) $(SOURCE) -o $(OUTPUT)

check: $(OUTPUT)
	@size=$$(stat -c%s $(OUTPUT)); \
	if [ $$size -ne 512 ]; then \
		echo "ERROR: Stage 1 must be exactly 512 bytes (got $$size)"; \
		exit 1; \
	fi
	@echo "Stage 1 size OK (512 bytes)"

.PHONY: all clean check

clean:
	rm -f $(OUTPUT)
