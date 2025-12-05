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
    CT_OPEN,
    CT_PAPER
} cell_type_t;

typedef struct {
    cell_type_t cell_type;
    bool can_access;
} cell_t;

typedef struct {
    int size;
    cell_t cells[256][256];
} grid_t;

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

void parse_grid(grid_t* grid, FILE* file) {
    char buffer[256] = {0};
    bool first_row = true;
    int r = 0;
    while(fgets(buffer, sizeof(buffer), file) != NULL) {
        int grid_row_size = strlen(buffer) - 1;
        buffer[grid_row_size] = '\0';
        if (first_row) {
            grid->size = grid_row_size;
            //printf("size: %d\n", grid_row_size);
            first_row = false;
        }
        //printf("line: %s\n", buffer);
        
        for(int c = 0; c < grid_row_size; c++) {
            if (buffer[c] == '@') {
                grid->cells[r][c].cell_type = CT_PAPER;
            } else if (buffer[c] == '.') {
                grid->cells[r][c].cell_type = CT_OPEN;
            }
        }
        r++;
    }
}

void print_grid(grid_t* grid) {
    printf("grid:\n");
    for(int r = 0; r < grid->size; r++) {
        for (int c = 0; c < grid->size; c++) {
            char cell_char = '.';
            if (grid->cells[r][c].can_access) {
                cell_char = 'x';
            } else if (grid->cells[r][c].cell_type == CT_PAPER) {
                cell_char = '@';
            }
            printf("%c", cell_char);
        }
        printf("\n");
    }
}

bool can_access_paper(grid_t* grid, int r, int c) {
    if (grid->cells[r][c].cell_type != CT_PAPER) {
        return(false);
    }

    int occupied_neighbors = 0;
    // check each 8 neighbors, if at boundery, count as clear
    // left
    if (c > 0) {
        cell_t* cell = &grid->cells[r][c - 1];
        if (cell->cell_type == CT_PAPER) {
            occupied_neighbors += 1;
        }
    }

    // left up
    if (r > 0 && c > 0) {
        cell_t* cell = &grid->cells[r - 1][c - 1];
        if (cell->cell_type == CT_PAPER) {
            occupied_neighbors += 1;
        }
    }
    // up
    if (r > 0) {
        cell_t* cell = &grid->cells[r - 1][c];
        if (cell->cell_type == CT_PAPER) {
            occupied_neighbors += 1;
        }
    }
    // up right
    if (r > 0 && c < grid->size) {
        cell_t* cell = &grid->cells[r - 1][c + 1];
        if (cell->cell_type == CT_PAPER) {
            occupied_neighbors += 1;
        }
    }
    // right
    if (c < grid->size) {
        cell_t* cell = &grid->cells[r][c + 1];
        if (cell->cell_type == CT_PAPER) {
            occupied_neighbors += 1;
        }
    }
    // down right
    if (r < grid->size && c < grid->size) {
        cell_t* cell = &grid->cells[r + 1][c + 1];
        if (cell->cell_type == CT_PAPER) {
            occupied_neighbors += 1;
        }
    }
    // down
    if (r < grid->size) {
        cell_t* cell = &grid->cells[r + 1][c];
        if (cell->cell_type == CT_PAPER) {
            occupied_neighbors += 1;
        }
    }
    // down left
    if (r < grid->size && c > 0) {
        cell_t* cell = &grid->cells[r + 1][c - 1];
        if (cell->cell_type == CT_PAPER) {
            occupied_neighbors += 1;
        }
    }

    if (occupied_neighbors < 4) {
        grid->cells[r][c].can_access = true;
        return(true);
    } else {
        return(false);
    }
}

int part1(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }
    grid_t grid = {0};
    parse_grid(&grid, args.file);
    //print_grid(&grid);
    int fewer_than_4 = 0;
    for(int r = 0; r < grid.size; r++) {
        for(int c = 0; c < grid.size; c++) {
            if (can_access_paper(&grid, r, c)) {
                fewer_than_4 += 1;
            }
        }
    }
    //print_grid(&grid);
    printf("p1 fewer than 4: %d\n", fewer_than_4);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

int get_total_accessible_papers(grid_t* grid) {
    int total_accessible = 0;
    for(int r = 0; r < grid->size; r++) {
        for(int c = 0; c < grid->size; c++) {
            if (can_access_paper(grid, r, c)) {
                total_accessible += 1;
            }
        }
    }
    return(total_accessible);
}

int remove_accessible_papers(grid_t* grid) {
    int removed_this_move = 0;
    for(int r = 0; r < grid->size; r++) {
        for(int c = 0; c < grid->size; c++) {
            if (grid->cells[r][c].can_access) {
                grid->cells[r][c].cell_type = CT_OPEN;
                grid->cells[r][c].can_access = false;
                removed_this_move += 1;
            }
        }
    }
    //printf("removed: %d\n", removed_this_move);
    return(removed_this_move);
}

int part2(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }
    grid_t grid = {0};
    parse_grid(&grid, args.file);
    //print_grid(&grid);
    int total_removed = 0;
    while(get_total_accessible_papers(&grid) != 0) {
        total_removed += remove_accessible_papers(&grid);
        //print_grid(&grid);
    }
    
    //print_grid(&grid);
    printf("p2 total_removed: %d\n", total_removed);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    part1(argc, argv);
    part2(argc, argv);
}
