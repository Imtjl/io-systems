#!/bin/bash

set -xue

QEMU=qemu-system-riscv32
OPENSBI_FILE="opensbi-riscv32-generic-fw_dynamic.bin"
OPENSBI_URL="https://github.com/qemu/qemu/raw/v8.0.4/pc-bios/opensbi-riscv32-generic-fw_dynamic.bin"

CC=/usr/bin/clang
CFLAGS="-std=c11 -O2 -g3 -Wall -Wextra --target=riscv32 -ffreestanding -nostdlib"

# Check if OpenSBI file exists, if not, download it
if [ ! -f "$OPENSBI_FILE" ]; then
    echo "OpenSBI file not found. Downloading..."
    curl -LO "$OPENSBI_URL"
fi

# Build the kernel
$CC $CFLAGS -Wl,-Tkernel.ld -Wl,-Map=kernel.map -o kernel.elf \
    kernel.c common.c

# Run QEMU
$QEMU -machine virt -bios "$OPENSBI_FILE" -nographic -serial mon:stdio --no-reboot \
    -kernel kernel.elf
