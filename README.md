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

### Registers

- **AX**: accumulator (general purpose calculation)
- **BP**: base pointer (used to access stack variable)
- **BX**: base register (used for addressing)
- **DS**: data segment
- **DX**: data register (used with multiplication, division, and I/O)
- **IP**: instruction pointer (tracks the *next* instruction)
- **SI**: source index (points to source data)
- **SP**: stack pointer (points to the top of the stack)

## Resources

- [Building an OS](https://youtube.com/playlist?list=PLFjM7v6KGMpiH2G-kT781ByCNC_0pKpPN&si=Pr8kDEQrsbQuhrR4)
- [Understanding the FAT file system](https://8dcc.github.io/programming/understanding-fat.html)
