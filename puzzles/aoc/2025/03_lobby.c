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

int find_2_largest_joltage(char* line, int line_length) {
    int large1 = 0;
    int large1_index = 0;
    for (int i = 0; i < line_length - 1; i++) {
        int num = line[i] - '0';
        //printf("%d", num);
        if (num > large1) {
            large1 = num;
            large1_index = i;
        }
    }
    //printf("large1: %d\n", large1);

    int large2 = 0;
    int large2_index = 0;
    for (int i = large1_index + 1; i < line_length; i++) {
        int num = line[i] - '0';
        if (num > large2) {
            large2 = num;
            large2_index = 2;
        }
    }
    //printf("large2: %d\n", large2);
    return((large1 * 10) + large2);
}

int part1(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }

    int total_joltage = 0;
    char buffer[128] = {0};
    while(fgets(buffer, sizeof(buffer), args.file) != NULL) {
        int buff_size = strlen(buffer);
        buffer[buff_size - 1] = '\0';
        //printf("line: %s\n", buffer);
        int line_joltage = find_2_largest_joltage(buffer, buff_size - 1);
        total_joltage += line_joltage;
    }
    printf("p1 joltage: %d\n", total_joltage);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

uint64_t find_12_largest_joltage(char* line, int line_length) {
    uint64_t joltage = 0;
    int large1 = 0;
    int large1_index = 0;
    for (int i = 0; i < line_length - 11; i++) {
        int num = line[i] - '0';
        if (num > large1) {
            large1 = num;
            large1_index = i;
        }
    }
    //printf("large1: %d\n", large1);
    joltage += (large1 * pow(10, 11));
    //printf("joltage: %llu\n", joltage);

    int large2 = 0;
    int large2_index = 0;
    for (int i = large1_index + 1; i < line_length - 10; i++) {
        int num = line[i] - '0';
        if (num > large2) {
            large2 = num;
            large2_index = i;
        }
    }
    //printf("large2: %d\n", large2);
    joltage += (large2 * pow(10, 10));
    //printf("joltage: %llu\n", joltage);

    int large3 = 0;
    int large3_index = 0;
    for (int i = large2_index + 1; i < line_length - 9; i++) {
        int num = line[i] - '0';
        if (num > large3) {
            large3 = num;
            large3_index = i;
        }
    }
    //printf("large3: %d\n", large3);
    joltage += (large3 * pow(10, 9));
    //printf("joltage: %llu\n", joltage);


    int large4 = 0;
    int large4_index = 0;
    for (int i = large3_index + 1; i < line_length - 8; i++) {
        int num = line[i] - '0';
        if (num > large4) {
            large4 = num;
            large4_index = i;
        }
    }
    joltage += (large4 * pow(10, 8));

    int large5 = 0;
    int large5_index = 0;
    for (int i = large4_index + 1; i < line_length - 7; i++) {
        int num = line[i] - '0';
        if (num > large5) {
            large5 = num;
            large5_index = i;
        }
    }
    joltage += (large5 * pow(10, 7));

    int large6 = 0;
    int large6_index = 0;
    for (int i = large5_index + 1; i < line_length - 6; i++) {
        int num = line[i] - '0';
        if (num > large6) {
            large6 = num;
            large6_index = i;
        }
    }
    joltage += (large6 * pow(10, 6));

    int large7 = 0;
    int large7_index = 0;
    for (int i = large6_index + 1; i < line_length - 5; i++) {
        int num = line[i] - '0';
        if (num > large7) {
            large7 = num;
            large7_index = i;
        }
    }
    joltage += (large7 * pow(10, 5));

    int large8 = 0;
    int large8_index = 0;
    for (int i = large7_index + 1; i < line_length - 4; i++) {
        int num = line[i] - '0';
        if (num > large8) {
            large8 = num;
            large8_index = i;
        }
    }
    joltage += (large8 * pow(10, 4));

    int large9 = 0;
    int large9_index = 0;
    for (int i = large8_index + 1; i < line_length - 3; i++) {
        int num = line[i] - '0';
        if (num > large9) {
            large9 = num;
            large9_index = i;
        }
    }
    joltage += (large9 * pow(10, 3));

    int large10 = 0;
    int large10_index = 0;
    for (int i = large9_index + 1; i < line_length - 2; i++) {
        int num = line[i] - '0';
        if (num > large10) {
            large10 = num;
            large10_index = i;
        }
    }
    joltage += (large10 * pow(10, 2));

    int large11 = 0;
    int large11_index = 0;
    for (int i = large10_index + 1; i < line_length - 1; i++) {
        int num = line[i] - '0';
        if (num > large11) {
            large11 = num;
            large11_index = i;
        }
    }
    joltage += (large11 * 10);

    int large12 = 0;
    int large12_index = 0;
    for (int i = large11_index + 1; i < line_length; i++) {
        int num = line[i] - '0';
        if (num > large12) {
            large12 = num;
            large12_index = i;
        }
    }
    joltage += large12;

    //printf("line joltage: %llu\n", joltage);
    return(joltage);
}

int part2(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }

    uint64_t total_joltage = 0;
    char buffer[128] = {0};
    while(fgets(buffer, sizeof(buffer), args.file) != NULL) {
        int buff_size = strlen(buffer);
        buffer[buff_size - 1] = '\0';
        //printf("line: %s\n", buffer);
        uint64_t line_joltage = find_12_largest_joltage(buffer, buff_size - 1);
        total_joltage += line_joltage;
    }
    printf("p2 joltage: %llu\n", total_joltage);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    part1(argc, argv);
    part2(argc, argv);
}
