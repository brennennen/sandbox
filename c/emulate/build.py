#!/usr/bin/env python3
import sys, os, subprocess, shlex

build_dir = '.build'
if not os.path.exists(build_dir):
    os.makedirs(build_dir)
subprocess.run(shlex.split(f"premake5 ecc"), check=True) # builds "compile_commands.json" for clangd
subprocess.run(shlex.split(f"premake5 gmake"), check=True)
subprocess.run(shlex.split("make -C ./.build/"), check=True)
