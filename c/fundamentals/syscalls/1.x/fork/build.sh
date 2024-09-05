#!/usr/bin/env sh
set -x

gcc ./fork.c \
    -no-pie \
    --output fork.out
