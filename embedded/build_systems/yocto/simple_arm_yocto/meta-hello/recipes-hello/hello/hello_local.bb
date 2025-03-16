# https://docs.yoctoproject.org/brief-yoctoprojectqs/index.html#building-your-image
# bitbake hello

# See https://git.yoctoproject.org/poky/tree/meta/files/common-licenses
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

# https://docs.yoctoproject.org/bitbake/2.0/bitbake-user-manual/bitbake-user-manual-ref-variables.html#term-SRC_URI
SRC_URI = "file://src.tar"

DESCRIPTION = "Prints Hello World"

# https://stackoverflow.com/questions/41058204/how-to-do-fetch-by-hand-in-a-yocto-project
do_fetch() {
    cp ${THISDIR}/files/src.tar ${WORKDIR}
}

do_unpack() {
    cd ${WORKDIR}
    tar xf src.tar
}

do_compile () {
    cd "${WORKDIR}/src"
    oe_runmake
}

do_install () {
    install -d ${D}${bindir}
    install -m 0755 "${WORKDIR}/src/hello" ${D}${bindir}
}
