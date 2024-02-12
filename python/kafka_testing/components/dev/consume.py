import sys
import os
import time

import confluent_kafka
from confluent_kafka import Consumer, TopicPartition, KafkaError

BROKERS = "127.0.0.1:9092"


def build_consumer_from_smallest_offset(topic_name, partition=0):
    """! Builds a consumer pointing at the earliest offset to re-consume all messages. """
    def set_offset_earliest(consumer, partitions):
        """! Sets the offset to the beginning to reconsume all messages. """
        for partition in partitions:
            partition.offset = confluent_kafka.OFFSET_BEGINNING
        consumer.assign(partitions)
    topic_partition = TopicPartition(topic_name, partition, offset=confluent_kafka.OFFSET_BEGINNING)
    consumer = Consumer({
            "bootstrap.servers": BROKERS,
            "group.id": "test_group",
            "auto.offset.reset": "earliest",
            "default.topic.config": {
                "auto.offset.reset": "smallest",
            }
        })
    consumer.subscribe([topic_name], on_assign=set_offset_earliest)
    low, high = consumer.get_watermark_offsets(topic_partition)
    print(f"Topic Name: '{topic_name}'")
    print(f"Offsets: (Low: {low}, High: {high})")
    return consumer

def build_consumer_from_latest_offset(topic_name, partition=0):
    """! Builds a consumer pointing at the """
    topic = TopicPartition(topic_name, partition)
    consumer = Consumer({
            "bootstrap.servers": BROKERS,
            "group.id": "test_group",
            "auto.offset.reset": "latest",
            "default.topic.config": {
                "auto.offset.reset": "latest",
            }
        })
    consumer.subscribe([topic_name])
    low, high = consumer.get_watermark_offsets(topic)
    print(f"Offsets: (Low: {low}, High: {high})")
    return consumer
    

def consume_to_console(consumer):
    print("Starting to listen for messages...")
    while True:
        message = consumer.poll(1.0)
        if message is None:
            print(f"poll: {message}")
            continue
        else:
            #print(f"poll: {message.value().hex()}")
            if message.error():
                if message.error().code() == KafkaError._PARTITION_EOF:
                    continue
                else:
                    print(message.error())
                    break
        print(f"Received Message: {message.value()}")
    consumer.close()

if __name__ == "__main__":
    consumer = build_consumer_from_smallest_offset("test_topic", 0)
    consume_to_console(consumer)

    #consumer = build_consumer_from_latest_offset("bs_test__bnsf_bos__bnsf.l.bnsf.9321", 0)
    #consume_to_console(consumer)