# CanMV-K230
One of the first accessible RISC-V devboards with a 1.0 implementation of the vector core. Designed primarily for exploring image processing for Asian markets, some documentation has not been translated.

## Resources
* https://github.com/kendryte/k230_canmv/releases
* https://developer.canaan-creative.com/k230/en/rtt/dev/CanMV_K230_Tutorial.html
* https://code.videolan.org/Courmisch/k230-boot/

## Official Setup
* Official setup has some special RTOS on the big core and an embedded linux image on the little core.
* https://developer.canaan-creative.com/k230_canmv/en/main/userguide/how_to_burn_firmware.html
* Download firmware - https://github.com/kendryte/k230_canmv/releases
  * `wget https://github.com/kendryte/k230_canmv/releases/download/v1.0/k230_canmv_micropython_sdcard_v1.6_nncase_v2.8.3.img.gz`
  * `gunzip ./k230_canmv_micropython_sdcard_v1.6_nncase_v2.8.3.img.gz`
* Flash sd card
  * `lsblk -l` - Note device that matches the size: /dev/sdc
  * `sudo dd if=./k230_canmv_micropython_sdcard_v1.6_nncase_v2.8.3.img of=/dev/sdc bs=1M oflag=sync`
* Connect
  * Power up card, left usbc is power and 2 debug serial ports, right is serial data, all prefixed with "ttyACM"
  * `ls -al /dev/ttyACM*`
  * debug serial ports
    * `/dev/ttyACM0` - small core linux debug serial port
      * `sudo screen /dev/ttyACM0 115200`
        * username: root, password: (none)
        * `uname -r` - kernel 5.10.4
    * `/dev/ttyACM1` - large core rt-smart debug serial port
  * `ls -al /dev/ttyACM*` - /dev/ttyACM0
  * `sudo screen /dev/ttyACM0 115200`
    * base image gives a python

## Linux Big Core
* Installing linux on the big core can provide access to some fancier RISC-V instruction set cores/modules that are not popular yet.
* https://code.videolan.org/Courmisch/k230-boot/
