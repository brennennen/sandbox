#!/usr/bin/env bash
##
# The goal with this repo is to create a simple ARM embedded linux kernal image
# that targets the STM32MP157 dev board with busybox using yocto and some small
# sandbox drivers to explore linux device drivers on actual hardware.
#
# This script copies everything over to a destination directory and runs bitbake
# there to save disk space. The supplied destination directory needs to have +100GB
# of free space (yocto is very disk space intensive)
#
# Example usage: ./build.bash /mnt/d_wsl/temp/mp157sandbox /mnt/d_wsl/yocto
#

if [ "$#" -ne 2 ]; then
	echo "Usage: $0 {build dir} {caches dir}"
	exit 1
fi

OUT_DIR="$1"
CACHES_DIR="$2"
SCRIPT_PATH="$(realpath "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"

mkdir -p ${OUT_DIR}
mkdir -p ${OUT_DIR}/build
mkdir -p ${OUT_DIR}/build/conf
cd ${OUT_DIR}

# Copy over the layer, meta package files, source code for our business logic
# In production scenarios, you'd use "fetcher" commands that pull from repos, I'm
# skipping that step to keep this as self-contained as possible.
rsync -au --delete ${SCRIPT_DIR}/meta-mp157sandbox ${OUT_DIR}/
# conf needs to be in the build folder
cp ${SCRIPT_DIR}/conf/local.conf ${OUT_DIR}/build/conf/local.conf

# tar up the hello source, bitbake likes "md5" hashable things.
# bitbake also expects files to be in a "files" or double-up named project folder by convention
cd ${OUT_DIR}
mkdir -p ./meta-mp157sandbox/recipes-mp157sandbox/hello/files/
cd ${OUT_DIR}/meta-mp157sandbox/recipes-mp157sandbox/hello/
tar -cpf files/src.tar src
cd ${OUT_DIR}

# Clone down yocto with a specific branch
if [ ! -d ${OUT_DIR}/poky ] ; then
    git clone -b styhead https://github.com/yoctoproject/poky.git
fi

# Clone down the oe layers (stm32 layers depend on oe layers)
if [ ! -d ${OUT_DIR}/meta-st-stm32mp ] ; then
    git clone -b styhead git://git.openembedded.org/meta-openembedded
fi

# Clone down the stm32 layer
if [ ! -d ${OUT_DIR}/meta-st-stm32mp ] ; then
    git clone -b styhead https://github.com/STMicroelectronics/meta-st-stm32mp.git
fi

# TODO: consider adding OpenSTLinux too? "meta-st-openstlinux"

cd ${OUT_DIR}
source ./poky/oe-init-build-env ./build # moves cwd to ./build dir, adds yocto tools to path, sets various env variables

# Set the machine
#export MACHINE="stm32mp15-disco"
export MACHINE="stm32mp1"

# Set cache directories to be shared with other yocto builds on the system (saves disk space).
export DL_DIR="${CACHES_DIR}/downloads"
export SSTATE_DIR="${CACHES_DIR}/sstate"
export TMPDIR="${CACHES_DIR}/tmp"

# Add layers
bitbake-layers show-layers | grep "meta-oe" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	bitbake-layers add-layer ../meta-openembedded/meta-oe
fi

bitbake-layers show-layers | grep "meta-python" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	bitbake-layers add-layer ../meta-openembedded/meta-python
fi

bitbake-layers show-layers | grep "meta-st-stm32mp" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	bitbake-layers add-layer ../meta-st-stm32mp
fi

bitbake-layers show-layers | grep "meta-mp157sandbox" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	bitbake-layers add-layer ../meta-mp157sandbox
fi

# if the build is in a bad state:   bitbake -c cleanall core-image-hello
bitbake core-image-mp157sandbox
# Build sdk for userspace applications and device drivers
#bitbake core-image-mp157sandbox -c populate_sdk

# Create the sd card flash image
# STM provided image creation script creates the linux rootfs with 5GB of space, and `dd` takes forever.
#./tmp/deploy/images/stm32mp1/scripts/create_sdcard_from_flashlayout.sh ./tmp/deploy/images/stm32mp1/flashlayout_core-image-mp157sandbox/opteemin/FlashLayout_sdcard_stm32mp157d-dk1-opteemin.tsv
# TODO: create a separate script that involves a smaller rootfs to make iteration quicker
# scp -P 9022 ./tmp/deploy/images/stm32mp1/FlashLayout_sdcard_stm32mp157d-dk1-opteemin.raw 192.168.1.144:~/FlashLayout_sdcard_stm32mp157d-dk1-opteemin.raw
echo "Clone the image to an sd card (NOTE: set 'of=/dev/???' before running, check dmesg):"
# sudo dd if=./tmp/deploy/images/stm32mp1/FlashLayout_sdcard_stm32mp157d-dk1-opteemin.raw of=/dev/sdc bs=32M conv=fdatasync status=progress
echo "sudo dd if=./tmp/deploy/images/stm32mp1/FlashLayout_sdcard_stm32mp157d-dk1-opteemin.raw of=/dev/??? bs=32M conv=fdatasync status=progress"
# Before installing the card, plug in the micro-usb STLink port for logs over the serial port.
echo "Connect a micro usb the to STLink port and run a serial comm to see boot logs before powering on board:"
echo "sudo picocom -b 115200 /dev/ttyACM0"
