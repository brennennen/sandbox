# STM32MP1
* Dual Core MPU: ARM Cortex A7 MPU 800MHz 32 bit + ARM Cortex-M4 32 bit

# Resources
* ST Resources
  * OE Distribution Package - https://wiki.st.com/stm32mpu/wiki/STM32MPU_Distribution_Package
  * Datasheet -
  * Data brief - https://www.st.com/resource/en/data_brief/stm32mp157f-dk2.pdf
  * Getting started - https://forum.digikey.com/t/debian-getting-started-with-the-stm32mp157/12459
  * Product page - https://www.digikey.com/en/products/detail/stmicroelectronics/STM32MP157D-DK1/13536964
  * ST provided yocto layer - https://github.com/STMicroelectronics/meta-st-stm32mp
  * workshop - https://www.youtube.com/watch?v=yD8M_UK4AgQ
  * STM32MP1 Platform Boot - https://www.st.com/content/ccc/resource/training/technical/product_training/group1/ac/45/53/56/1f/0b/47/68/STM32MP1-Software-Platform_boot_BOOT/files/STM32MP1-Software-Platform_boot_BOOT.pdf/_jcr_content/translations/en.STM32MP1-Software-Platform_boot_BOOT.pdf
* Misc Resources
  * bootlin course - bootlin.com/blog/building-a-linux-system-for-the-stm32mp1-basic-system/
  * yocto bringup blog post - https://www.cocoacrumbs.com/blog/2021-10-15-building-a-linux-distro-for-the-stm32mp157f-dk2/
  * digikey stm32mp1 playlist - https://www.youtube.com/watch?v=9vsu67uMcko

## STM32MP1 Technical Specs
* Built with a ARM Cortex-M4 embedded inside it.
* 2 peripheral buses: AXI

# Yocto Notes
* OpenSTLinux distribution: openstlinux-6.6-yocto-scarthgap-mpu-v24.11.06
* https://wiki.st.com/stm32mpu/wiki/STM32MPU_Distribution_Package#Installing_the_OpenSTLinux_distribution
*

# DiscoveryKit
## Ports/IO/Buttons
* Front
  * Left
    * usb c - Power In 5V 3A
    * Ethernet
    * DSI (ribbon display)
  * Bottom
    * Audio jack
    * STLink V2.1 micro-usb (terminal)
    * Reset
  * Right
    * 4 usb host ports
    * usb c (data) (to host pc for stm32cubeprogrammer)
    * hdmi out
  * Top
    * 40 pin gpio/rasberry pi standard connector
* Back
  * Left
    * boot mode dipswitch
  * Middle
    * Arduino compatible headers
  * Right
    * SD card

## ICs
* Power - Front, center-left, between DSI and 40 pin header
* DDR3 VRAM - Front, top center, just under of 40 pin header
* STM32MP1 - Front, center, under DDR3 dram
* HDMI Bridge - Lattice, follow hdmi traces, bottom right, left of usb-c
* Debugger/Programmer - STLink, above micro-usb

## Out of box
* Booted up to confirm it worked, plug in STLink micro usb port first and configured a serial terminal, the powered the board through the left usbc port. Boot time was ~1 minute and eventually provided a root tty after on the micro-usb serial port after the boot sequence.
  * `sudo picocom -b 115200 /dev/ttyACM0`
  * saved off original boot image: `sudo dd if=/dev/sdc of=./st32mp157_out_of_box_image.img bs=4M status=progress`
* TODO: look into how STM uses "weston"

## MISC
```
DL_DIR ?= "/mnt/d_wsl/yocto/downloads"
SSTATE_DIR ?= "/mnt/d_wsl/yocto/sstate"
TMPDIR ?= "/mnt/d_wsl/yocto/tmp"
```

MACHINE=stm32mp15-disco bitbake core-image-minimal
MACHINE=stm32mp15-eval bitbake core-image-minimal

ssh 192.168.1.144 -p 9022

## SD Card Schema
```
===========================================================================================================================================================
=      fsbl1  =      fsbl2  =  metadata1  =  metadata2  =      fip-a  =      fip-b  =  u-boot-env =     bootfs  =   vendorfs  =     rootfs  =     userfs  =
=  mmcblk0p1  =  mmcblk0p2  =  mmcblk0p3  =  mmcblk0p4  =  mmcblk0p5  =  mmcblk0p6  =  mmcblk0p7  =  mmcblk0p8  =  mmcblk0p9  =  mmcblk0p10 =  mmcblk0p11 =
=      (1 )   =      (2 )   =      (3 )   =      (4 )   =      (5 )   =      (6 )   =      (7 )   =      (8 )   =      (9 )   =      (10)   =      (11)   =
===========================================================================================================================================================
```
Notes:
* See "{yocto_out_dir}/build/tmp/deploy/images/stm32mp15-eval/FlashLayout_sdcard_stm32mp157f-ev1-opteemin.how_to_update.txt" for exact images copied into each partition.
* 11 partitions on sd card
* mmcblk = "Multimedia card block device" (sd card)
* fsbl = "First stage boot loader", 2 copies "fsbl1" and "fsbl2" for redundancy.
* metadata1, metadata2 = ???, 2 copies for redundancy
* fip-a, fip-b = "Firmware Image Package", U-Boot, 2 copies (although the second one is empty...)
* u-boot-env = u-boot environment variables
* bootfs = uImage linux kernel image and device tree "stm32mp1*.dtb"
* vendorfs = Out of the box image came with ~9 or so libraries (libEGL.so, libVSC.so, libGAL.so, etc.)
* rootfs = main linux root filesystem
* userfs = Out of the box example files, scripts to blink leds, display pngs, etc.
