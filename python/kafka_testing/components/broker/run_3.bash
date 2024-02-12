#!/usr/bin/env bash
cd $(dirname $BASH_SOURCE)
docker run \
    --detach \
    --interactive \
    --tty \
    --net=host \
    --name "broker_0" \
    --env BROKER_ID="0" \
    --env LISTENERS="PLAINTEXT://:9092" \
    --env ADVERTISED_LISTENERS="PLAINTEXT://:9092" \
    "broker_image"
docker run \
    --detach \
    --interactive \
    --tty \
    --net=host \
    --name "broker_1" \
    --env BROKER_ID="1" \
    --env LISTENERS="PLAINTEXT://:9093" \
    --env ADVERTISED_LISTENERS="PLAINTEXT://:9093" \
    "broker_image"
docker run \
    --detach \
    --interactive \
    --tty \
    --net=host \
    --name "broker_2" \
    --env BROKER_ID="2" \
    --env LISTENERS="PLAINTEXT://:9094" \
    --env ADVERTISED_LISTENERS="PLAINTEXT://:9094" \
    "broker_image"
cd -
