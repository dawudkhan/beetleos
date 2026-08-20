# BeetleOS

A hobby RISC-V kernel, built from scratch.

## Status

BeetleOS boots on QEMU's `virt` machine into supervisor mode and currently implements:

- Physical page allocator
- Sv39 virtual memory
- Trap and interrupt handling (including timer interrupts)
- Per-process address spaces, user-mode execution
- Round-robin scheduling across processes
- System calls (write, exit)

The project is under active development.

## Build & run

### Docker

```sh
docker build -t beetleos .
docker run -it --rm beetleos
```

### Locally

Requires a `riscv64-unknown-elf` toolchain and `qemu-system-riscv64`.

```sh
make run
```

`Ctrl-A` then `x` to exit QEMU.

