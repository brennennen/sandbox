FROM alpine:3.8

ARG librdkafka_download_url="https://github.com/edenhill/librdkafka/archive/v0.11.6.tar.gz"
ARG librdkafka_package_name="v0.11.6.tar.gz"
ARG librdkafka_uncompressed_name="librdkafka-0.11.6"

COPY "requirements.txt" \
     "print_metadata.py" \
     "create_topic.py" \
     "delete_topic.py" \
     "consume.py" \
     "produce.py" \
     /

RUN apk update \
    && apk add \
        bash \
        wget \
        nano \
        linux-headers \
        musl-dev \
        libc-dev \
        gcc \
        g++ \
        make \
        python3-dev \
        librdkafka \
        librdkafka-dev \
        python3 \
    && rm /var/cache/apk/*

# Install librdkafka from source and then install the confluent kafka python wrapper.
RUN wget ${librdkafka_download_url} \
    && tar -xzf "${librdkafka_package_name}" \
    && rm "${librdkafka_package_name}" \
    && cd ${librdkafka_uncompressed_name} \
    && ./configure \
    && make \
    && make install \
    && cd / \
    && python3 -m pip install -r requirements.txt

# Not intended to be automated, jump in and call scripts like "print_metadata.py"
CMD ["bash"]
