# 05 · Linux character device driver

*Inside the OS.* A minimal loadable kernel module that registers a character
device and creates `/dev/mychdev`. The same `open`/`read`/`write`/`close`
syscalls a user program issues land in this module's callbacks — the top of the
I/O stack, mirroring the bare-metal bottom in project 01.

## What it demonstrates

- **Kernel module lifecycle** — `module_init` / `module_exit` with full,
  ordered teardown on every failure path.
- **The char device model** — `alloc_chrdev_region` → `class_create` →
  `device_create` → `cdev_init` / `cdev_add`, exposing a `file_operations`
  table.
- **The user ↔ kernel boundary** — `__user` buffers, `ssize_t` semantics, and
  logging each call to the kernel ring buffer.
- **Portability** — `class_create` lost its `struct module *` argument in Linux
  6.4; the code is version-guarded so it builds on old and new kernels alike.

## Build & use

```bash
make                                   # builds ch_drv.ko against your running kernel
sudo insmod ch_drv.ko && dmesg | tail  # load; watch the log
echo hi | sudo tee /dev/mychdev        # triggers write()
sudo cat /dev/mychdev                  # triggers read()
sudo rmmod ch_drv                      # unload
```

**Requirements:** kernel headers for your running kernel
(`/lib/modules/$(uname -r)/build`).

## Files

| File | Role |
|------|------|
| `ch_drv.c` | the module: fops, device/class setup, teardown |
| `Makefile` | out-of-tree kbuild wrapper |
