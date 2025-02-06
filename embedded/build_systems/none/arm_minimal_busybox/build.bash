#!/usr/bin/env bash
##
# The goal with this repo is to create a minimal ARM embedded linux kernal image with
# busybox from scratch without buildroot or yocto that targets QEMU.
#
# Assumptions:
# * all tooling needing to build the linux kernel is installed
# * A cross-compile toolchain for ARM is installed
# * qemu is installed (sudo apt install qemu-system)
#

OUT_DIR="/tmp/arm_minimal_busybox"
ARCH="arm64"
TOOLCHAIN_DIR="/home/b/toolchains/arm-gnu-toolchain-13.3.rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/"
CROSS_COMPILE="aarch64-none-linux-gnu-"

# Clear out the old rootfs if one is present.
sudo rm -rf ${OUT_DIR}/rootfs
sudo rm -f ${OUT_DIR}/initramfs.cpio.gz

# Build the example hello.c program
${CROSS_COMPILE}gcc ./hello.c
cp ./a.out ${OUT_DIR}

# Copy the init script to the output area.
cp ./init ${OUT_DIR}

# Create a directory for the build artifacts and root filesystem
mkdir -p ${OUT_DIR}
mkdir -p ${OUT_DIR}/rootfs
cd ${OUT_DIR}/rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log
cp ${OUT_DIR}/init ${OUT_DIR}/rootfs/
cp ${OUT_DIR}/a.out ${OUT_DIR}/rootfs/a.out
chmod +x ${OUT_DIR}/rootfs/init

if [ ! -e ${OUT_DIR}/Image ]; then
    echo "Building linux kernel..."
    cd ${OUT_DIR}
    git clone git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git --depth 1 --single-branch --branch v6.13
    cd linux-stable
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    # for qemu, can use default arm defconfig. for specific boards see below:
    # make ARCH=arm help | grep defconfig
    # need this? CONFIG_BLK_DEV_INITRD=y CONFIG_BINFMT_SCRIPT=y
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
    cp ${OUT_DIR}/linux-stable/arch/${ARCH}/boot/Image ${OUT_DIR}/Image
fi

if [ ! -e ${OUT_DIR}/busybox/init/init.o ]; then
    echo "Building busybox..."
    cd ${OUT_DIR}
    git clone git://busybox.net/busybox.git
    cd busybox
    git checkout 1_36_stable
    make distclean
    make defconfig
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
fi
cd ${OUT_DIR}/busybox
make CONFIG_PREFIX=${OUT_DIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install

# Find and add the program interpeter required to run busybox
# ex: aarch64-none-linux-gnu-readelf -a bin/busybox | grep "program interpreter"
# output: /lib/ld-linux-aarch64.so.1
# this library should be in the toolchain package.
cd ${OUT_DIR}/rootfs
toolchain_libc_dir=$(${CROSS_COMPILE}gcc -print-sysroot)
cp ${toolchain_libc_dir}/lib/ld-linux-aarch64.so.1 ./lib/

# Find and add any libraries/dependencies required to run busybox
# ex: aarch64-none-linux-gnu-readelf -a bin/busybox | grep "Shared library"
# output: libm.so.6, libresolv.so.2, libc.so.6
# These files should be located in your toolchains package.
# ex: find ~/toolchains/ -name "libm.so.6"
cp ${toolchain_libc_dir}/lib64/libm.so.6 ./lib64/
cp ${toolchain_libc_dir}/lib64/libresolv.so.2 ./lib64/
cp ${toolchain_libc_dir}/lib64/libc.so.6 ./lib64/

# Build device tree
cd ${OUT_DIR}/rootfs
sudo mknod -m 666 ./dev/null c 1 3
sudo mknod -m 666 ./dev/console c 5 1

# Build root filesystem
cd ${OUT_DIR}
rm -f ./initramfs.cpio.gz
sudo chown -R root:root ${OUT_DIR}/rootfs
cd ${OUT_DIR}/rootfs
find . | cpio -H newc -ov --owner root:root > ${OUT_DIR}/initramfs.cpio
cd ${OUT_DIR}
gzip -f initramfs.cpio

echo "run the command below to start qemu and enter the vm:"
echo "qemu-system-arm64 \
    -m 256M \
    -M virt \
    -cpu cortex-a53 \
    -nographic \
    -smp 1 \
    -kernel ${OUT_DIR}/Image \
    -chardev stdio,id=char0,mux=on,logfile=${OUT_DIR}/serial.log,signal=off \
    -serial chardev:char0 \
    -mon chardev=char0 \
    -append "console=ttyAMA0" \
    -initrd ${OUT_DIR}/initramfs.cpio.gz"
