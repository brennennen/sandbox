SUMMARY = "STM32MP157 sandbox image."
DESCRIPTION = "A small image to explore the STM32MP157's capabilities."
LICENSE = "MIT"

IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_LINGUAS = " "

inherit core-image
CORE_IMAGE_EXTRA_INSTALL += "openssh"
CORE_IMAGE_EXTRA_INSTALL += "i2c-tools"
CORE_IMAGE_EXTRA_INSTALL += "hello"
CORE_IMAGE_EXTRA_INSTALL += "bmp180log"

IMAGE_OVERHEAD_FACTOR ?= "1.0"
IMAGE_ROOTFS_SIZE ?= "512000"
#IMAGE_ROOTFS_EXTRA_SPACE:append = "${@bb.utils.contains("DISTRO_FEATURES", "systemd", " + 4096", "", d)}"

inherit extrausers
# https://docs.yoctoproject.org/singleindex.html#extrausers-bbclass
# printf "%q" $(mkpasswd -m sha256crypt mp157sandbox)
PASSWD = "\$5\$mJthEjlyLHV5ee6p\$YXyHB6ZDIO7wxd7nzaE8Cgme4Jg4y0zARFaW7qoIMhA"
EXTRA_USERS_PARAMS = "usermod -p '${PASSWD}' root;"
