# Emulator
Toy single program emulator to review and deeper learn popular processors, ISAs, and ABIs.

## Build
Currently uses some c23 features when available. Expects at least gcc-14 or later to use
these c23 features on linux. I used premake with the intention to support any OS, but I haven't tested on any other OSes.

Linux
```sh
# One time
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
./bin/Debug/test_emulate8086
./bin/Debug/test
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
```

## References
* Intel
  * Intel developer manuals - https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
  * Intel 8086 datasheet (16 bit) - https://edge.edx.org/c4x/BITSPilani/EEE231/asset/8086_family_Users_Manual_1_.pdf
  * Intel x86/80386 (32 bit) - https://css.csail.mit.edu/6.858/2014/readings/i386.pdf
  * Intel x86-64 (64 bit) -
* ARM
  * https://developer.arm.com/documentation/ddi0602/2025-03/Base-Instructions?lang=en
    * Much better docs than intel, intel just provides a 1000+ page image pdf and says goodluck, arm provides hyper text with links with actual technical writing (concise bullet lists vs pages of wordy paragrpahs).

