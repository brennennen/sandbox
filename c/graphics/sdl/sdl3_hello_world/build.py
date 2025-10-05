#!/usr/bin/env python3
import sys, os, subprocess, shlex

def build():
    build_dir = '.build'
    if not os.path.exists(build_dir):
        os.makedirs(build_dir)
    if sys.platform == "win32":
        subprocess.run(shlex.split("cmake .. -G 'MinGW Makefiles'"), cwd=build_dir, check=True)
        subprocess.run(shlex.split("cmake --build ."), cwd=build_dir, check=True)
    elif sys.platform == "linux":
        subprocess.run(shlex.split('cmake ..'), cwd=build_dir, check=True)
        subprocess.run(shlex.split('cmake --build .'), cwd=build_dir, check=True)

def main():
    build()

if __name__ == "__main__":
    main()
