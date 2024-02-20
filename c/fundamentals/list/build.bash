#!/usr/bin/env bash
set -x
gcc ./list.c \
    ./test__list.c \
    --output test__list.exe
./test__list.exe
