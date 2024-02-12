#
# Dockerfile to build a Kafka Broker container image.
# NOTE: alpine uses musl libc instead of the more common glibc.
#
FROM alpine:3.8

# Arguments can be overriden during a docker build with the --build-arg flag.
ARG kafka_package_extension=".tgz"
ARG kafka_package_name="kafka_2.11-2.0.0"
ARG kafka_download_url="http://www-us.apache.org/dist/kafka/2.0.0/kafka_2.11-2.0.0.tgz"

# Environment variables can be overriden at docker run with the --env flag.
ENV BROKER_PROPERTIES_FILE="broker_0.properties"

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

# Copy over the configuration file template and a wrapper script to update configurations
# from environment variables.
COPY "server.properties" \
     "start_broker.bash" \
     /

CMD ["bash", "start_broker.bash"]
