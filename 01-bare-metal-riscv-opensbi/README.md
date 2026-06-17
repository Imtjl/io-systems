# 01 · Bare-metal RISC-V over OpenSBI

*Below the OS.* A freestanding `rv32` "kernel" that brings up its own stack and
talks to the **OpenSBI** firmware through the RISC-V `ecall` ABI, exposing an
interactive console menu. No operating system, no libc.

## What it demonstrates

- **Custom boot** — a `naked` stub in `.text.boot` sets up `sp` and jumps into
  `kernel_main` (see `kernel.c` + `kernel.ld`, load address `0x80200000`).
- **The firmware ↔ supervisor boundary** — every console op is an SBI call:
  arguments in `a0..a5`, function id in `a6`, extension id in `a7`, result in
  `{a0, a1}`. EIDs/FIDs are named constants, not magic numbers.
- **Freestanding C** — own `putchar` / `getchar` / `puts` and a from-scratch
  `printf` (`%s/%d/%x`), built with `-ffreestanding -nostdlib`.

## Menu

1. Get SBI implementation version (Base extension, FID 2)
2. Hart get status (HSM extension `0x48534D`, FID 2)
3. Hart stop (HSM, FID 1)
4. System shutdown (legacy EID `0x08`)

## Build & run

```bash
./script.sh
```

The script builds `kernel.elf` against `kernel.ld`, downloads the OpenSBI
firmware blob if missing, and launches `qemu-system-riscv32`.

**Requirements:** `clang` (with the `riscv32` target) and `qemu-system-riscv32`.

## Files

| File | Role |
|------|------|
| `kernel.c` | boot stub, SBI calls, menu loop |
| `common.c` / `common.h` | freestanding `printf` |
| `kernel.h` | `struct sbiret` |
| `kernel.ld` | sections, load address, BSS/stack symbols |
| `script.sh` | build + fetch OpenSBI + run QEMU |

Full write-up: [`report.pdf`](report.pdf).
