#!/usr/bin/env bash
# 
# Deploys the kafka zookeeper to a local docker container.
#
ZOOKEEPER_DIR=$(dirname "$BASH_SOURCE")
DOCKERFILE="alpine.dockerfile"
KAFKA_DIR=$(realpath $ZOOKEEPER_DIR/..)

CONTAINER_IMAGE_NAME="kafka_admin_image"
CONTAINER_NAME="kafka_admin"

docker stop $CONTAINER_NAME
docker rm $CONTAINER_NAME
docker build -t $CONTAINER_IMAGE_NAME -f $ZOOKEEPER_DIR/$DOCKERFILE $ZOOKEEPER_DIR
docker run -it -p 2181:2181 --net=host --name $CONTAINER_NAME $CONTAINER_IMAGE_NAME bash
#docker run -it -p 2181:2181 --name $CONTAINER_NAME $CONTAINER_IMAGE_NAME bash