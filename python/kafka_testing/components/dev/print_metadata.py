#!/usr/bin/env python

import json
from confluent_kafka.admin import AdminClient

brokers = "127.0.0.1:9092"
#brokers = "0.0.0.0:9092"


def print_topics():
    admin = AdminClient({'bootstrap.servers': brokers})
    cluster_metadata = admin.list_topics()
    print(cluster_metadata.topics.keys())

def print_metadata_raw():
    admin = AdminClient({'bootstrap.servers': brokers})
    cluster_metadata = admin.list_topics()
    print(cluster_metadata.__dict__)

def print_partition_count():
    admin = AdminClient({'bootstrap.servers': brokers})
    cluster_metadata = admin.list_topics()
    topics = cluster_metadata.topics
    topic_partitions = {}
    for topic, topic_value in topics.items():
        topic_partitions[topic] = len(topic_value.partitions)
    print(json.dumps(topic_partitions, indent=4))

if __name__ == "__main__":
    #print_metadata_raw()
    print_topics()
    #print_partition_count()
