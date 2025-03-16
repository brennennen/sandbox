# Yocto Notes


## TLDR
* `source oe-init-build-env`
* `bitbake core-image-sato` or `bitbake core-image-minimal`
* explore build output at `build/tmp/deploy/images/`
  * directory name after images is the first argument to runqemu.
* `runqemu qemux86` or `runqemu qemuarm64`
* login as root, no password.

## Resources
* Bootlin - https://bootlin.com
  * Great free set of technical deep dives and educational exercises.
* Yocto Youtube Channel - https://www.youtube.com/channel/UC2_mG3h-AxxK2oz08j2bz2g
  * Surprisingly active and has many years of training material.
* Docs - https://docs.yoctoproject.org/
  * Variable Reference - https://docs.yoctoproject.org/ref-manual/variables.html

## Terminology
* Yocto - Tools to create linux based images. Part of the linux foundation.
* Poky - Yocto project's reference implementation.
* OpenEmbedded - A metadata set utilized by BitBake.
* BitBake - A generic task executor.
