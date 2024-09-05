#!/usr/bin/env sh
set -x

gcc ./hello.c \
    -no-pie \
    --output hello.out

gcc ./debugger.c \
    -no-pie \
    --output debugger.out
