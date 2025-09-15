CC = gcc
CFLAGS = -Wall -g -pthread

BUILD_DIR := .build

all: $(BUILD_DIR)/deadlock $(BUILD_DIR)/ordered_lock_schema $(BUILD_DIR)/livelock $(BUILD_DIR)/lock_free_queue

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/deadlock: deadlock.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/ordered_lock_schema: ordered_lock_schema.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/livelock: livelock.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/lock_free_queue: lock_free_queue.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf ./.build
