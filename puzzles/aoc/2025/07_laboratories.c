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

typedef enum {
    CT_NONE,
    CT_START,
    CT_OPEN,
    CT_SPLITTER,
    CT_BEAM,
} cell_type_t;

typedef struct {
    int r;
    int c;
} coord_t;

typedef struct {
    bool started;
    coord_t start;
    int splits_count;
    int beam_heads_size;
    coord_t beam_heads[256];
    int cells_size;
    cell_type_t cells[256][256];
} tachyon_simulation_t;

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

void parse_sim(tachyon_simulation_t* sim, FILE* file) {
    char buffer[256] = {0};
    bool first_row = true;
    int r = 0;
    while(fgets(buffer, sizeof(buffer), file) != NULL) {
        int map_row_size = strlen(buffer) - 1;
        buffer[map_row_size] = '\0';
        if (first_row) {
            sim->cells_size = map_row_size;
            //printf("size: %d\n", map_row_size);
            first_row = false;
        }
        //printf("line: %s\n", buffer);
        
        for(int c = 0; c < map_row_size; c++) {
            if (buffer[c] == 'S') {
                sim->cells[r][c] = CT_START;
                sim->start.r = r;
                sim->start.c = c;
            } else if (buffer[c] == '^') {
                sim->cells[r][c] = CT_SPLITTER;
            } else if (buffer[c] == '.') {
                sim->cells[r][c] = CT_OPEN;
            }
        }
        r++;
    }
}

void print_sim(tachyon_simulation_t* sim) {
    printf("sim: beam_heads: %d\n", sim->beam_heads_size);
    for(int r = 0; r < sim->cells_size; r++) {
        for (int c = 0; c < sim->cells_size; c++) {
            char cell_char = '.';
            if (sim->cells[r][c] == CT_START) {
                cell_char = 'S';
            } else if (sim->cells[r][c] == CT_SPLITTER) {
                cell_char = '^';
            } else if (sim->cells[r][c] == CT_OPEN) {
                cell_char = '.';
            } else if (sim->cells[r][c] == CT_BEAM) {
                cell_char = '|';
            }
            printf("%c", cell_char);
        }
        printf("\n");
    }
}

void simulate_single_step(tachyon_simulation_t* sim) {
    for (int i = sim->beam_heads_size - 1; i >= 0; i--) {
        //printf("stepping beam: %d (%d, %d)\n", i, sim->beam_heads[i].r, sim->beam_heads[i].c);
        coord_t* head = &sim->beam_heads[i];
        if (head->r + 1 >= sim->cells_size) {
            //printf("   [%d] off of map, deleting\n", i);
            sim->beam_heads[i].r = sim->beam_heads[sim->beam_heads_size - 1].r;
            sim->beam_heads[i].c = sim->beam_heads[sim->beam_heads_size - 1].c;
            sim->beam_heads_size -= 1;
            continue;
        }

        if (sim->cells[head->r + 1][head->c] == CT_OPEN) {
            //printf("   beam[%d] step open\n", i);
            // move down 1, draw to map
            sim->beam_heads[i].r += 1;
            sim->cells[head->r][head->c] = CT_BEAM;
        } else if (sim->cells[head->r + 1][head->c] == CT_SPLITTER) {
            sim->splits_count += 1;
            coord_t splitter = {
                .r = head->r + 1,
                .c = head->c
            };
            //printf("   beam[%d] hit splitter!\n", i);
            // split left
            if (head->c > 0) {
                // make sure beam doesn't collide with another beam!
                sim->beam_heads[i].r = head->r + 1;
                sim->beam_heads[i].c = head->c - 1;
                if (sim->cells[head->r][head->c] == CT_BEAM) {
                    // todo: delete
                    sim->beam_heads[i].r = sim->beam_heads[sim->beam_heads_size - 1].r;
                    sim->beam_heads[i].c = sim->beam_heads[sim->beam_heads_size - 1].c;
                    sim->beam_heads_size -= 1;
                } else {
                    sim->cells[head->r][head->c] = CT_BEAM;
                }
            }

            // split right
            if (splitter.c < sim->cells_size) {
                // make sure beam doesn't collide with another beam!
                if (sim->cells[splitter.r][splitter.c + 1] == CT_BEAM) {
                    // do nothing
                } else {
                    sim->cells[splitter.r][splitter.c + 1] = CT_BEAM;
                    sim->beam_heads[sim->beam_heads_size].r = splitter.r;
                    sim->beam_heads[sim->beam_heads_size].c = splitter.c + 1;
                    sim->beam_heads_size += 1;
                }
            }
        } else if (sim->cells[head->r + 1][head->c] == CT_BEAM) {
            //printf("   [%d] hit previous beam, deleting\n", i);
            sim->beam_heads[i].r = sim->beam_heads[sim->beam_heads_size - 1].r;
            sim->beam_heads[i].c = sim->beam_heads[sim->beam_heads_size - 1].c;
            sim->beam_heads_size -= 1;
        }
    }    
}

void start_simulation(tachyon_simulation_t* sim) {
    //printf("starting sim:\n");
    sim->beam_heads[0].r = sim->start.r + 1;
    sim->beam_heads[0].c = sim->start.c;
    sim->beam_heads_size = 1;
    sim->cells[sim->beam_heads[0].r][sim->beam_heads[0].c] = CT_BEAM;
    //print_sim(sim);

    int sentinel = 0;
    while(true) {
        sentinel++;
        if (sentinel > 255) {
            break;
        }
        simulate_single_step(sim);
        //print_sim(sim);
        if (sim->beam_heads_size == 0) {
            break;
        }
    }
}

int part1(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }
    tachyon_simulation_t sim = {0};
    parse_sim(&sim, args.file);
    start_simulation(&sim);
    printf("p1: splits count: %d\n", sim.splits_count);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    part1(argc, argv);
    // skipping step2
}

