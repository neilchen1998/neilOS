# Custom OS

##

##

Compile the code:

```sh
make
```

Load floppy disk into the emulator:

```sh
qemu-system-i386 -fda build/main_floppy.img
```

## Registers

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
