#!/usr/bin/env bash
##
# The goal with this repo is to create a simple ARM embedded linux kernal image
# that targets QEMU with busybox using yocto and a small "business logic" program.
#
# This script copies everything over to a destination directory and runs bitbake
# from there instead (out of this repos tree) because of how much disk space
# building images with yocto/oe can take up.
#

#set -x
#set -e

OUT_DIR="/mnt/d_wsl/temp/simple_arm_yocto"
SCRIPT_PATH="$(realpath "$0")"
SCRIPT_DIR="$(dirname "$SCRIPT_PATH")"
EXTERNAL_DIR="${SCRIPT_DIR}/external"

mkdir -p ${OUT_DIR}
mkdir -p ${OUT_DIR}/build
mkdir -p ${OUT_DIR}/build/conf
cd ${OUT_DIR}

# Copy over the layer, meta package files, source code for our business logic
# In production scenarios, you'd use "fetcher" commands that pull from repos, I'm
# skipping that step to keep this as self-contained as possible.
rsync -au --delete ${SCRIPT_DIR}/meta-hello ${OUT_DIR}/
# conf needs to be in the build folder
cp ${SCRIPT_DIR}/conf/local.conf ${OUT_DIR}/build/conf/local.conf

# tar up the hello source, bitbake likes "md5" hashable things.
# bitbake also expects files to be in a "files" or double-up named project folder by convention
cd ${OUT_DIR}
mkdir -p ./meta-hello/recipes-hello/hello/files/
cd ${OUT_DIR}/meta-hello/recipes-hello/hello/
tar -cpf files/src.tar src
cd ${OUT_DIR}


# Clone down yocto with a specific branch
if [ ! -d ${OUT_DIR}/poky ] ; then
    git clone -b styhead https://github.com/yoctoproject/poky.git
fi

cd ${OUT_DIR}
source ./poky/oe-init-build-env # moves cwd to ./build dir, adds yocto tools to path, sets various env variables

bitbake-layers show-layers | grep "meta-hello" > /dev/null
layer_info=$?
if [ $layer_info -ne 0 ];then
	bitbake-layers add-layer ../meta-hello
fi

# if the build is in a bad state:   bitbake -c cleanall core-image-hello
bitbake core-image-hello
echo "Run the commands below to emulate the image produced (user/pass: root):"
echo "cd ${OUT_DIR} && source ./poky/oe-init-build-env && runqemu"

# TODO: incorporate setting QB_SLIRP_OTP to allow for ssh and other port forwarding
# source poky/oe-init-build-env
# export QB_SLIRP_OPT="-netdev user,id=net0,hostfwd=tcp::10022-:22,hostfwd=tcp::9000-:9000"
# runqemu slirp nographic
