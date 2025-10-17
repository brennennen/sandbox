# Emulator
Toy single program emulator to review and deeper learn popular processors, ISAs, and ABIs.

## Build
Currently uses some c23 features when available. Expects at least gcc-14 or later to use
these c23 features on linux. I used premake with the intention to support any OS, but I haven't tested on any other OSes.

Linux
```sh
# One time - use a modern gcc to access c23 features (skip if your gcc already supports c23)
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100

# Clean intermediate files
rm -rf ./.build ./bin

# Build meta-build files
premake5 gmake

# Build
make -C ./.build/
make config=debug -C ./.build/
make config=release -C ./.build/

# Run/Test
./bin/Debug/sandbox
./bin/Debug/test_emulate_intel
./bin/Debug/test_emulate_arm
./bin/Debug/test_emulate_riscv
./bin/Debug/test
```

Windows
```sh
# use mingw/w64devkit
# requires criterion, no prebuilt packages exist for this, requires building from source.
# build criterion notes:
# git clone criterion
# criterion build scripts use "patch --version" which w64devkit chokes up on, if you have git, you can use that version of patch, otherwise msys2 mingw shouldn't have this problem.
# $env:Path = "C:\Program Files\Git\usr\bin;" + $env:Path
# If you're using a modern gcc, criterion had build errors without this line.
# $env:CFLAGS = "-Wno-incompatible-pointer-types"
# meson setup build
# meson compile -C build
# meson install -C build
premake5 gmake
premake5 --os=windows gmake
make -C ./.build/
```

## Build env Setup
* Tooling:
  * premake - https://premake.github.io
* Dependencies:
  * criterion - https://github.com/Snaipe/Criterion
* Misc:
  * nasm - https://www.nasm.us/

Linux (Ubuntu):
```sh
# Install premake as a build generator
mkdir -p ~/bin && cd ~/bin
wget https://github.com/premake/premake-core/releases/download/v5.0.0-beta4/premake-5.0.0-beta4-linux.tar.gz
tar -xvzf premake-5.0.0-beta4-linux.tar.gz
chmod +x ./premake5

# Install criterion dev package for unit testing
apt-get install libcriterion-dev

# Install tools to assemble machine code
# intel: nasm, web based install: https://www.nasm.us/, `nasm` and `xxd -i`
# arm: gnu `as` and `objdump -d`
apt-get install binutils-arm-none-eabi
# riscv: gnu `as` and `objdump -d`
# install from source (see readme): https://github.com/riscv-collab/riscv-gnu-toolchain
```

## References
* Intel
  * Intel developer manuals - https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
  * Intel 8086 datasheet (16 bit) - https://edge.edx.org/c4x/BITSPilani/EEE231/asset/8086_family_Users_Manual_1_.pdf
  * Intel x86/80386 (32 bit) - https://css.csail.mit.edu/6.858/2014/readings/i386.pdf
  * Intel x86-64 (64 bit) - TODO
  * Assembling and exploring test programs:
    * nasm, web based install: https://www.nasm.us/, `nasm my_program.asm`
    * use `xxd` to see assembled machine code bytes to use as test input for decoding (`xxd -i` provides a nice copy pastable c array syntax to plop into unit tests).
    * why not gnu? gnu married itself to the at&t syntax for intel's ISAs. This project just implements the intel spec to keep things simple.
* ARM
  * https://developer.arm.com/documentation/ddi0602/2025-03/Base-Instructions?lang=en
    * Much better docs than intel, intel just provides a 1000+ page image pdf and says goodluck, arm provides hyper text with links with actual technical writing (concise bullet lists vs pages of wordy paragrpahs).
  * Assembling and exploring test programs:
    * gnu `as` and `objdump -d`
      * ubuntu install: `apt-get install binutils-arm-none-eabi`
      * Examples:
        * `aarch64-none-linux-gnu-as -o add.o add.s`
        * `aarch64-none-linux-gnu-objdump -d add.o`
* RISCV
  * Ratified docs: https://riscv.org/specifications/ratified/
    * Docs deep dive into design decisions and trade-offs, which is greatly appreciated. They get a bit academic/bike sheddy stuff going on at times (ex: defining "hart" vs just using "thread" like everyone else...), but are generally a great resource (intel/older architectures have had sales/business people speak creep into over time).
  * Assembling and exploring test programs:
    * gnu `as` and `objdump -d`
      * install from source (4+ hour build): https://github.com/riscv-collab/riscv-gnu-toolchain
      * aur pre-built binaries: https://aur.archlinux.org/packages/riscv-gnu-toolchain-bin
      * Examples:
        * `riscv64-unknown-elf-as -o add.o add.s`
        * `riscv64-unknown-elf-objdump -d add.o`
          * objdump displays as big-endian
          * retrieve the text section as a c array for embedding in tests:
          * `riscv64-unknown-elf-objcopy -O binary -j .text add.o /dev/stdout | xxd -i`
          * `--disassembler-options=no-aliases` - disable pseudo instruction aliasing
    * link and test in qemu
      * `riscv64-unknown-elf-as -o add.o add.s`
      * `riscv64-unknown-elf-ld -Ttext=0x1000 -e _start -o add add.o`
      * `qemu-system-riscv64 -M virt -nographic -S -s -bios none`
      * `gdb-multiarch ./add`
        * `target remote :1234`
        * `load`
        * `set $pc = 0x1000`
        * `stepi`
        * `info registers t0`
        * `kill`
        * `q`
  * vector intrinsics in c: https://fprox.substack.com/p/risc-v-vector-programming-in-c-with
   * vector instructions: https://godbolt.org/z/x1q8qvdhr
   * for `riscv64-unknown-elf-as` instructions, need to add the 'v' extension `-march=rv64gcv`
     * ex: `riscv64-unknown-elf-as -march=rv64gcv -o vector_misc.o vector_misc.asm`
  * with pause: `riscv64-unknown-elf-as -march=rv64gcv_zihintpause -o rv64i_extras.o rv64i_extras.asm`

