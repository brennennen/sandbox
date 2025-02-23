# 8086 Emulator
Toy partial 8086 emulator to review and deeper learn x86/x64 architecture.

## Build
Currently uses some c23 features when available. Expects at least gcc-14 or later to use
these c23 features on linux.

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

Windows:
```
premake5 vs2022
MSBuild.exe ./.build/PremakeSandbox.sln
./bin/debug/sandbox.exe
./bin/Debug/test_emulate8086.exe
./bin/Debug/test.exe
```

## Build env Setup
* Tooling:
  * premake - https://premake.github.io
* Dependencies:
  * criterion - https://github.com/Snaipe/Criterion
* Misc:
  * nasm - https://www.nasm.us/

Linux (Ubuntu):
```
# Install premake as a build generator
mkdir -p ~/bin && cd ~/bin
wget https://github.com/premake/premake-core/releases/download/v5.0.0-beta4/premake-5.0.0-beta4-linux.tar.gz
tar -xvzf premake-5.0.0-beta4-linux.tar.gz
chmod +x ./premake5

# Install criterion dev package for unit testing
apt-get install libcriterion-dev
```

## References
* Intel developer manuals - https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html
* Intel 8086 datasheet (16 bit) - https://edge.edx.org/c4x/BITSPilani/EEE231/asset/8086_family_Users_Manual_1_.pdf
* Intel x86/80386 (32 bit) - https://css.csail.mit.edu/6.858/2014/readings/i386.pdf
* Intel x86-64 (64 bit) -