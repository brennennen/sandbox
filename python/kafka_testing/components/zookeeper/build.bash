#!/usr/bin/env bash
cd $(dirname $BASH_SOURCE)
docker build \
    --tag "zookeeper_image" \
    .
cd -
