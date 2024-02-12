#!/usr/bin/env bash
cd $(dirname $BASH_SOURCE)
docker run \
    --interactive \
    --tty \
    --net=host \
    --name "broker_0" \
    --env BROKER_ID="0" \
    --env LISTENERS="PLAINTEXT://:9092" \
    --env ADVERTISED_LISTENERS="PLAINTEXT://:9092" \
    "broker_image"
cd -
