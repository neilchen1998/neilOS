# neilOS

A small x86 operating system built from scratch using NASM assembly and C.

neilOS is a learning-focused operating system project that implements a custom BIOS boot process, a second-stage loader, and a minimal kernel. The goal of the project is to understand the fundamentals of how computers boot and how an operating system begins execution.

## Features

- Custom BIOS bootloader
- 512-byte FAT12-compatible boot sector
- Stage 1 bootloader written in x86 assembly
- Stage 2 loader written in C and assembly
- Custom linker script
- Minimal assembly kernel
- Automated build system
- QEMU testing environment

## Requirements

The following tools are required:

- NASM
- GCC
- GNU Binutils (`ld`, `objcopy`)
- GNU Make
- mtools
- QEMU

### Fedora

Install dependencies:

```bash
sudo dnf install nasm gcc binutils make mtools qemu-system-x86
```

## Building

Build the operating system image:

```sh
make
```

The generated files are stored under:

```sh
_build/
```

The final bootable floppy image:

```sh
_build/neilOS.img
```

## Running

Launch neilOS using QEMU:

```sh
make run
```

This boots the generated floppy image.

## Debugging

Start QEMU paused with a debugging port enabled:

```sh
make debug
```

## Note

### Assembly Byte, Word, DWord, QWord

- **db** (define byte):	1 byte / 8 bits
- **dw** (define word):	2 bytes / 16 bits
- **dd** (define doubleword):	4 bytes / 32 bits
- **dq** (define quadword):	8 bytes / 64 bits

### cdecl

On 32-bit x86, the most common C calling convention is cdecl.
Under cdecl, registers are divided into caller-saved and callee-saved.

Caller-saved (volatile) registers:

- EAX
- ECX
- EDX

Callee-saved (non-volatile) registers:

- EBX
- ESI
- EDI
- EBP
- ESP

### Registers

- **AX**: accumulator (general purpose calculation)
- **BP**: base pointer (used to access stack variable)
- **BX**: base register (used for addressing)
- **DS**: data segment
- **DX**: data register (used with multiplication, division, and I/O)
- **IP**: instruction pointer (tracks the *next* instruction)
- **SI**: source index (points to source data)
- **SP**: stack pointer (points to the top of the stack)

### Registers (32-bit x86 protected mode)

- **DS**:	data segment selector
- **ES**:	extra segment selector
- **FS**:	segment selector
- **GS**:	segment selector
- **SS**:	stack segment selector

### Line control register

| Bits | Name         | Access | Description |
|------|--------------|--------|-------------|
| 31-8 | Reserved     | N/A    | Reserved |
| 7    | DLAB         | R/W    | 1 = Allows access to the Divisor Latch Registers and reading of the FIFO Control Register. 0 = Allows access to RBR, THR, IER and IIR registers. |
| 6    | Set Break    | R/W    | 1 = Enables break condition. 0 = Disables break condition. |
| 5    | Stick Parity | R/W    | 1 = Enables Stick Parity. 0 = Disables Stick Parity. |
| 4    | EPS          | R/W    | 1 = Selects Even parity. 0 = Selects Odd parity. |
| 3    | PEN          | R/W    | 1 = Enables parity. 0 = Disables parity. |
| 2    | STB          | R/W    | 0 = 1 Stop bit. 1 = 2 Stop bits or 1.5, if 5 bits/character selected. |
| 1-0  | WLS          | R/W    | 00 = 5 bits/character. 01 = 6 bits/character. 10 = 7 bits/character. 11 = 8 bits/character. |

### PIC (Programmable Interrupt Controller)

Hardware device
      │
      │ IRQ
      ▼
     PIC
      │
      │ interrupt vector
      ▼
     CPU
      │
      │ looks up vector in IDT
      ▼
    isr32
      │
      ▼
  isr_common
      │
      ▼
Handle interrupt

### Programmable Interview Timer (PIT)

PIT
 │
 │ generates periodic signal
 ▼
IRQ 0
 │
 ▼
8259 PIC
 │
 │ remapped to interrupt 32 (0x20)
 ▼
IDT[32]
 │
 ▼
isr32
 │
 ▼
interrupt_handler()
 │
 ▼
case 32:
    pit_tick();

### Memory Management

This is the diagram of the memory when we find a valid space in the memory:

```text
b                                      split
│                                         │
▼                                        ▼
┌────────────────┬────────────────────────┬───────────────────────┐
│  b's header    │   allocated portion    │     leftover space    │
│  HEADER_SIZE   │        size            │                       │
└────────────────┴────────────────────────┴───────────────────────┘
                 ↑                         ↑
                 │                         │
              b + HEADER_SIZE            split
```

### Memory Map

#### Real mode address space

### Kernel Code Descriptor

#### Decode **0x00CF9A000000FFFF**

```text
63            56 55       48 47       40 39       32 31           24 23       16 15        8 7         0
+---------------+-----------+-----------+-----------+---------------+-----------+-----------+-----------+
| 0x00          | 0xCF      | 0x9A      | 0x00      | 0x00          | 0x00      | 0xFF      | 0xFF      |
+---------------+-----------+-----------+-----------+---------------+-----------+-----------+-----------+
```

#### Access byte:

```text
0x9A = 1001 1010
       ││││ │││└─ Accessed = 0
       ││││ ││└── Readable = 1
       ││││ │└─── Conforming = 0
       ││││ └──── Executable = 1
       │││└────── Descriptor type = 1 (code/data)
       ││└─────── DPL bit 1 = 0
       │└──────── DPL bit 0 = 0
       └───────── Present = 1
```

#### Upper limit bits & flags:

```text
0xCF = 1100 1111
       ││││ │││└─ Limit bit 16
       ││││ ││└── Limit bit 17
       ││││ │└─── Limit bit 18
       ││││ └──── Limit bit 19
       │││└────── AVL = 0
       ││└─────── L = 0
       │└──────── D/B = 1 (32-bit)
       └───────── G = 1 (4 KiB)
```

## Resources

- [Building an OS](https://youtube.com/playlist?list=PLFjM7v6KGMpiH2G-kT781ByCNC_0pKpPN&si=Pr8kDEQrsbQuhrR4)
- [cdecl](https://linux.die.net/man/1/cdecl)
- [GDT](https://www.youtube.com/watch?v=Wh5nPn2U_1w)
- [Line Control Register](https://docs.amd.com/r/en-US/pg143-axi-uart16550/Line-Control-Register)
- [Understanding the FAT file system](https://8dcc.github.io/programming/understanding-fat.html)
