# ssh sandbox
Toy project using the libssh library

## Build/Run
```sh
./build.py
./.build/bin/ssh_sandbox --help
./.build/bin/ssh_sandbox john@127.0.0.1:22 "cd ~/some_dir && ls"
```

## Linux
Install libssh through your package manager (ex: `sudo pacman -Syu libssh-dev`).
Reference system packages and pkg-config (`pkg-config --list-all | grep -i libssh-dev`).

## Windows
Requires:
* MinGW/w64devkit: https://github.com/skeeto/w64devkit/releases
* vcpkg and manually setting an environment variable `VCPKG_ROOT`.
