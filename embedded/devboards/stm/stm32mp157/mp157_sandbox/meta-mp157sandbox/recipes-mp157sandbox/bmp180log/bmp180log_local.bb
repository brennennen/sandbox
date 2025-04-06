# https://docs.yoctoproject.org/brief-yoctoprojectqs/index.html#building-your-image
# bitbake bmp180log
# example package output: build/tmp/deploy/ipk/cortexa7t2hf-neon-vfpv4/bmp180log-dev_local-r0_cortexa7t2hf-neon-vfpv4.ipk
# example bin output: build/tmp/work/cortexa7t2hf-neon-vfpv4-oe-linux-gnueabi/bmp180log/local/package/usr/bin/bmp180log
# scp -P 9022 build/tmp/work/cortexa7t2hf-neon-vfpv4-oe-linux-gnueabi/bmp180log/local/package/usr/bin/bmp180log 192.168.1.144:~/bmp180log
# scp -P 9022 b@192.168.1.144:~/bmp180log ./bmp180log
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://src.tar"

DESCRIPTION = "Reads temperature and pressure from the BMP180 and prints them to console."

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
    install -m 0755 "${WORKDIR}/src/bmp180log" ${D}${bindir}
}
