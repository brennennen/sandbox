#!/usr/bin/env bash
cd $(dirname $BASH_SOURCE)
docker run \
    --interactive \
    --tty \
    --net=host \
    --name "producer" \
    "producer_image"
cd -
