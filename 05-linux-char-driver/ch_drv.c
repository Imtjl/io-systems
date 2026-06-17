// SPDX-License-Identifier: GPL-2.0
/*
 * ch_drv — a minimal Linux character device driver.
 *
 * Registers a char device, creates /dev/mychdev via a device class, and wires
 * up the open/release/read/write file operations. Each call is logged to the
 * kernel ring buffer (see `dmesg`). This is the "inside the OS" end of the I/O
 * stack: the same read()/write() syscalls a user program issues land in the
 * callbacks below.
 *
 * Build:   make
 * Load:    sudo insmod ch_drv.ko && dmesg | tail
 * Use:     echo hi | sudo tee /dev/mychdev ; sudo cat /dev/mychdev
 * Unload:  sudo rmmod ch_drv
 */
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/version.h>

static dev_t first;
static struct cdev c_dev;
static struct class *cl;

static int my_open(struct inode *i, struct file *f) {
    printk(KERN_INFO "ch_drv: open()\n");
    return 0;
}

static int my_close(struct inode *i, struct file *f) {
    printk(KERN_INFO "ch_drv: close()\n");
    return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "ch_drv: read()\n");
    return 0; /* EOF — nothing to hand back yet */
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len,
                        loff_t *off) {
    printk(KERN_INFO "ch_drv: write() %zu bytes\n", len);
    return len; /* pretend we consumed everything */
}

static struct file_operations mychdev_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write,
};

static int __init ch_drv_init(void) {
    printk(KERN_INFO "ch_drv: loading\n");

    if (alloc_chrdev_region(&first, 0, 1, "ch_dev") < 0)
        return -1;

/* class_create lost its (struct module *) parameter in Linux 6.4. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    cl = class_create("chardrv");
#else
    cl = class_create(THIS_MODULE, "chardrv");
#endif
    if (IS_ERR_OR_NULL(cl)) {
        unregister_chrdev_region(first, 1);
        return -1;
    }

    if (IS_ERR_OR_NULL(device_create(cl, NULL, first, NULL, "mychdev"))) {
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -1;
    }

    cdev_init(&c_dev, &mychdev_fops);
    if (cdev_add(&c_dev, first, 1) == -1) {
        device_destroy(cl, first);
        class_destroy(cl);
        unregister_chrdev_region(first, 1);
        return -1;
    }
    return 0;
}

static void __exit ch_drv_exit(void) {
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "ch_drv: unloaded\n");
}

module_init(ch_drv_init);
module_exit(ch_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Boris Dvorkin");
MODULE_DESCRIPTION("Minimal character device driver (IO-Systems course)");
