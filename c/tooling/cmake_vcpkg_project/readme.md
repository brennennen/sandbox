# CMake vcpkg Project
Minmal project to test using cmake with vcpkg.

## Build/Run
```sh
./build.py
./.build/bin/main
```

## Linux
Reference system packages and pkg-config (`pkg-config --list-all | grep -i sdl`).

## Windows
Requires vcpkg: https://vcpkg.io/en/
Also requires setting an environment variable `VCPKG_ROOT` to the install directory.
