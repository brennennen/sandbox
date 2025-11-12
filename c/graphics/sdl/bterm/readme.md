# BTerm
Toy remote terminal project. Really rough draft.

## Build/Run
```sh
./build.py
cd ./.build/bin && ./main
```

## Linux
Reference system packages and pkg-config (`pkg-config --list-all | grep -i sdl`).

## Windows
Requires:
* MinGW/w64devkit: https://github.com/skeeto/w64devkit/releases
* vcpkg and manually setting an environment variable `VCPKG_ROOT`.
* Uses vcpkg just for libssh, sdl3_ttf and sd3_image are not on vcpkg, and libssh is pain to install any other way than vcpkg, so this is a bit of a mess.
