inherit core-image
CORE_IMAGE_EXTRA_INSTALL += "hello"
CORE_IMAGE_EXTRA_INSTALL += "openssh"
inherit extrausers
# https://docs.yoctoproject.org/singleindex.html#extrausers-bbclass
# printf "%q" $(mkpasswd -m sha256crypt root)
PASSWD = "\$5\$2WoxjAdaC2\$l4aj6Is.EWkD72Vt.byhM5qRtF9HcCM/5YpbxpmvNB5"
EXTRA_USERS_PARAMS = "usermod -p '${PASSWD}' root;"
