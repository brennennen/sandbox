/**
 * Example from LDD3, updated to work on kernel version: 6.13.5
```
make all
su
insmod hello.ko
dmesg
rmmod hello
dmesg
```
 */

#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");

static int hello_init(void) {
    printk(KERN_ALERT "hello, world\n");
    // struct task_struct *current
    printk(KERN_ALERT "the process is \"%s\" (pid %i)\n",
        current->comm, current->pid);
    return 0;
}

static void hello_exit(void) {
    printk(KERN_ALERT "Goodbye, cruel world!\n");
}

module_init(hello_init);
module_exit(hello_exit);
