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
    uint64_t low;
    uint64_t high;
} range_t;

typedef struct {
    range_t fresh_ranges[256];
    int ranges_count;
    uint64_t ids[2048];
    int ids_count;
} inventory_t;

typedef enum {
    PM_FRESH_RANGES,
    PM_INGREDIENT_IDS
} parse_mode_t;

void parse_inventory(FILE* file, inventory_t* inventory) {
    char buffer[256] = {0};
    parse_mode_t parse_mode = PM_FRESH_RANGES;
    while(fgets(buffer, sizeof(buffer), file) != NULL) {
        if (strncmp(buffer, "\n", 2) == 0) {
            parse_mode = PM_INGREDIENT_IDS;
        }
        switch(parse_mode) {
            case PM_FRESH_RANGES: {
                uint64_t low = 0;
                uint64_t high = 0;
                sscanf(buffer, "%llu-%llu", &low, &high);
                inventory->fresh_ranges[inventory->ranges_count].low = low;
                inventory->fresh_ranges[inventory->ranges_count].high = high;
                inventory->ranges_count += 1;
                break;
            }
            case PM_INGREDIENT_IDS: {
                inventory->ids[inventory->ids_count] = atoll(buffer);
                inventory->ids_count += 1;
                break;
            }
        }
    }
}

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

void print_inventory(inventory_t* inventory) {
    for (int i = 0; i < inventory->ranges_count; i++) {
        printf("range: %llu-%llu\n", inventory->fresh_ranges[i].low, inventory->fresh_ranges[i].high);
    }
    for (int i = 0; i < inventory->ids_count; i++) {
        printf("id: %llu\n", inventory->ids[i]);
    }
}

int part1(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }
    inventory_t inventory = {0};
    parse_inventory(args.file, &inventory);
    //print_inventory(&inventory);

    int fresh_count = 0;
    for (int i = 0; i < inventory.ids_count; i++) {
        //printf("id: %llu\n", inventory.ids[i]);
        for (int j = 0; j < inventory.ranges_count; j++) {
            if (inventory.fresh_ranges[j].low <= inventory.ids[i] 
                && inventory.ids[i] <= inventory.fresh_ranges[j].high
            ) {
                fresh_count += 1;
                //printf("   fresh!\n");
                break;
            }
        }
    }
    printf("p1 fresh: %d\n", fresh_count);

    fclose(args.file);
    return(EXIT_SUCCESS);
}

void print_ranges(inventory_t* inventory) {
    for (int i = 0; i < inventory->ranges_count; i++) {
        printf("range: %llu-%llu\n", inventory->fresh_ranges[i].low, inventory->fresh_ranges[i].high);
    }
}

int try_merge_ranges(inventory_t* inventory) {
    int merged_ranges = 0;
    // walk backwards to be able to do merges in place easier
    for (int i = inventory->ranges_count - 1; i >= 0; i--) {
        for (int j = 0; j < inventory->ranges_count; j++) {
            if (i == j) {
                continue;
            }
    
            if ((inventory->fresh_ranges[i].low <= inventory->fresh_ranges[j].high) 
                && (inventory->fresh_ranges[j].low <= inventory->fresh_ranges[i].high)
            ) {
                uint64_t new_low = inventory->fresh_ranges[i].low;
                if (inventory->fresh_ranges[j].low < new_low) {
                    new_low = inventory->fresh_ranges[j].low;
                }
                uint64_t new_high = inventory->fresh_ranges[i].high;
                if (inventory->fresh_ranges[j].high > new_high) {
                    new_high = inventory->fresh_ranges[j].high;
                }

                // printf("merge! i: %d (%llu-%llu) and j: %d (%llu-%llu) into: (%llu-%llu) \n", 
                //     i, inventory->fresh_ranges[i].low, inventory->fresh_ranges[i].high,
                //     j, inventory->fresh_ranges[j].low, inventory->fresh_ranges[j].high,
                //     new_low, new_high
                // );
                inventory->fresh_ranges[i].low = new_low;
                inventory->fresh_ranges[i].high = new_high;
                inventory->fresh_ranges[j] = inventory->fresh_ranges[inventory->ranges_count - 1];
                inventory->ranges_count -= 1;
                merged_ranges += 1;
                j--;
                goto end;
            }
        }
    }
end:
    //printf("merges: %d\n", merged_ranges);
    return merged_ranges;
}

int part2(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }
    inventory_t inventory = {0};
    parse_inventory(args.file, &inventory);
    //print_ranges(&inventory);

    while (try_merge_ranges(&inventory) != 0) {
        //print_ranges(&inventory);
    }
    //print_ranges(&inventory);
    uint64_t total_range_count = 0;
    for (int i = 0; i < inventory.ranges_count; i++) {
        uint64_t range_count = inventory.fresh_ranges[i].high - inventory.fresh_ranges[i].low + 1;
        total_range_count += range_count;
    }
    printf("p2 total ids: %llu\n", total_range_count);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    part1(argc, argv);
    part2(argc, argv);
}
