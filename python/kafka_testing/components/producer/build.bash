#!/usr/bin/env bash
cd $(dirname $BASH_SOURCE)
docker build \
    --tag "producer_image" \
    .
cd -
