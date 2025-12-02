/**
 * gcc -O3 ./main.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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

// count each time dial lands on zero
int part1(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }
    
    int dial = 50;
    int zeros = 0;
    char buffer[16];
    while(fgets(buffer, sizeof(buffer), args.file) != NULL) {
        //printf("%s", buffer);
        bool left = buffer[0] == 'L';
        int ticks = atoi(&buffer[1]);
        //printf("   %c - %d\n", left ? 'L' : 'R', ticks);
        if (left) {
            dial = dial - ticks;
            int r = dial % 100;
            if (r < 0) {
                dial = r + 100;
            } else {
                dial = r;
            }
        } else {
            dial = (dial + ticks) % 100;
        }
        if (dial == 0) {
            zeros += 1;
        }
        //printf("   %d\n", dial);
    }
    printf("zeros: %d\n", zeros);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

// count each time dial passes zero or lands on it
int part2(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }
    
    int dial = 50;
    int zeros = 0;
    char buffer[16];
    bool prev_on_zero = false;
    while(fgets(buffer, sizeof(buffer), args.file) != NULL) {
        //printf("%s", buffer);
        bool left = buffer[0] == 'L';
        int ticks = atoi(&buffer[1]);
        //printf("   %c - %d\n", left ? 'L' : 'R', ticks);
        if (left) {
            int wrap_distance = dial;
            if (dial == 0) {
                wrap_distance = 100;
            }
            if (ticks >= wrap_distance) {
                int times_around = 1;
                ticks -= wrap_distance;
                times_around += (ticks / 100);
                //printf("   zero land/wrap left: %d\n", times_around);
                zeros += times_around;
                int r = ticks % 100;
                if (r == 0) {
                    dial = 0;
                } else {
                    dial = 100 - r;
                }
            } else {
                dial = wrap_distance - ticks;
            }
        } else {
            int new_dial = dial + ticks;
            int times_around = new_dial / 100;
            if (times_around > 0) {
                //printf("   zero land/wrap right: %d\n", times_around);
                zeros += times_around;
            }
            dial = new_dial % 100;
        }
    }
    printf("zeros: %d\n", zeros);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    part1(argc, argv);
    part2(argc, argv);
}
