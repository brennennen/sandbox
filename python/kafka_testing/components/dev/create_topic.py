#!/usr/bin/env python3
import time
import json
import argparse
from confluent_kafka.admin import AdminClient, NewTopic

import print_metadata

brokers = "127.0.0.1:9092"

def create_topic(topic_name, partitions, redundancy):
    admin = AdminClient({'bootstrap.servers': brokers})
    new_topic = NewTopic(topic_name, partitions, redundancy)
    #new_topic = NewTopic("test", 1, 1)
    result = admin.create_topics([new_topic])
    time.sleep(1)
    print_metadata.print_partition_count()

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('name')
    parser.add_argument('-p', '--partitions', type=int, default=1)
    parser.add_argument('-r', '--redundancy', type=int, default=1)
    return parser.parse_args()

if __name__ == "__main__":
    arguments = parse_arguments()
    create_topic(arguments.name, arguments.partitions, arguments.redundancy)
