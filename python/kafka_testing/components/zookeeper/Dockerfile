#
# Dockerfile to build a Kafka Zookeeper container image.
# NOTE: alpine uses musl libc instead of the more common glibc.
#
FROM alpine:3.8

# Arguments can be overriden during a docker build with the --build-arg flag.
ARG kafka_package_extension=".tgz"
ARG kafka_package_name="kafka_2.11-2.0.0"
ARG kafka_download_url="http://www-us.apache.org/dist/kafka/2.0.0/kafka_2.11-2.0.0.tgz"

COPY "zookeeper.properties" /

# Install the necessary packages to install and run the kafka zookeeper.
RUN apk update \
    && apk add \
        bash \
        wget \
        openjdk8-jre \
    && rm /var/cache/apk/*

# Download and unpack kafka.
RUN wget ${kafka_download_url} \
    && tar -xzf "${kafka_package_name}${kafka_package_extension}" \
    && mv ${kafka_package_name} "kafka" \
    && rm "${kafka_package_name}${kafka_package_extension}" \
    && apk del wget

# The Kafka zookeper default port is 2181.
# This should mirror clientPort in zookeeper.properties.
EXPOSE 2181

# Start the zookeeper server with the specified configurations.
# This can be overriden during a docker run by adding a command as the last positional argument.
# It's common to override with `bash` and then start the command manually to debug issues.
CMD ["bash", "/kafka/bin/zookeeper-server-start.sh", "zookeeper.properties"]