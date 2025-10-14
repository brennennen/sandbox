#!/usr/bin/env python3
import sys, os, subprocess, shlex

build_dir = '.build'
if not os.path.exists(build_dir):
    os.makedirs(build_dir)
if sys.platform == "win32":
    vcpkg_root = os.environ.get('VCPKG_ROOT')
    subprocess.run(shlex.split(f"cmake .. -G 'Visual Studio 17 2022' -DCMAKE_TOOLCHAIN_FILE='{vcpkg_root}/scripts/buildsystems/vcpkg.cmake'"), cwd=build_dir, check=True)
    subprocess.run(shlex.split("cmake --build ."), cwd=build_dir, check=True)
elif sys.platform == "linux":
    subprocess.run(shlex.split('cmake ..'), cwd=build_dir, check=True)
    subprocess.run(shlex.split('cmake --build .'), cwd=build_dir, check=True)
