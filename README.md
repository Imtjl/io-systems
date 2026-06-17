# Low-Level I/O — from bare metal to the kernel

A walk down the whole I/O stack, one small project per layer: from running
**below the operating system** on bare-metal RISC-V, through **register-level
MCU firmware** and **decoding real buses on a logic analyzer**, up to **inside
the Linux kernel** as a device driver.

Coursework for _I/O Systems_ (ITMO, Faculty of Software Engineering & Computer
Engineering), but the work is genuinely low-level: no HAL, no libc where it
counts, hand-written boot code, hand-decoded waveforms.

## What this demonstrates

- **Bare-metal C, no OS / no libc** — custom boot stub (set up the stack, jump
  to C), linker script, freestanding build, a from-scratch `printf`.
- **The firmware ↔ OS boundary** — talking to OpenSBI from S-mode via the RISC-V
  `ecall` ABI (args in `a0..a5`, FID in `a6`, EID in `a7`).
- **Register-level MCU firmware** — AVR (ATmega328) configured straight on the
  registers: UART, Timer1 (CTC), interrupts — no Arduino `Serial`.
- **Buses, hands-on** — `I²C`, `1-Wire`, `SPI`, `UART` used _and_ decoded
  byte-by-byte from logic-analyzer traces.
- **Protocol design** — a framed link-layer protocol (SYNC + length + CRC8) in
  firmware with a Python host counterpart, plus a from-scratch interface design
  (PHY / link / transport) with a throughput budget.
- **Inside the Linux kernel** — a character device driver: `/dev` node, file
  operations, the user↔kernel boundary.

## The stack, layer by layer

| #                                 | Project                                | Layer         | Highlights                                                                                                           |
| --------------------------------- | -------------------------------------- | ------------- | -------------------------------------------------------------------------------------------------------------------- |
| [01](01-bare-metal-riscv-opensbi) | Bare-metal RISC-V over OpenSBI         | below the OS  | naked boot + linker script @ `0x80200000`, `ecall` SBI ABI, own `printf`, runs in QEMU                               |
| [02](02-avr-uart-protocol)        | AVR firmware + framed UART protocol    | on the metal  | register-level UART (19200 8E1) + Timer1 CTC + ISRs; BMP280 over I²C; SYNC/len/**CRC8** framing; Python host         |
| [03](03-bus-decoding-i2c-1wire)   | I²C / 1-Wire decoding (logic analyzer) | on the wire   | decoded **BMP280 → 27.24 °C** from its calibration table by hand; **DHT-11 → 35 % / 27.2 °C** with checksum verified |
| [04](04-interface-design)         | Custom 3-wire interface design         | on the wire   | synchronous half-duplex bus, link-layer framing, **235 → 469 kbit/s** effective throughput budget, transport layer   |
| [05](05-linux-char-driver)        | Linux character device driver          | inside the OS | `alloc_chrdev_region` → `class_create` → `cdev_add`, `/dev/mychdev`, version-guarded for Linux ≥ 6.4                 |

## Quick start (the headline one)

```bash
cd 01-bare-metal-riscv-opensbi
./script.sh          # builds the freestanding kernel, fetches OpenSBI, boots QEMU
```

Needs `clang` (with the RISC-V target) and `qemu-system-riscv32`. Each project
has its own README with build/run details and the engineering notes.

## Reports

Full lab write-ups (RU) live next to their code as `report.pdf` — timing-diagram
analysis, the decoding math, and the throughput derivations.
