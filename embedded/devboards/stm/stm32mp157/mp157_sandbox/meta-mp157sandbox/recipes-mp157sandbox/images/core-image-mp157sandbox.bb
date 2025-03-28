SUMMARY = "STM32MP157 sandbox image."
DESCRIPTION = "A small image to explore the STM32MP157's capabilities."
LICENSE = "MIT"

IMAGE_INSTALL = "packagegroup-core-boot ${CORE_IMAGE_EXTRA_INSTALL}"
IMAGE_LINGUAS = " "

inherit core-image

CORE_IMAGE_EXTRA_INSTALL += "hello"
CORE_IMAGE_EXTRA_INSTALL += "openssh"


IMAGE_OVERHEAD_FACTOR ?= "1.0"
IMAGE_ROOTFS_SIZE ?= "512000"
#IMAGE_ROOTFS_EXTRA_SPACE:append = "${@bb.utils.contains("DISTRO_FEATURES", "systemd", " + 4096", "", d)}"

inherit extrausers
# https://docs.yoctoproject.org/singleindex.html#extrausers-bbclass
# printf "%q" $(mkpasswd -m sha256crypt root)
PASSWD = "\$5\$2WoxjAdaC2\$l4aj6Is.EWkD72Vt.byhM5qRtF9HcCM/5YpbxpmvNB5"
EXTRA_USERS_PARAMS = "usermod -p '${PASSWD}' mp157sandbox;"
