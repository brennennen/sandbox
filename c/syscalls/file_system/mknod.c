/*
Build:
(reading and writing to /dev/ requires admin access)
```
gcc ./mknod.c \
    && sudo ./a.out \
    && sudo rm -f /tmp/my_test_text_file \
    && rm -f /tmp/my_fifo \
    && sudo rm /dev/my_fake_device
```


`mknod` creates a file system nodes. For regular text/binary files and directories, 
`open`, `creat`, and `mkdir` are preferred. `mknod` is usually just used for "special" 
files, such as "device" files or "fifo" (aka named pipes) files.

Devices Guidance: https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/Documentation/admin-guide/devices.txt

Below is a small test program that does the following:
* Creates a regular text file and writes "hello world!" to it.
* Creates a FIFO
* Creates a device file mapped to a random character generator and requests 4 bytes from it.
*/

#include <errno.h> // errno
#include <fcntl.h> // open
#include <stdio.h> // printf
#include <string.h> // strerror
#include <sys/stat.h> // mknod
#include <sys/sysmacros.h> // makedev
#include <unistd.h> // read

void make_txtfile() {
    // NOTE: don't do this, use creat or open instead. This is just for understanding mknod.
    printf("making regular txtfile...\n");
    int mknod_result = mknod("/tmp/my_test_text_file", S_IFREG | 0777, 0);
    if (mknod_result == -1) {
        printf("Failed to create text file: %s\n", strerror(errno));
        return;
    }
    int text_fd = open("/tmp/my_test_text_file", O_RDWR);
    ssize_t write_result = write(text_fd, "hello world!\n", strlen("hello world!\n"));
    if (write_result == -1) {
        printf("Failed to write to '/tmp/my_test_text_file'\n");
        return;
    }
    printf("end making regular txtfile\n");
}

void make_fifo() {
    printf("making fifo...\n");
    mknod("/tmp/my_fifo", S_IFIFO | 0644 , 0);
    struct stat sb = {0};
    stat("my_fifo", &sb);

    // FIFO/PIPE testing code requires multiple processes/threads which would bloat this
    // file. See "pipe.c" or "fifo.c" for actual examples.

    if (S_ISFIFO(sb.st_mode) == 0) {
        printf("   is fifo\n");
    }
    printf("end making fifo\n");
}

void make_device() {
    printf("making device file...\n");
    int major = 1; // major 1 = character devices
    int minor = 8; // minor 8 = random number gen
    mode_t mode = S_IFCHR | 0666; // Character device with read/write

    // Essentially creates a duplicate of /dev/random.
    int mknod_result = mknod("/dev/my_fake_device", mode, makedev(1, 8));
    if (mknod_result == -1) {
        printf("Failed to create device file: %s\n", strerror(errno));
        return;
    }

    char buffer[4];
    int my_fake_device_fd = open("/dev/my_fake_device", O_RDONLY);
    ssize_t read_result = read(my_fake_device_fd, &buffer, 4);
    
    if (read_result == -1) {
        printf("Failed to read from created device.\n");
        return;
    }

    int my_num = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24);
    printf("reading 4 bytes from my_fake_device: %d\n", my_num);
    printf("end making device file\n");
}

int main(int argc, char* argv[]) {
    printf("Entering kill main...\n");
    make_txtfile();
    make_fifo();
    make_device();
    printf("Exiting kill main...\n"); // Never called because we un-alived ourselves.
}
