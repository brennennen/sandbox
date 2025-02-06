# Buildroot Notes


## Terminology
* defconfig - "default config". Used by buildroot and the linux kernel for a set of default configurations that are known to work with the target environment/platform/architecture.


# Menuconfig Deep Dive
As of buildroot trunk February 2025. The notes below are not going to be super useful to anyone besides myself. I find writing small summaries or even just re-wording content I've just read to be useful for recall. Below is more or less a rewording/summarization of the buildroot `make menuconfig` and any external readers should just explore that.

### Target Options
#### Target Architecture
* ARC (little endian)
* ARC (big endian)
* ARM (little endian)
* ARM (big endian)
* AArch64 (little endian)
* AArch64 (big endian)
* i386
* m68k
* Microblaze AXI (little endian)
* Microblaze non-AXI (big endian)
* MIPS (big endian)
* MIPS (little endian)
* MIPS64 (big endian)
* MIPS64 (little endian)
* OpenRISC
* PowerPC
* PowerPC64 (big endian)
* PowerPC64 (little endian)
* RISCV
* s390x
* SuperH
* SPARC
* SPARC64
* x86_64
* Xtensa
#### ARM/AArch64
##### Target Architecture Variant (ARM/AArch64)
* arm
  * armv8 cores
    * cortex-A35
    * cortex-A53
    * cortex-A57
    * cortex-A57/A53 big.LITTLE
    * cortex-A72
    * cortex-A72/A53 big.LITTLE
    * cortex-A73
    * cortex-A73/A35 big.LITTLE
    * cortex-A73/A53 big.LITTLE
    * emag
    * exynos-m1
    * falkor
    * phecda
    * qdf24xx
    * thunderx (aka octeontx)
    * thunderxt81 (aka octeontx81)
    * thunderxt83 (aka octeontx83)
    * thunderxt88
    * thunderxt88p1
    * xgene1
  * armv8.1a cores
    * thunderx2t99
    * thunderx2t99p1
    * vulcan
  * armv8.2a cores
    * cortex-A55
    * cortex-A75
    * cortex-A75/A55 big.LITTLE
    * cortex-A76
    * cortex-A76/A55 big.LITTLE
    * cortex-A78
    * neoverse-N1 (aka ares)
    * tsv110
  * armv8.4a cores
    * saphira
##### Floating point strategy
* VFPv2
* VFPv3
* VFPv3-D16
* VFPv4
* VFPv4-D16
* FP-ARMv8
##### MMU Page Size
* 4KB
* 16KB
* 64KB
##### Target Binary Format
* ELF (no other options)

#### x86-64
##### Target Architecture Variant (x86-64)
* x86-64
* x86-64-v2
* x86-64-v3
* x86-64-v4
* nocona
* core2
* corei7
* nehalem
* westmere
* corei7-avx
* sandybridge
* ivybridge
* core-avx2
* haswell
* broadwell
* skylake
* atom
* bonnell
* silvermont
* goldmont
* goldmont-plus
* tremont
* sierraforest
* grandridge
* skylake-avx512
* cannonlake
* icelake-client
* icelake-server
* cascadelake
* cooperlake
* tigerlake
* sapphirerapids
* alderlake
* rocketlake
* graniterapids
* graniterapids-d
* opteron
* opteron w/ SSE3
* barcelona
* bobcat
* jaguar
* bulldozer
* piledriver
* steamroller
* excavator
* zen
* zen 2
* zen 3
* zen 4
#### RISCV
##### Target Architecture Variant (RISCV)
* General purpose (G)
* Custom architecture
###### Instruction Set Extensions
(some instruction sets can't be set or removed in buildroots menuconfig, I'm going to assume these instruction extensions are mandatory for a general operating system like linux.)
* Base Integer (I) - (mandatory)
* Integer Multiplication and Division (M) - (mandatory)
* Atomic Instructions (A) - (mandatory)
* Single-precision Floating-point (F) - (mandatory)
* Double-precision Floating-point (D) - (mandatory)
* Compressed Instructions (C) (NEW)
* Vector Instructions (V) (NEW)
##### Target Architecture Size (64-bit)
* 32-bit
* 64-bit
##### MMU Support (NEW)
* YES/NO
##### Target ABI (lp64d)
* lp64
* lp64f
* lp64d
##### Target Binary Format (ELF)
* ELF (no other options)

### Toolchain
#### Toolchain type
* Buildroot toolchain
* External toolchain
* custom toolchain vendor name - default: "buildroot"
#### C Library
* uClibc-ng
* glibc
* musl
#### Hernel Header Options
* Kernel Headers
  * Same as kernel being built
  * Linux 5.4.x kernel headers
  * Linux 5.10.x kernel headers
  * Linux 5.15.x kernel headers
  * Linux 6.1.x kernel headers
  * Linux 6.6.x kernel headers
  * Linux 6.12.x kernel headers
  * Manually specified Linux version
  * Custom tarball
  * Custom Git repository
* Custom kernel headers series
  * 6.12.x or later
  * 6.11.x
  * ... (many versions between 6.11.x and 2.6.x)
  * 2.6.x
#### Glibc Options
* Enable compatibility shims to run on older kernels
* Install glibc utilities
#### Binutils Options
* Binutils Version
  * binutils 2.41
  * binutils 2.42
  * binutils 4.43.1
* gprofng support
* Additional binutils options
#### GCC Options
* GCC compiler version
  * gcc 12.x
  * gcc 13.x
  * gcc 14.x
* Additional gcc options
* Enable c++ support
* Enable fortran support
* Enable compiler OpenMP support
* Enable graphite support
#### Host GDB Options
* Build cross gdb for the host
#### Toolchain Generic Options
* Copy gconv libraries
* Extra toolchain libraries to be copied to target
* Target Optimizations
* Target Linker options
#### Bare metal toolchain
* Build a bare metal toolchain

### Build Options
* Commands
  * curl - "curl -q --ftp-pasv --retry 3"
  * wget - "wget -nd -t 3"
  * svn - "svn --non-interactive"
  * bzr
  * git
  * cvs
  * cp
  * scp
  * sftp
  * hg
  * zcat - "gzip -d -c"
  * bzcat
  * xzcat
  * lzcat - "lzip -d -c"
  * zstdcat
  * tar
* Location to save - (defaults to path to the defconfig you use)
* Download dir - "$(TOPDIR)/dl"
* Host dir - "$(BASE_DIR)/host"
* Mirrors and Download locations
  * Primary download site
  * Backup download site - "https://sources.buildroot.net"
  * Kernel.org mirror - "https://cdn.kernel.org/pub"
  * GNU Software mirror - "https://gtpmirror.gnu.org"
  * LuaRocks mirror - "http://rocks.moonscript.org"
  * CPAN mirror (Perl packages) - "https://cpan.metacpan.org"
* Number of jobs to run simultaneously (0 for auto) - default: "0"
* Enable compiler cache
* build packages with debugging symbols
* build packages with runtime debugging info
* strip target binaries - default: "YES"
  * executables that should not be stripped
  * directories that should be skipped when stripping
* gcc optimization level
  * optimization level 0
  * optimization level 1
  * optimization level 2
  * optimization level 3
  * optimize for debugging
  * optimize for size
  * optimize for fast (may break packages!)
* build packages with link-time optimisation
* libraries
  * static only
  * shared only
  * both static and shared
* location of a package override file - default: "$(CONFIG_DIR)/local.mk"
* global patch and hash directories - default: "board/qemu/patches"
* Advanced
  * Force the building of host dependencies
  * Force all downloads to have a valid hash
  * Make the build reproducible (experimental)
  * Use per-package directories (experimental)
* Build code with PIC/PIE - default: YES
* Stack Smashing Protection
  * None
  * -fstack-protector
  * -fstack-protector-strong - default
  * -fstack-protector-all
* RELRO Protection
  * None
  * Partial
  * Full - default
* Buffer-overflow Detection
  * None
  * Conservative - default
  * Aggressive
  * Extended

### System Configuration
* Rootf FS Skeleton
  * default target skeleton
  * custom target skeleton
* System hostname - default "buildroot"
* System banner - default "Welcome to Buildroot"
* Passwords encoding
  * sha-256 - default
  * sha-512
* Init system
  * BusyBox - default
  * systemV
  * OpenRC
  * systemd
  * tini
  * tiny init
  * Custom (None)
* /dev management
  * Static using device table
  * Dynamic using devtmpfs only - default
  * Dynamic using devtmpfs + mdev
  * Dynamic using devtmpfs + eudev
* Path to the permission tables - default: "system/device_table.txt"
* support extended attributes in device tables
* Use symlinks to /usr for /bin, /sbin and /lib
* Enable root login with password - default
* Root password - default ""
* /bin/sh
  * busybox default shell
  * bash, dash, mksh, zsh
  * none
* Run a getty (login prompt) after boot
  * TTY port - default: "console"
  * Baudrate
    * kernel default
    * 9600
    * 19200
    * 38400
    * 57600
    * 115200
  * TERM environment variable - default: "vt100"
  * other options to pass to getty - default: ""
* Remount root filesystem read-write during boot
* Network interface to configure through DHCP - default: "eth0"
* Set the system's default PATH - default: "/bin:/sbin:/usr/sbin"
* Purge unwanted locales
* Locales to keep - default: "C en_US"
* Generate locale data
* Enable Native Language Support
* Install timezone info
* Path to the users tables
* Root filesystem overlay directories
* Custom scripts to run before commencing the build
* Custom scripts to run before creating filesystem images
* Custom scripts to run inside the fakeroot environment
* Custom scripts to run after creating filesystem images - default: "board/qemu/post-image.sh"
* Extra arguments passed to custom scripts - default: "$(BR2_DEFCONFIG)"
* Extra arguments apss to POST_IMAGE_SCRIPT

### Kernel
* Linux Kernel (I'm surprised this can be disabled)
  * Kernel version
    * Latest version (6.12)
    * Latest CIP SLTS version (5.10.162.cip24)
    * Latest CIP RT SLTS version (5.10.162.cip24-rt10)
    * Custom version
    * Custom tarball
    * Custom Git repository
    * Custom Mercurial repository
    * Custom Subversion repository
  * Kernel verion (string) - default: "6.12.9"
* Custom kernel patches
* Kernel configuration
  * Using an in-tree defconfig file
  * Use the architecture default configuration
  * Using a custom (def)config file
* Configuration file path - default: (based on defconfig selected)
* Additional configuration fragment files
* Custom boot logo file path
* Kernel binary format
  * Image
  * Image.gz
  * vmlinux
  * custom target
* Kernel comrpession format
  * gzip
  * lz4
  * lzma
  * lzo
  * xz
  * zstd
* Build a Device Tree Blob (DTB)
* Install kernel image to /boot in target
* Needs host OpenSSL - default
* Needs host libelf
* Needs host pahole
* Needs host python3
* Linux Kernel Extensions - default: (based on defconfig selected)
* Linux Kernel Tools
  * cpupower
  * gpio
  * iio
  * mm
  * pci
  * perf
  * rtla
  * selftests
  * USB test programs
  * tmon

### Target Packages
* BusyBox (mandatory)
  * Additional busyBox configuration fragment files
  * Show packages that are also provided by busybox
  * Individual binaries
  * Install the watchdog daemon startup script
* Audio and video applications
  * alsa-utils
  * atest
  * aumix
  * bluez-alsa
  * dvblast
  * dvdauthor
  * dvdrw-tools
  * espeak
  * faad2
  * ffmpeg
  * flac
  * flite
  * fluid-soundfont
  * fluidsynth
  * gmrender-resurrect
  * gstreamer 1.x
  * jack1
  * jack2
  * kodi
  * lame
  * madplay
  * mimic
  * minimodem
  * miraclecast
  * minimodem
  * miraclecast
  * mjpegtools
  * modplugtools
  * motion
  * mpd
  * mpd-mpc
  * mpg123
  * mpv
  * multicat
  * musepack
  * ncmpc
  * opus-tools
  * pipewire
  * pulseaudio
  * sox
  * speechd
  * squeezelite
  * tinycompress
  * tstools
  * twolame
  * udpxy
  * upmpdcli
  * v4l2grab
  * v4l2loopback
  * vlc
  * vorbis-tools
  * wavpack
  * yavta
  * ympd
  * zynaddsubfx
* Compressors and decompressors
  * brotli
  * bzip2
  * lrzip
  * lzip
  * lzop
  * p7zip
  * pigz
  * pixz
  * unrar
  * xz-utils
  * zip
  * zstd
* Debugging, profiling, and benchmark
  * babeltrace2
  * bcc
  * blktrace
  * bonnie++
  * bpftool
  * bpftrace
  * cache-calibrator
  * clinfo
  * clpeak
  * coremark
  * coremark-pro
  * dacapo
  * delve
  * dhrystone
  * dieharder
  * dmalloc
  * dropwatch
  * dstat
  * dt
  * duma
  * fio
  * fwts
  * gdb/gdbserver
  * google-breakpad
  * hyperfine
  * iozone
  * kexec
  * kmemd
  * latencytop
  * libbpf
  * libtraceevent
  * libtracefs
  * lmbench
  * ltp-testsuite
  * ltrace
  * lttng-babeltrace
  * lttng-modules
  * lttng-tools
  * mbpoll
  * mbw
  * memstat
  * netperf
  * netsniff-ng
  * nmon
  * oprofile
  * pax-utils
  * perftest
  * piglit
  * ply
  * poke
  * ptm2human
  * pv
  * ramspeed/smp
  * ramspeed
  * rt-tests
  * rwmem
  * sentry-native
  * signal-estimator
  * spidev_test
  * strace
  * stress
  * stress-ng
  * sysdig
  * sysprof
  * tbtools
  * tcf-agent
  * trace-cmd
  * trinity
  * uclibc-ng-test
  * uftrace
  * valgrind
  * vmtouch
  * whetstone
* Development tools
  * avocado
  * binutils
  * bitwise
  * check
  * ctest
  * cppunit
  * cukinia
  * cuinit
  * cvs
  * cxxtest
  * fd
  * flex
  * gettext
  * git
  * git-crypt
  * gperf
  * jo
  * jq
  * libtool
  * make
  * mawk
  * pkgconf
  * ripgrep
  * subversion
  * tig
  * tree
  * unifdef
* Filesystem and flash utilities
  * abootimg
  * aufs-util
  * autofs
  * btrfs-progs
  * cifs-utils
  * cpio
  * cramfs
  * curlftpfs (FUSE)
  * davfs2
  * dosfstools
  * dust
  * e2fsprogs
  * e2tools
  * ecryptfs-utils
  * erofs-utils
  * exFAT (FUSE)
  * exfat-utils
  * exfatprogs
  * f2fs-tools
  * firmware-utils
  * flashbench
  * fscryptctl
  * fuse-overlayfs
  * fwup
  * genext2fs
  * genpart
  * genromfs
  * gocryptfs
  * imx-usb-loader
  * mmc-utils
  * mtd, jffs2, and ubi/ubifs tools
  * mtools
  * nfs-utils
  * nilfs-utils
  * ntfs-3g
  * sp-oops-extract
  * squashfs
  * sshfs (FUSE)
  * udftools
  * unionfs (FUSE)
  * xfsprogs
  * zerofree
  * zfs
* Fonts, cursors, icons, sounds and themes
  * Cursors
    * comix-cursors
    * obsidian-cursors
  * Fonts
    * Bitstream Vera
    * cantarell
    * DejaVu fonts
    * font-awesome
    * ghostscript-fonts
    * inconsolata
    * Liberation (Free fonts)
    * wqy-zenhei
  * Icons
    * hicolor icon theme
  * Sounds
    * sound-theme-borealis
    * sound-theme-freedesktop
  * Themes
* Games
  * ascii-invaders
  * chocolate-doom
  * flare-engine
  * gnuchess
  * LBreakout2
  * LTris
  * minetest
  * OpenTyrian
  * prboom
  * sl
  * solarus
  * stella
  * xorcurses
* Graphic libraries and applications (graphic/text)
  * Graphic Applications
    * cage
    * cog
    * dmenu-wayland
    * flutter
    * flutter-pi
    * foot
    * fswebcam
    * ghostscript
    * glmark2
    * glslsandbox-player
    * gnuplot
    * igt-gpu-tools
    * ivi-homescreen
    * jhead
    * kmscube
    * libva-utils
    * netsurf
    * pngquant
    * rddtool
    * spirv-tools
    * stellarium
    * sway
    * swaybg
    * tesseract-ocr
    * tinifier
  * Graphic libraries
    * cegui
    * efl
    * fb-test-app
    * fbdump
    * fbgrab
    * fbterm
    * fbv
    * freerdp
    * graphicsmagick
    * imagemagick
    * libglvnd
    * mesa3d
    * ocrad
    * ogre
    * psplash
    * SDL
    * sdl2
    * spirv-headers
    * vulkan-headers
    * vulkan-tools
  * Other GUIS
    * QT5
    * qt6
    * tekui
    * weston
    * X.org X Window System
    * apitrace
    * mupdf
    * vte
    * xkeyboard-config
* Hardware handling
  * Firmware
    * armbian-firmware
    * b43-firmware
    * brcmfmac-sdio-firmware-rpi
    * linux-firmware
    * murata-cyw-fw
    * odroidc2-firmware
    * panel-mipi-dbi firmware
    * qcom-db410c-firmware
    * qoriq-fm-ucode
    * rcw-smarc-sal28
    * rpi-firmware
    * ux500-firmware
    * wilc1000-firmware
    * wilc3000-firmware
    * wilink-bt-firmware
    * zd1211-firmware
  * 18xx-ti-utils
  * acpica
  * acpid
  * acpitool
  * aer-inject
  * altera-stapl
  * apcupsd
  * avrdude
  * bcache-tools
  * bfscripts
  * brickd
  * brltty
  * cc-tool
  * cdrkit
  * cpuburn-arm
  * crucible
  * cryptsetup
  * swiid
  * dahdi-linux
  * dahdi-tools
  * dbus
  * dbusbroker
  * dbus-cxx
  * dfu-programmer
  * dfu-tuil
  * dmidecode
  * dmraid
  * dt-utils
  * dtbocfg
  * dtv-scan-tables
  * dump1090
  * dvbsnoop
  * edid-decode
  * esp-hosted
  * espflash
  * eudev
  * evemu
  * evtest
  * flashrom
  * fmtools
  * freeipmi
  * Freescal i.MX libraries
  * fwupd
  * fwupd-efi
  * fxload
  * gcnano-binaries
  * gpm
  * gpsd
  * gptfdisk
  * gvfs
  * hddtemp
  * hwdata
  * hwloc
  * input-event-daemon
  * iotools
  * ipmitool
  * irda-utils
  * kbd
  * lcdproc
  * ledmon
  * libiec61850
  * libmanette
  * libubootenv
  * libuio
  * linux-backports
  * linux-serial-test
  * linuxconsoletools
  * lirc-tools
  * lm-sensors
  * lshw
  * lsscsi
  * lsuio
  * luksmeta
  * lvm2 & device mapper
  * mali-driver
  * mbpfan
  * mdadm
  * mdevd
  * mdio-tools
  * memtester
  * memtool
  * mhz
  * minicom
  * mxt-app
  * nanocom
  * neard
  * nvidia-modprobe
  * nvidia-persistenced
  * nvme
  * ofono
  * ola
  * open2300
  * openfpgaloader
  * openipmi
  * openocd
  * parted
  * pciutils
  * pdbg
  * picocom
  * picotool
  * pigpio
  * powertop
  * pps-tools
  * qoriq-cadence-dp-firmware
  * raspi-gpio
  * rdma-core
  * read-edid
  * rng-tools
  * rockchip-mali
  * rockchip-rkbin
  * rpi-userland
  * rs485conf
  * rtc-tools
  * rtl8188eu
  * rtl8189es
  * rtl8192eu
  * rtl8723bu
  * rtl8723ds
  * rtl8723ds-bt
  * rtl8812au-aircrack-ng
  * rtl8821au
  * rtl8821cu
  * rtl8822cs
  * sane-backends
  * sdparm
  * sdutil
  * setserial
  * sg3-utils
  * sigrok-cli
  * sispmctl
  * smartmontools
  * smstools3
  * spi-tools
  * sredird
  * statserial
  * stm32flash
  * sunxi-mali-utgard
  * sysstat
  * ti-uim
  * ti-utils
  * tio
  * triggerhappy
  * uboot-bootcount
  * u-boot tools
  * ubus
  * udisks
  * uefisettings
  * uhubctl
  * umtprd
  * upower
  * usb_modeswitch
  * usb_modeswitch_data
  * usbguard
  * usbip
  * usbmount
  * usbutils
  * w_scan
  * wilc kernel module
  * wipe
  * xorriso
  * xr819-xradio
* Interpreter languages and scripting
  * fth
  * chicken
  * enscript
  * erlang
  * execline
  * ficl
  * guile
  * haserl
  * hanet
  * jimtcl
  * lua
  * luajit
  * micropython
  * moarvm
  * mono
  * nodejs
  * octave
  * openjdk
  * perl
  * php
  * python3
  * quickjs
  * ruby
  * swipl
  * tcl
* Libraries
  * Audio/Sound
  * Compression and decompression
  * Crypto
  * Database
  * Filesystem
  * Graphics
  * Hardware handling
  * Javascript
  * JSON/XML
  * Logging Multimedia
  * Networking
  * Other
  * Security
  * Text and terminal handling
* Mail
  * dovecot
  * exim
  * fetchmail
  * heirloom-mailx
  * libesmtp
  * msmtp
  * mutt
* Miscellaneous
  * aespipe
  * bc
  * bitcoin
  * clamav
  * collectd
  * collectl
  * domoticz
  * empty
  * fft-eval
  * gitlab-runner
  * gnuradio
  * Google font directory
  * gsettings-desktop-schemas
  * haveged
  * linux-syscall-support
  * mobile-broadband-provider-info
  * netdata
  * proj
  * QEMU
  * qpdf
  * rtl_433
  * shared-mime-info
  * sunwait
  * taskd
  * xmrig
  * util-macros
* Networking applications
  * aircrack-ng
  * alfred
  * aoetools
  * apache
  * argus
  * arp-scan
  * arptables-legacy
  * asterisk
  * atftp
  * avahi
  * axel
  * babeld
  * bandwidthd
  * batctl
  * bcusdk
  * bind
  * bird
  * bluez-utils
  * bmon
  * bmx7
  * boinc
  * brcm-patchram-plus
  * bridge-utils
  * bwm-ng
  * c-icap
  * can-utils
  * cannelloni
  * casync
  * cfm
  * chrony
  * civetweb
  * cloudflared
  * connman
  * connman-gtk
  * conntrack-tools
  * corkscrew
  * crda
  * ctorrent
  * cups
  * cups-filter
  * dante
  * darkhttpd
  * dehydrated
  * dhcpcd
  * dhcpdump
  * dnsmasq
  * drbd-utils
  * dropbear
  * easyframes
  * edtables
  * ejabberd
  * ethtool
  * faifa
  * fastd
  * fcgiwrap
  * firewalld
  * flannel
  * fmc
  * fping
  * freeradius-server
  * freeswitch
  * frr
  * gerbera
  * gesftpserver
  * gloox
  * glorytun
  * gupnp-tools
  * hans
  * haproxy
  * hostapd
  * htpdate
  * httping
  * i2pd
  * IANA assignments
  * ibrdtn-tools
  * ibrdtnd
  * ifmetric
  * iftop
  * ifupdown scripts - default
  * igd2-for-linux
  * igh-ethercat
  * igmpproxy
  * inadyn
  * iodine
  * iperf
  * iperf3
  * iproute2
  * ipset
  * iptables
  * iptraf-ng
  * iputils
  * irssi
  * iw
  * iwd
  * janus-gateway
  * keepalived
  * kismet
  * knock
  * ksmbd-tools
  * leafnode2
  * lft
  * lftp
  * lighttpd
  * linknx
  * links
  * linphone
  * linux-zigbee
  * linuxptp
  * lldpd
  * lrzsz
  * lynx
  * macchanger
  * memcached
  * mii-daig
  * mini-snmpd
  * minidlna
  * minissdpd
  * mjpg-streamer
  * modem-manager
  * mogrel2
  * mosh
  * mosquitto
  * mrouted
  * mrp
  * mstpd
  * mtr
  * nbd
  * ncftp
  * ndisc6 tools
  * netatalk
  * netcalc
  * nethogs
  * netplug
  * netsnmp
  * netstat-nat
  * NetworkManager
  * nfacct
  * nftables
  * nginx
  * ngircd
  * ngrep
  * nload
  * nmap-nmap
  * noip
  * ntp
  * ntpsec
  * nuttcp
  * odhcp6c
  * odhcploc
  * olsr
  * open-iscsi
  * open-lldp
  * open-plc-utils
  * openconnect
  * openntpd
  * openobex
  * openresolv
  * openssh
  * openswan
  * openvpn
  * p910nd
  * parprouted
  * phidgetwebservice
  * phytool
  * pimd
  * pixiwps
  * pound
  * pppd
  * pptp-linux
  * privoxy
  * proftpd
  * prosody
  * proxychains-ng
  * ptpd
  * ptpd2
  * pure-ftpd
  * putty
  * radvd
  * reaver
  * redir
  * rp-pppoe
  * prcbind
  * rsh-redone
  * rsync
  * rtorrent
  * rtptools
  * s6-dns
  * s6-networking
  * samba4
  * sconeserver
  * ser2net
  * shadowsocks-libev
  * shairport-sync
  * shellinabox
  * smcroute
  * sngrepsnort
  * snort3
  * socat
  * socketcand
  * softether
  * spawn-fcgi
  * spice protocol
  * squid
  * ssdp-responder
  * sshguard
  * sshpass
  * sslh
  * strongswan
  * stunnel
  * suricata
  * tailscale
  * tcpdump
  * tcping
  * tcpreplay
  * thttpd
  * tinc
  * tinyproxy
  * tinyssh
  * tipidee
  * tor
  * traceroute
  * transmission
  * tunctl
  * tvheadend
  * uacme
  * udpcast
  * uftp
  * uhttpd
  * ulogd
  * unbound
  * uqmi
  * uredir
  * ushare
  * ussp-push
  * ustreamer
  * vde2
  * vdr
  * vnstat
  * vpnc
  * vsftpd
  * vtun
  * wavemon
  * wirefuard tools
  * wireless-regdb
  * wireless tools
  * wireshark
  * wpa-supplicant
  * wpan-tools
  * xinetd
  * xl2tp
  * xtables-addons
  * zabbix
  * zeek
  * znc
* Package managers
  * See http://buildroot.org/manual.html#faq-no-binary-packages
  * opkg
  * opkg-utils
* Real-Time
  * Xenomai Userspace
* Security
  * apparmor
  * checkpolicy
  * ima-evm-utils
  * optee-client
  * paxtest
  * policycoreutils
  * refpolicy
  * restorecond
  * selinux-python
  * semodule-utils
  * setools
  * urandom-initscripts
* Shell and utilities
  * Shells
    * mksh
    * nushell
    * zsh
  * Utilities
    * apg
    * at
    * catatonit
    * ccrypt
    * dialog
    * dtach
    * easy-rsa
    * eza
    * file
    * gnupg
    * gnupg2
    * inotify-tools
    * lockfile programs
    * logrotate
    * logsurfer
    * minisign
    * pdmenu
    * pinentry
    * qprint
    * ranger
    * rlwrap
    * rtty
    * screen
    * sexpect
    * sudo
    * tini
    * tmux
    * ttyd
    * wtfutil
    * xmlstarlet
    * xxhash
    * ytree
    * zoxide
* System tools
  * acl
  * android-tools
  * atop
  * attr
  * audit
  * balena-engine
  * bubblewrap
  * cgroupfs-mount
  * circus
  * conmon
  * containerd
  * cpulimit
  * cpuload
  * criu
  * crun
  * daemon
  * dc3dd
  * ddrescue
  * docker-cli
  * docker-cli-buildx
  * docker-compose
  * docker-engine
  * earlyoom
  * efibootmgr
  * efivar
  * embiggen-disk
  * emlog
  * fluent-bit
  * ftop
  * getent
  * gkrellm
  * htop
  * ibm-sw-tpm2
  * iotop
  * iprutils
  * irqbalance
  * jailhouse
  * keyutils
  * kmod
  * kmon
  * kvmtool
  * libostree
  * libvirt
  * lxc
  * makedumpfile
  * mender
  * mender-connect
  * mfoc
  * moby-buildkit
  * mokutil
  * monit
  * multipath-tools
  * ncdu
  * nerdctl
  * netifrc
  * numactl
  * nut
  * openvmtools
  * pamtester
  * petitboot
  * polkit
  * procrank_linux
  * procs
  * pwgen
  * qbee-agent
  * quota
  * quotatool
  * rauc
  * rauc-hawkbit-updater
  * runc
  * s6
  * s6-linux-init
  * s6-linux-utils
  * s6-portable-utils
  * s6-rc
  * scrub
  * scrypt
  * sdbus-c++
  * sdbusplus
  * seatd
  * shadow
  * smack
  * supervisor
  * swupdate
  * tealdeer
  * tpm-tools
  * tpm2-abrmd
  * tpm2-tools
  * tpm2-totp
  * unscd
  * util-linux
  * watchdog
  * watchdogd
  * xdg-dbus-proxy
  * xen
  * xvisor
* Text editors and viewers
  * bat
  * ed
  * joe
  * mc
  * mg
  * most
  * nano
  * uemacs
### Filesystem Images
* axfx root filesystem
* btrfs root filesystem
* cloop root filesystem for the target device
* cpio the root filesystem (for use as an initial RAM filesystem)
* cramfs root filesystem
* erofs root filesystem
* ext2/3/4 root filesystem
  * ext2/3/4 variant (ext4)
* filesystem label - default: "rootfs"
* exact size - default: "60M"
* inode size - default: "256"
* reserved blocks percentage - default: "5"
* additional mke2fs options - default: "-0 ^64bit"
  * Compression method - default: "no compression"
* f2fs root filesystem
* initial RAM filesystem linked into linux kernel
* jffs2 root filesystem
* oci image
* romfs root filesystem
* squashfs root filesystem
* tar the root filesystem
* ubi image containing an ubifs root filesystem
* ubifs root filesystem
* yaffs2 root filesystem
### Bootloaders
* ARM Trusted Firmware (ATF)
* Barebox
* binaries-marvell
* boot-wrapper-aarch64
* EDK2
* grub2
* mv-ddr-marvell
* optee_os
* shim
* ti-k3-boot-firmware
* ti-k3-r5-loader
* U-Boot
* vexpress-firmware
* xilinx-embeddedsw
* xilinx-prebuilt
### Host Utilities
* abootimg
* aespipe
* agent-proxy
* amlogic-boot-fip
* android-tools
* asn1c
* babeltrace2
* bmap-tools
* bootgen
* btrfs-progs
* checkpolicy
* checksec
* cmake
* composer
* cramfs
* crudini
* cryptsetup
* debus-python
* delve
* depot-tools
* dfu-util
* dos2unix
* dosfstools
* doxygen
* dtc
* e2fsprogs (mandatory)
* e2tools
* environment-setup
* erofs-utils
* exfatprogs
* f2fs-tools
* faketime
* fatcat
* firmware-utils
* flutter-sdk-bin
* fwup
* genext2fs
* genimage
* genpart
* gnupg
* gnupg2
* go
* gptfdisk
* imagemagick
* imx-mkimage
* imx-usb-loader
* jq
* jsmin
* kmod (mandatory)
  * support gzip-compressed modules
  * support zstd-compressed modules
  * support xz-compressed modules
* libp11
* lld
* lpc3250loader
* lttng-babeltrace
* lzma-alone
* mender-artifact
* meson-tools
* microchip-hss-payload-generator
* minisign
* mkpasswd
* moby-buildkit
* mosquitto
* mtd, jffs2, and ubi/ubifs tools
* mtools
* nodejs
* host-odb
* opencd
* opkg-utils
* pahole
* parted
* pigz
* pkgconf
* pwgen
* python-cython
* python-greenlet
* python-iniparse
* python-lxml
* python-pyyaml
* python-six
* python-uswid
* python-xlrd
* python3
* qemu (enabled by default)
  * enable system emulation (enabled by default)
  * enable linux user-land emulation
  * vde2 support
  * virtual filesystem support
  * usb passthrough support
* qoriq-rcw
* raspberrypi-usbboot
* rauc
* riscv-isa-sim
* runc
* rustc
* sam-ba
* sdbus-c++
* host-sdbusplus
* sentry-cli
* skopeo
* sloci-image
* squashfs
* swig
* swtpm
* swugenerator
* tipidee
* u-boot tools
* util-linux (mandatory)
* utp_com
* uuu
* vboot utils
* xorriso
* zip
* zstd
### Legacy Config Options
(too many to list/explore. some cairo libraries I've used in here)





# Buildroot User Manual Notes
The notes below are not going to be super useful to anyone besides myself. I find writing small summaries or even just re-wording content I've just read to be useful for recall. Below is more or less a rewording/summarization of the buildroot user manual and any external readers should just read that.

## Toolchains
* Internal toolchain - built for you, supports cross-compiling, rebuilt on make clean
* External toolchain - Not rebuilt on make clean, requires setup work, yocto and openembedded tool chains are not supported. Use `make sdk` to get started.

## Device Tree Management
* Static device table - Traditional way of managing device tree. Device files are persistent across reboots and setup on initial install/first boot.
* devtmpfs - Fill device table dynamically as components are plugged/unplugged. Not persistent across reboots.
* devtmpfs + mdev - Calls userspace `mdev` busybox utility whenever devices are added/removed. `mdev` can be configured to handle things like firmware uploads, user permissions, configuration, etc.
* devtmpfs + eudev - systemd udev userspace daemon to manage devices.

## Init System
init program is the first userspace program started by the kernel, PID 1.
* busybox - default, driven by `/etc/inittab`, see `package/busybox/inittab`. Mounts filesystems, start `etc/init.d/rcS`, and start a `getty` program. `BR2_INIT_BUSYBOX`.
* systemV - sysvinit, see `package/sysvinit/inittab`. mostly replaced by systemd.
* systemd - modern approach, does far more than old style initrc/systemv approaches. D-Bus activation to start services, on-demand daemons, linux control groups, snapshotting, restoring from snapshots. Is complex.

## Core Component Configuration
Buildroot is a collection of the linux kernel, busybox, libc, and potentially `u-boot` (and `barebox` if not using the linux kernel). Configuring these before diving too deep into the buildroot configs is advised.
* BusyBox - BR2_PACKAGE_BUSYBOX_CONFIG - make busybox-menuconfig
* uClibc - BR2_UCLIBC_CONFIG - make uclibc-menuconfig
* Linux Kernel - BR2_LINUX_KERNEL_USE_CUSTOM_CONFIG, BR2_LINUX_KERNEL_USE_DEFCONFIG, BR2_LINUX_KERNEL_USE_CUSTOM_CONFIG - make linux-menuconfig
* uboot - BR2_TARGET_UBOOT_USE_CUSTOM_CONFIG, BR2_TARGET_UBOOT_USE_DEFCONFIG - make uboot-menuconfig

## Make Tips
* `make V=1 <target>` - display all commands executed by make
* `make list-defconfigs` - display list of boards with a defconfig
* `make help` - display all available targets
  * not all targets are always available and some settings in the .config file may hide some targets
* `make clean` - delete build artifacts
* `make distclean` - delete all artifacts and configurations, full reset.
* `make -s printvars VARS='VAR1 VAR2` - debug print local make variables
  * ex: `make -s printvars VARS=BUSYBOX_%DEPENDENCIES` - '%' used for wildcard

## Rebuilding

## Out-of-tree

## Environment Variables

## Filesystem Images

## Details

## Graphing
* `make graph-depends` - graph of dependency relations - output/graphs/graph-depends.pdf
* `make graph-build` - histogram of build durations - output/graphs/build.hist-build.pdf
* `make graph-size` - pie chart of package file sizes - output/graphs/graph-size.pdf

## Advanced
* Top level parallel builds are parallel, don't use `make -j4 ...`, buildroot will handle threading.
* Use the toolchain generated by buildroot at `output/host/bin` to compile user-land programs.
* `make sdk` can be used to export the toolchain, which can be independently distributed for userland focused team members who won't need to make kernel/bsp level changes. Users who install this package must run the script `relocate-sdk.sh` in the output tarball. `make prepare-sdk` is similar, but doesn't tarball.

## GDB


## ccache

## Download caching

## Package Specific Make Targets

(I have a good enough understanding at this point, but may continue this in the future.)
