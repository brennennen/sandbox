#!/usr/bin/env bash
##
# The goal with this repo is to create a minimal ARM embedded linux kernal image
# that targets QEMU with busybox using buildroot.
#

#set -x
set -e

# don't put in /tmp, runs out of inodes.
OUT_DIR="$HOME/temp/simple_arm_buildroot"
SCRIPT_PATH="$(realpath "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
EXTERNAL_DIR="${SCRIPT_DIR}/external"

mkdir -p ${OUT_DIR}
mkdir -p ${OUT_DIR}/.build
cd ${OUT_DIR}


# Clone down buildroot, maybe checkout a specific branch.
if [ ! -d ${OUT_DIR}/buildroot ] ; then
    git clone https://github.com/buildroot/buildroot.git
fi

# Build up the initial configuration. In a production environment, you wouldn't rebuild
# this from scratch everytime, you'd version control this defconfig, but you also wouldn't
# update buildroot very often and I'd like to keep this script focused on the latest.
if [ ! -e ${OUT_DIR}/my_arm_qemu_defconfig ] ; then
    cd ${OUT_DIR}/buildroot
    # use "qemu_aarch64_virt_defconfig" as the basis for our defconfig
    # found this config by digging around buildroot/config folder.
    # doesn't look like it actually cross compiles though. may need to setup a custom toolchain...
    make qemu_aarch64_virt_defconfig BR2_EXTERNAL=${EXTERNAL_DIR} O=${OUT_DIR}/.build

    # probably best to run everything in this if block manually and use menuconfig to add
    # what is needed.
    # make menuconfig BR2_DEFCONFIG=./../my_arm_qeum_defconfig O=./../.build
    make savedefconfig BR2_DEFCONFIG=${OUT_DIR}/my_arm_qemu_defconfig O=${OUT_DIR}/.build

    # The below mimicks selecting dropbear and enabling a root password.
    echo "BR2_TARGET_GENERIC_ROOT_PASSWD=\"root\"" >> ${OUT_DIR}/my_arm_qemu_defconfig
    echo "BR2_PACKAGE_DROPBEAR=y" >> ${OUT_DIR}/my_arm_qemu_defconfig
    echo "BR2_PACKAGE_HELLO=y" >> ${OUT_DIR}/my_arm_qemu_defconfig
fi

# Copy over external package to a known location. Typically you'd use a site method that would
# pull down the code from somewhere, to keep this demo self contained, I'm just copying the
# folder over.
mkdir -p ${OUT_DIR}/hello/src
cp -r ${SCRIPT_DIR}/external/packages/hello/src/* ${OUT_DIR}/hello/src


# Build buildroot
cd ${OUT_DIR}/buildroot
make BR2_EXTERNAL=${EXTERNAL_DIR} BR2_DEFCONFIG=${OUT_DIR}/my_arm_qemu_defconfig O=${OUT_DIR}/.build

# A sdk/toolchain can be generated and distributed to team members building the userland
# business logic of the system.
#make sdk

cd ${OUT_DIR}

echo "run the command below to start qemu and enter the vm:"
echo "qemu-system-aarch64 \
    -M virt  \
    -cpu cortex-a53 -nographic -smp 1 \
    -kernel ./.build/images/Image \
    -append "rootwait root=/dev/vda console=ttyAMA0" \
    -netdev user,id=eth0,hostfwd=tcp::10022-:22 \
    -device virtio-net-device,netdev=eth0 \
    -drive file=./.build/images/rootfs.ext4,if=none,format=raw,id=hd0 \
    -device virtio-blk-device,drive=hd0 -device virtio-rng-pci"
