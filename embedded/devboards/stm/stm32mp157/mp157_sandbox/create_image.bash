#!/usr/bin/env bash
##
# Creates a simple STM32MP1 bootable image from a yocto build. Simpler than the provided
# reference image creation script and excludes out-of-the box code to allow for a much
# smaller image (less time waiting to clone it to an sd card). Assumes 512 byte sectors.
#
# References:
# * STM reference/demo image creation script: "{yocto_out_dir}/build/tmp/deploy/images/stm32mp15-eval/FlashLayout_sdcard_stm32mp157f-ev1-opteemin.how_to_update.txt"
# * STM32MP1 Platform Boot doc: https://www.st.com/content/ccc/resource/training/technical/product_training/group1/ac/45/53/56/1f/0b/47/68/STM32MP1-Software-Platform_boot_BOOT/files/STM32MP1-Software-Platform_boot_BOOT.pdf/_jcr_content/translations/en.STM32MP1-Software-Platform_boot_BOOT.pdf
#

set -e
set -x

machine="stm32mp1"
image_file="./mp157_sandbox.img"

dd if=/dev/zero of=./${image_file} bs=1M count=1024 # ~1GB
sgdisk --mbrtogpt ${image_file}

#
# MARK: Setup partitions
#

# fsbl1
fsbl1_start=34 # set first sector we write the first partition to past the gpt header
fsbl1_size=512 # +256KiB
fsbl1_end=$((fsbl1_start + fsbl1_size - 1))
sgdisk --set-alignment 1 --new 1:${fsbl1_start}:${fsbl1_end} --change-name 1:fsbl1 \
    --typecode 1:8301 ${image_file}

# fsbl2
fsbl2_start=$((fsbl1_end + 1))
fsbl2_size=512 # +256KiB
fsbl2_end=$((fsbl2_start + fsbl2_size - 1))
sgdisk --set-alignment 1 --new 2:${fsbl2_start}:${fsbl2_end} --change-name 2:fsbl2 \
    --typecode 2:8301 ${image_file}

# metadata1
metadata1_start=$((fsbl2_end + 1))
metadata1_size=512 # +256KiB
metadata1_end=$((metadata1_start + metadata1_size - 1))
sgdisk --set-alignment 1 --new 3:${metadata1_start}:${metadata1_end} --change-name 3:metadata1 \
    --typecode 3:8a7a84a0-8387-40f6-ab41-a8b9a5a60d23 ${image_file}

# metadata2
metadata2_start=$((metadata1_end + 1))
metadata2_size=512 # +256KiB
metadata2_end=$((metadata2_start + metadata2_size - 1))
sgdisk --set-alignment 1 --new 4:${metadata2_start}:${metadata2_end} --change-name 4:metadata2 \
    --typecode 4:8a7a84a0-8387-40f6-ab41-a8b9a5a60d23 ${image_file}

# fip-a
fip_a_start=$((metadata2_end + 1))
fip_a_size=8192 # +4MiB
fip_a_end=$((fip_a_start + $fip_a_size - 1))
sgdisk --set-alignment 1 --new 5:${fip_a_start}:${fip_a_end} --change-name 5:fip-a \
    --partition-guid 5:4fd84c93-54ef-463f-a7ef-ae25ff887087 \
    --typecode 5:19d5df83-11b0-457b-be2c-7559c13142a5 \
    ${image_file}

# fip-b
fip_b_start=$((fip_a_end + 1))
fip_b_size=8192 # +4MiB
fip_b_end=$((fip_b_start + $fip_b_size - 1))
sgdisk --set-alignment 1 --new 6:${fip_b_start}:${fip_b_end} --change-name 6:fip-b \
    --partition-guid 6:09c54952-d5bf-45af-acee-335303766fb3 \
    --typecode 6:19d5df83-11b0-457b-be2c-7559c13142a5 \
    ${image_file}

# u-boot-env
u_boot_env_start=$((fip_b_end + 1))
u_boot_env_size=1024 # +512KiB
u_boot_env_end=$((u_boot_env_start + $u_boot_env_size - 1))
sgdisk --set-alignment 1 --new 7:${u_boot_env_start}:${u_boot_env_end} --change-name 7:u-boot-env \
    --typecode 7:3de21764-95bd-54bd-a5c3-4abe786f38a8 \
    ${image_file}

# bootfs
bootfs_start=$((u_boot_env_end + 1))
bootfs_size=131072 # +64MiB
bootfs_end=$((bootfs_start + $bootfs_size - 1))
sgdisk --set-alignment 1 --new 8:${bootfs_start}:${bootfs_end} --change-name 8:bootfs \
    --typecode 8:8300 --attributes=8:set:2 ${image_file}

# rootfs - default bootfs config points to rootfs through hard-coded uuid (bootfs/mmc0_extlinux/extlinux.conf)
rootfs_start=$((bootfs_end + 1))
rootfs_size=1048576 # +512MiB
rootfs_end=$((rootfs_start + $rootfs_size - 1))
sgdisk --set-alignment 1 --new 9:${rootfs_start}:${rootfs_end} --change-name 9:rootfs \
    --typecode 9:8300 \
    --partition-guid 9:e91c4e10-16e6-4c0e-bd0e-77becf4a3582 \
    ${image_file}
#
# MARK: Write images
#
yocto_images_dir=./tmp/deploy/images/${machine}/

# fsb1
dd if=${yocto_images_dir}/arm-trusted-firmware/tf-a-stm32mp157d-dk1-opteemin-sdcard.stm32 \
    of=${image_file} \
    conv=fdatasync,notrunc seek=1 bs=$(($fsbl1_start * 512))
# fsb2
dd if=${yocto_images_dir}/arm-trusted-firmware/tf-a-stm32mp157d-dk1-opteemin-sdcard.stm32 \
    of=${image_file} \
    conv=fdatasync,notrunc seek=1 bs=$(($fsbl2_start * 512))
# metadata1
dd if=${yocto_images_dir}/arm-trusted-firmware/metadata.bin \
    of=${image_file} \
    conv=fdatasync,notrunc seek=1 bs=$(($metadata1_start * 512))
# metadata2
dd if=${yocto_images_dir}/arm-trusted-firmware/metadata.bin \
    of=${image_file} \
    conv=fdatasync,notrunc seek=1 bs=$(($metadata2_start * 512))
# fip-a
dd if=${yocto_images_dir}/fip/fip-stm32mp157d-dk1-opteemin-sdcard.bin \
    of=${image_file} \
    conv=fdatasync,notrunc seek=1 bs=$(($fip_a_start * 512))
# fip-b - empty
# u-boot-env - empty
# bootfs
dd if=${yocto_images_dir}/st-image-bootfs-nodistro-stm32mp1.bootfs.ext4 \
    of=${image_file} \
    conv=fdatasync,notrunc seek=1 bs=$(($bootfs_start * 512))
# rootfs
dd if=${yocto_images_dir}/core-image-mp157sandbox-stm32mp1.rootfs.ext4 \
    of=${image_file} \
    conv=fdatasync,notrunc seek=1 bs=$(($rootfs_start * 512))

sgdisk --print $image_file
sgdisk --verify $image_file
# losetup --detach $loop_device
echo "create_image:done"
