# 8086 Disassembler
Toy partial 8086 emulator to review how cpus work.

## Build
Linux
```
premake5 gmake
make -C ./.build/
./bin/Debug/sandbox.exe
./bin/Debug/test.exe
```

Windows:
```
premake5 vs2022
MSBuild.exe ./.build/PremakeSandbox.sln
./bin/debug/sandbox.exe
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
