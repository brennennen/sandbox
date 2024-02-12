
import json
import time
import argparse
from confluent_kafka.admin import AdminClient, NewTopic

import print_metadata

brokers = "127.0.0.1:9092"

admin_client = AdminClient({'bootstrap.servers': brokers})

def delete_topics_with_prefix(topic_prefix):
    #topics_list = admin_client.list_topics().topics.keys()
    topic_list = admin_client.list_topics().topics.keys()
    matching_topic_list = []
    for topic in topic_list:
        if topic_prefix in topic:
            matching_topic_list.append(topic)
    if (len(matching_topic_list) != 0):
        print(f"Found matches: {matching_topic_list}")
        result = admin_client.delete_topics(matching_topic_list)
        
        time.sleep(25)
        print_metadata.print_partition_count()


def delete_topic(topic_name):
    result = admin_client.delete_topics([topic_name])
    time.sleep(5)
    print_metadata.print_partition_count()

def parse_arguments():
    parser = argparse.ArgumentParser()
    parser.add_argument('name')
    return parser.parse_args()

if __name__ == "__main__":
    #arguments = parse_arguments()
    #delete_topic(arguments.name)
    delete_topics_with_prefix("bs_test_")
