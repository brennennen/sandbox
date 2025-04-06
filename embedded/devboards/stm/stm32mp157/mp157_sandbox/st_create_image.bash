#!/usr/bin/env bash
# Scraped the sgdisk and dd commands out of the st reference image creation script 
# to build a minimal image creation script.
# See: {yocto_build_dir}/tmp/deploy/images/stm32mp1/scripts/create_sdcard_from_flashlayout.sh
machine="stm32mp1"
image_file="./mp157_sandbox.img"

dd if=/dev/zero of=${image_file} bs=1024 count=0 seek=5120K
sgdisk -og -a 1 ${image_file}

sgdisk -a 1 -n 1:34:545 -c 1:fsbl1 -t 1:8301 ${image_file}
sgdisk -a 1 -n 2:546:1057 -c 2:fsbl2 -t 2:8301 ${image_file}
sgdisk -a 1 -n 3:1058:1569 -c 3:metadata1 -t 3:8a7a84a0-8387-40f6-ab41-a8b9a5a60d23 ${image_file}
sgdisk -a 1 -n 4:1570:2081 -c 4:metadata2 -t 4:8a7a84a0-8387-40f6-ab41-a8b9a5a60d23 ${image_file}
sgdisk -a 1 -n 5:2082:10273 -c 5:fip-a -t 5:19d5df83-11b0-457b-be2c-7559c13142a5 -u 5:4fd84c93-54ef-463f-a7ef-ae25ff887087 ${image_file}
sgdisk -a 1 -n 6:10274:18465 -c 6:fip-b -t 6:19d5df83-11b0-457b-be2c-7559c13142a5 -u 6:09c54952-d5bf-45af-acee-335303766fb3 ${image_file}
sgdisk -a 1 -n 7:18466:19489 -c 7:u-boot-env -t 7:3de21764-95bd-54bd-a5c3-4abe786f38a8 ${image_file}
sgdisk -a 1 -n 8:19490:150561 -c 8:bootfs -t 8:8300  -A 8:set:2 ${image_file}
sgdisk -a 1 -n 9:150562:183329 -c 9:vendorfs -t 9:8300 ${image_file}
sgdisk -a 1 -n 10:183330:8571937 -c 10:rootfs -t 10:8300  -u 10:e91c4e10-16e6-4c0e-bd0e-77becf4a3582 ${image_file}
sgdisk -a 1 -n 11:8571938:  -c 11:userfs -t 11:8300 ${image_file}

yocto_images_dir=./tmp/deploy/images/${machine}/
dd if=${yocto_images_dir}/arm-trusted-firmware/tf-a-stm32mp157d-dk1-opteemin-sdcard.stm32 of=${image_file} conv=fdatasync,notrunc seek=1 bs=17408
dd if=${yocto_images_dir}/arm-trusted-firmware/tf-a-stm32mp157d-dk1-opteemin-sdcard.stm32 of=${image_file} conv=fdatasync,notrunc seek=1 bs=279552
dd if=${yocto_images_dir}/arm-trusted-firmware/metadata.bin of=${image_file} conv=fdatasync,notrunc seek=1 bs=541696
dd if=${yocto_images_dir}/arm-trusted-firmware/metadata.bin of=${image_file} conv=fdatasync,notrunc seek=1 bs=803840
dd if=${yocto_images_dir}/fip/fip-stm32mp157d-dk1-opteemin-sdcard.bin of=${image_file} conv=fdatasync,notrunc seek=1 bs=1065984
# fip-b is empty
# u-boot-env is empty
dd if=${yocto_images_dir}/st-image-bootfs-nodistro-stm32mp1.bootfs.ext4 of=${image_file} conv=fdatasync,notrunc seek=1 bs=9978880
dd if=${yocto_images_dir}/st-image-vendorfs-nodistro-stm32mp1.vendorfs.ext4 of=${image_file} conv=fdatasync,notrunc seek=1 bs=77087744
dd if=${yocto_images_dir}/core-image-mp157sandbox-stm32mp1.rootfs.ext4 of=${image_file} conv=fdatasync,notrunc seek=1 bs=93864960
dd if=${yocto_images_dir}/st-image-userfs-nodistro-stm32mp1.userfs.ext4 of=${image_file} conv=fdatasync,notrunc seek=1 bs=4388832256
