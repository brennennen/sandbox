
import sys
import threading
import datetime

from confluent_kafka import Producer

BROKERS = "127.0.0.1:9092"

def configure_producer():
    conf = {"bootstrap.servers": BROKERS}
    producer = Producer(**conf)
    return producer
    
def produce_on_interval(producer):
    threading.Timer(1.0, produce_on_interval, [producer]).start()
    try:
        current_time = datetime.datetime.now()
        producer.produce("test_topic", f"test message {current_time}", callback=delivery_callback)
        producer.poll(0)
        producer.flush()
    except BufferError:
        print("Local producer queue is full.")

def delivery_callback(error, message):
    if error:
        print(f"Error: {error}")
    else:
        print(f"Delivered: {message}")

if __name__ == "__main__":
    producer = configure_producer()
    produce_on_interval(producer)
