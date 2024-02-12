#!/usr/bin/env bash
cd $(dirname $BASH_SOURCE)
docker run \
    --interactive \
    --tty \
    --net=host \
    --name "zookeeper" \
    "zookeeper_image"
cd -
