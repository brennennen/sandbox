#!/usr/bin/env bash
cd $(dirname $BASH_SOURCE)
docker build \
    --tag "consumer_image" \
    .
cd -
