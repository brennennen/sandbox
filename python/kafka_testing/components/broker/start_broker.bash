#!/usr/bin/env bash

# Update the configuration file with configurations passed in as environment variable.
sed -i \
    -e "s|{{BROKER_ID}}|${BROKER_ID:-0}|g" \
    -e "s|{{LISTENERS}}|${LISTENERS:-PLAINTEXT://:9092}|g" \
    -e "s|{{ADVERTISED_LISTENERS}}|${LISTENERS:-PLAINTEXT://:9092}|g" \
    "server.properties"

bash /kafka/bin/kafka-server-start.sh "server.properties"
