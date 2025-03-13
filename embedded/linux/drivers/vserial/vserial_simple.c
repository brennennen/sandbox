/**
 * Toy virtual serial character device driver exploration to learn linux device drivers.
 * Exposes a virtual serial device (just some memory) `/dev/vserial` that can be written to
 * and read from.
 *
 * Writes accumulate in a large 4096 byte buffer (ex: write("hello") and write("world") would
 * result in the buffer containing: "helloworld\0") and to keep things simple, reads are
 * assumed to consume the entire buffer (circular buffer logic will be covered in a different
 * toy/exploration).
 *
 * Note: most unix tools (like echo) expect to always be able to write the full amount of data
 * and will constantly retry if they are unable to (such as if the buffer is full). The echo
 * command will appear to hang and this will also flood your kernel logs if you log in this
 * path. Reading will empty the buffer and "unhang" the process, and adding rate limiting to
 * any logging in this path should be considered.
```sh
# build/install/create
make
sudo su
echo "7" > /proc/sys/kernel/printk
insmod vserial_simple.ko
mknod /dev/vserial_simple c $(cat /proc/devices | grep vserial_simple | awk '{print $1}') 0

# test
echo -n "hello" > /dev/vserial_simple
cat /dev/vserial_simple
dmesg

# cleanup
rmmod vserial_simple
rm /dev/vserial_simple
```
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "vserial_simple"

#define BUFFER_SIZE 4096

static char *vserial_buffer;
static int buffer_count = 0;
static dev_t dev_num;
static struct cdev vserial_cdev;
static DEFINE_MUTEX(vserial_mutex);

static int vserial_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "vserial: device opened\n");
    return 0;
}

static int vserial_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "vserial: device closed\n");
    return 0;
}

static ssize_t vserial_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    mutex_lock(&vserial_mutex);
    // If nothing has been written, return early.
    if (buffer_count == 0) {
        printk(KERN_INFO "vserial: nothing to read.\n");
        mutex_unlock(&vserial_mutex);
        return 0;
    }

    // Don't allow reading more than has been written.
    if (count > buffer_count) {
        count = buffer_count;
    }

    if (copy_to_user(buf, vserial_buffer, count)) {
        printk(KERN_ALERT "vserial: Failed copy_to_user.\n");
        mutex_unlock(&vserial_mutex);
        return -EFAULT;
    }

    buffer_count = 0;
    mutex_unlock(&vserial_mutex);
    printk(KERN_INFO "vserial: read %d bytes\n", count);
    return count;
}

static ssize_t vserial_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    mutex_lock(&vserial_mutex);

    // Truncate write amount if the buffer is full.
    if (buffer_count + count > BUFFER_SIZE - 1) {
        count = BUFFER_SIZE - buffer_count - 1;
        if (count <= 0) {
            printk_ratelimited(KERN_INFO "vserial: buffer full.\n");
            mutex_unlock(&vserial_mutex);
            return 0;
        } else {
            printk(KERN_INFO "vserial: truncating write amount: %ld, buffer full.\n", count);
        }
    }

    if (copy_from_user(vserial_buffer + buffer_count, buf, count)) {
        printk(KERN_ALERT "vserial: copy_from_user failed\n");
        mutex_unlock(&vserial_mutex);
        return -EFAULT;
    }

    buffer_count = buffer_count + count;
    vserial_buffer[buffer_count] = '\0';
    mutex_unlock(&vserial_mutex);
    printk(KERN_INFO "vserial: Wrote %ld bytes. buffer_count: %d, buffer: %s\n",
        count, buffer_count, vserial_buffer);
    return count;
}

static struct file_operations vserial_fops = {
    .open = vserial_open,
    .read = vserial_read,
    .write = vserial_write,
    .release = vserial_release,
};

static int __init vserial_init(void)
{
    int result = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (result < 0) {
        printk(KERN_ALERT "vserial: Failed to allocate character device region\n");
        return result;
    }

    cdev_init(&vserial_cdev, &vserial_fops);
    result = cdev_add(&vserial_cdev, dev_num, 1);
    if (result < 0) {
        printk(KERN_ALERT "vserial: failed to add character device\n");
        unregister_chrdev_region(dev_num, 1);
        return result;
    }

    vserial_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!vserial_buffer) {
        printk(KERN_ALERT "vserial: failed to allocate buffer\n");
        cdev_del(&vserial_cdev);
        unregister_chrdev_region(dev_num, 1);
        return -ENOMEM;
    }
    vserial_buffer[0] = '\0';
    buffer_count = 0;

    printk(KERN_INFO "vserial: Device registered (major: %d, minor: %d)\n", MAJOR(dev_num), MINOR(dev_num));
    return 0;
}

static void __exit vserial_exit(void)
{
    kfree(vserial_buffer);
    cdev_del(&vserial_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "vserial: Device unregistered\n");
}

module_init(vserial_init);
module_exit(vserial_exit);

MODULE_AUTHOR("Brennen Sprimont <brennen.j.sprimont@gmail.com>");
MODULE_DESCRIPTION("Virtual serial driver");
MODULE_LICENSE("GPL");
