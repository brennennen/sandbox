#!/usr/bin/env bash
cd $(dirname $BASH_SOURCE)
docker run \
    --detach \
    --interactive \
    --tty \
    --net=host \
    --name "zookeeper" \
    "zookeeper_image"
docker run \
    --detach \
    --interactive \
    --tty \
    --net=host \
    --name "broker_1" \
    "broker_image"
docker run \
    --detach \
    --interactive \
    --tty \
    --net=host \
    --name "broker_2" \
    "broker_image"
docker run \
    --detach \
    --interactive \
    --tty \
    --net=host \
    --name "broker_3" \
    "broker_image"
docker run \
    --detach \
    --interactive \
    --tty \
    --net=host \
    --name "producer" \
    "producer_image"
docker run \
    --interactive \
    --tty \
    --net=host \
    --name "consumer" \
    "consumer_image"
cd -
