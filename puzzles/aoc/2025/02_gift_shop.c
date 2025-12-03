/**
 * gcc -O3 ./main.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef enum {
    RES_FAILURE = 0,
    RES_SUCCESS
} result_t;

typedef struct {
    FILE* file;
} arguments_t;

typedef struct {
    uint64_t start;
    int start_digits;
    uint64_t end;
    int end_digits;
} range_t;

result_t parse_arguments(int argc, char* argv[], arguments_t* out_args) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filepath>\n", argv[0]);
        return(RES_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        fprintf(stderr, "failed to open file\n");
        return(RES_FAILURE);
    }
    out_args->file = file;
    return(RES_SUCCESS);
}

typedef enum {
    PRM_START,
    PRM_END
} parse_ranges_mode_t;

result_t parse_ranges(FILE* file, range_t* ranges, int ranges_size, int* ranges_count) {
    int ch = 0;
    char start_string[256] = {0};
    int start_string_index = 0;
    char end_string[256] = {0};
    int end_string_index = 0;

    parse_ranges_mode_t mode = PRM_START;
    int ranges_index = 0;

    while (ch != EOF) {
        ch = fgetc(file);
        switch(mode) {
            case PRM_START: {
                if (ch == '-') {
                    start_string[start_string_index] = '\0';
                    mode = PRM_END;
                } else if (isdigit(ch)) {
                    start_string[start_string_index] = ch;
                    start_string_index += 1;
                }
                break;
            }
            case PRM_END: {
                if (ch == ',' || ch == EOF || ch == '\n') {
                    if (end_string_index > 0) {
                        end_string[end_string_index] = '\0';
                        ranges[ranges_index].start = atoll(start_string);
                        ranges[ranges_index].start_digits = start_string_index;
                        ranges[ranges_index].end = atoll(end_string);
                        ranges[ranges_index].end_digits = end_string_index;
                        ranges_index += 1;
                        start_string_index = 0;
                        end_string_index = 0;
                        *ranges_count += 1;
                        mode = PRM_START;
                    }
                } else if (isdigit(ch)) {
                    end_string[end_string_index] = ch;
                    end_string_index += 1;
                }
                break;
            }
        }
    }
}

uint64_t count_invalid_ids_in_range_part1(range_t* range) {
    //printf("range: %llu - %llu (%d, %d)\n", range->start, range->end, range->start_digits, range->end_digits);
    if (range->start_digits == range->end_digits
        && range->start_digits % 2 == 1 
    ) {
        //printf("skipping odd range\n");
        return(0);
    }
    int i_digits = range->start_digits;
    uint64_t invalid_ids_in_range = 0;
    
    uint64_t next_10s_place = pow(10, i_digits);
    uint64_t middle_place = pow(10, i_digits/2);
    for (uint64_t i = range->start; i <= range->end; i++) {
        if (i >= next_10s_place) {
            i_digits += 1;
            next_10s_place *= 10;
            middle_place = pow(10, i_digits / 2);
        }
        if (i_digits % 2 == 0) {
            uint64_t high = i / middle_place;
            uint64_t low  = i % middle_place;
            if (high == low) {
                invalid_ids_in_range += i;
            }
        }
    }
    return(invalid_ids_in_range);
}

// count invalid ids (single repeats only)
int part1(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }
    uint64_t total_invalid_ids = 0;
    range_t ranges[64] = {0};
    int ranges_count = 0;
    parse_ranges(args.file, ranges, sizeof(ranges), &ranges_count);
    for (int i = 0; i < ranges_count; i++) {
        total_invalid_ids += count_invalid_ids_in_range_part1(&ranges[i]);
    }

    printf("part1 total_invalid_ids: %llu\n", total_invalid_ids);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

bool is_repeating_part2(uint64_t num) {
    char num_string[64];
    snprintf(num_string, sizeof(num_string), "%llu", num);
    int total_len = strlen(num_string);
    for (int chunk_len = 1; chunk_len <= total_len / 2; chunk_len++) {
        if (total_len % chunk_len != 0) {
            continue;
        }
        bool all_match = true;
        for (int i = chunk_len; i < total_len; i += chunk_len) {
            if (strncmp(num_string, num_string + i, chunk_len) != 0) {
                all_match = false;
                break;
            }
        }
        if (all_match) {
            return(true);
        }
    }
    return(false);
}

uint64_t count_invalid_ids_in_range_part2(range_t* range) {
    //printf("range: %llu - %llu (%d, %d)\n", range->start, range->end, range->start_digits, range->end_digits);
    int i_digits = range->start_digits;
    uint64_t invalid_ids_in_range = 0;
    
    uint64_t next_10s_place = pow(10, i_digits);
    for (uint64_t i = range->start; i <= range->end; i++) {
        if (i >= next_10s_place) {
            i_digits += 1;
            next_10s_place *= 10;
        }
        if (is_repeating_part2(i)) {
            invalid_ids_in_range += i;
        }
    }
    return(invalid_ids_in_range);
}

// count invalid ids (single or multiple repeats)
int part2(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }
    uint64_t total_invalid_ids = 0;
    range_t ranges[64] = {0};
    int ranges_count = 0;
    parse_ranges(args.file, ranges, sizeof(ranges), &ranges_count);
    for (int i = 0; i < ranges_count; i++) {
        total_invalid_ids += count_invalid_ids_in_range_part2(&ranges[i]);
    }

    printf("part2 total_invalid_ids: %llu\n", total_invalid_ids);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    part1(argc, argv);
    part2(argc, argv);
}
