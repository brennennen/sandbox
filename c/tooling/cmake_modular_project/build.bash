#!/usr/bin/env bash
mkdir -p ./.build
cd ./.build
cmake ..
make
./debug/bin/module_a
./debug/bin/module_b
./debug/bin/module_c
