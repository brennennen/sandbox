/**
 * Sandbox program to demo the arena allocator.
 * For each frame, allocates 128 "greeblies" (some arbitrary data) at the beginning of the frame,
 * does some work with them, and then frees them by the end of the frame.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "arena.h"

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep(ms * 1000)
#endif

#define TARGET_FPS 60
#define TARGET_FRAME_TIME_US (1000000 / TARGET_FPS)

#define START_MIN_GREEBLIES 100
#define START_MAX_GREEBLIES 100

float random_float(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

typedef struct {
    float some_data;
    uint8_t other_data[32];
} greebly_t;

typedef struct {
    int count;
    int capacity;
    greebly_t** items;
} greebly_list_t;


int main(int argc, char* argv[]) {
    printf("starting memory arena sandbox:\n");
    srand(time(NULL));

    arena_t greeblies_frame_arena = arena_create(sizeof(greebly_t) * 512);
    greebly_list_t greebly_list = {0};

    struct timespec start;
    struct timespec end;
    float frame_time_us;
    long frame_count = 0;

    int greeblies_count = 128;

    while(1) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        frame_count += 1;

        if (frame_count % 100 == 0) {
            greeblies_count = greeblies_count * 2;
            arena_prune(&greeblies_frame_arena);
        }

        // allocate the greeblies
        int required_capacity = greeblies_count;
        if (greebly_list.capacity < required_capacity) {
            greebly_list.capacity = required_capacity;
            greebly_list.items = arena_alloc(&greeblies_frame_arena, sizeof(greebly_t*) * greebly_list.capacity);
        }
        for (int i = 0; i < greeblies_count; i++) {
            greebly_t* greebly = arena_alloc(&greeblies_frame_arena, sizeof(greebly_t));
            greebly->some_data = random_float(0.0f, 1.0f);
            for (int j = 0; j < 32; j++) {
                greebly->other_data[j] = rand() % 256;
            }
        }

        // do some work with the greeblies
        float greebly_avg_some_data_running_sum = 0.0;
        for (int i = 0; i < greebly_list.count; i++) {
            // todo: get greebly current greebly here!
            greebly_t* greebly = greebly_list.items[i];
            greebly_avg_some_data_running_sum += greebly->some_data;
        }
        float greebly_avg_some_data = greebly_avg_some_data_running_sum / greeblies_count;

        // free the greeblies
        arena_reset(&greeblies_frame_arena);
        greebly_list.count = 0;

        clock_gettime(CLOCK_MONOTONIC, &end);
        frame_time_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        printf("frame: %lu, time: %f, greeblies: %d, chains: %d, head offset: %ld\n",
            frame_count, frame_time_us, greeblies_count, greeblies_frame_arena.chain_count,
            greeblies_frame_arena.head->offset);
        if (frame_time_us < TARGET_FRAME_TIME_US) {
            usleep(TARGET_FRAME_TIME_US - frame_time_us);
        }
    }

    arena_free(&greeblies_frame_arena);
    return(0);
}
