#!/usr/bin/env python3
import sys, os, subprocess, shlex

build_dir = '.build'
if sys.platform == "win32":
    vcpkg_install_dir = os.path.abspath(".vendor/vcpkg_installed")
    vcpkg_root = os.environ.get('VCPKG_ROOT')
    setup_cmd_old = f"cmake -S . -B {build_dir} -G 'MinGW Makefiles' \
-DCMAKE_TOOLCHAIN_FILE='{vcpkg_root}/scripts/buildsystems/vcpkg.cmake' \
-DVCPKG_TARGET_TRIPLET=x64-mingw-static \
-DVCPKG_INSTALLED_DIR=f{vcpkg_install_dir}"
    setup_cmd = f"cmake -S . -B {build_dir} -G 'MinGW Makefiles' \
-DCMAKE_TOOLCHAIN_FILE='{vcpkg_root}/scripts/buildsystems/vcpkg.cmake' \
-DVCPKG_TARGET_TRIPLET=x64-mingw-static"
    print(setup_cmd)
    subprocess.run(shlex.split(setup_cmd), check=True)
    subprocess.run(shlex.split(f"cmake --build {build_dir}"), check=True)
elif sys.platform == "linux":
    subprocess.run(shlex.split(f"cmake -S . -B {build_dir}"), check=True)
    subprocess.run(shlex.split(f"cmake --build {build_dir}"), check=True)
