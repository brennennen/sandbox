#!/usr/bin/env bash
cd $(dirname $BASH_SOURCE)
docker build \
    --tag "broker_image" \
    .
cd -
