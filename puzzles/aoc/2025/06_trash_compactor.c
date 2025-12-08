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
    OP_NONE,
    OP_PLUS,
    OP_MULTIPLY
} operation_t;

typedef struct {
    int operands1[1024];
    int operands1_size;
    int operands2[1024];
    int operands2_size;
    int operands3[1024];
    int operands3_size;
    int operands4[1024];
    int operands4_size;
    operation_t operations[1024];
    int operations_size;
} trash_compactor_t;

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

bool char_is_operation(char c) {
    return '0' < c;
}

operation_t char_to_operation(char c) {
    switch(c) {
        case '+':
            return OP_PLUS;
        case '*':
            return OP_MULTIPLY;
        default:
            return OP_NONE;
    }
}

char operation_to_char(operation_t operation) {
    switch(operation) {
        case OP_PLUS:
            return '+';
        case OP_MULTIPLY:
            return '*';
        default:
            return '?';
    }
}

typedef enum {
    PM_OPERANDS1,
    PM_OPERANDS2,
    PM_OPERANDS3,
    PM_OPERANDS4,
    PM_OPERATIONS,
    PM_DONE
} parse_mode_t;


void parse_trash_compactor(FILE* file, trash_compactor_t* trash_compactor) {
    parse_mode_t parse_mode = PM_OPERANDS1;
    char buffer[64];
    int buffer_index = 0;
    char c = fgetc(file);
    int operands1_index = 0;
    int operands2_index = 0;
    int operands3_index = 0;
    int operands4_index = 0;
    int operations_index = 0;
    while(c != EOF) {
        switch (parse_mode) {
            case PM_OPERANDS1: {
                if (c == ' ' || c == '\n') {
                    if (buffer_index != 0) {
                        buffer[buffer_index] = '\0';
                        trash_compactor->operands1[operands1_index] = atoi(buffer);
                        operands1_index += 1;
                        buffer_index = 0;
                    }
                    if (c == '\n') {
                        //printf("done parsing operands1\n");
                        trash_compactor->operands1_size = operands1_index;
                        parse_mode = PM_OPERANDS2;
                    }
                } else {
                    buffer[buffer_index] = c;
                    buffer_index += 1;
                }
                break;
            }
            case PM_OPERANDS2: {
                if (c == ' ' || c == '\n') {
                    if (buffer_index != 0) {
                        buffer[buffer_index] = '\0';
                        trash_compactor->operands2[operands2_index] = atoi(buffer);
                        operands2_index += 1;
                        buffer_index = 0;
                    }
                    if (c == '\n') {
                        //printf("done parsing operands2\n");
                        trash_compactor->operands2_size = operands2_index;
                        parse_mode = PM_OPERANDS3;
                    }
                } else {
                    buffer[buffer_index] = c;
                    buffer_index += 1;
                }
                break;
            }
            case PM_OPERANDS3: {
                if (c == ' ' || c == '\n') {
                    if (buffer_index != 0) {
                        buffer[buffer_index] = '\0';
                        trash_compactor->operands3[operands3_index] = atoi(buffer);
                        operands3_index += 1;
                        buffer_index = 0;
                    }
                    if (c == '\n') {
                        //printf("done parsing operands3\n");
                        trash_compactor->operands3_size = operands3_index;
                        parse_mode = PM_OPERANDS4;
                    }
                } else {
                    buffer[buffer_index] = c;
                    buffer_index += 1;
                }
                break;
            }
            case PM_OPERANDS4: {
                if (c == ' ' || c == '\n') {
                    if (buffer_index != 0) {
                        buffer[buffer_index] = '\0';
                        trash_compactor->operands4[operands4_index] = atoi(buffer);
                        operands4_index += 1;
                        buffer_index = 0;
                    }
                    if (c == '\n') {
                        //printf("done parsing operands4\n");
                        trash_compactor->operands4_size = operands4_index;
                        parse_mode = PM_OPERATIONS;
                    }
                } else {
                    buffer[buffer_index] = c;
                    buffer_index += 1;
                }
                break;
            }
            case PM_OPERATIONS: {
                if (c == ' ' || c == '\n') {
                    if (buffer_index != 0) {
                        buffer[buffer_index] = '\0';
                        trash_compactor->operations[operations_index] = char_to_operation(buffer[buffer_index - 1]);
                        operations_index += 1;
                        buffer_index = 0;
                    }
                    if (c == '\n') {
                        //printf("done parsing operations\n");
                        trash_compactor->operations_size = operations_index;
                        parse_mode = PM_DONE;
                    }
                } else {
                    buffer[buffer_index] = c;
                    buffer_index += 1;
                }
                break;
            }
        }
        c = fgetc(file);
    }
}

void print_trash_compactor(trash_compactor_t* trash_compactor) {
    printf("operands1: ");
    for (int i = 0; i < trash_compactor->operands1_size; i++) {
        printf("%d, ", trash_compactor->operands1[i]);
    }
    printf("\n");
    printf("operands2: ");
    for (int i = 0; i < trash_compactor->operands2_size; i++) {
        printf("%d, ", trash_compactor->operands2[i]);
    }
    printf("\n");
    printf("operands3: ");
    for (int i = 0; i < trash_compactor->operands3_size; i++) {
        printf("%d, ", trash_compactor->operands3[i]);
    }
    printf("\n");
    printf("operands4: ");
    for (int i = 0; i < trash_compactor->operands4_size; i++) {
        printf("%d, ", trash_compactor->operands4[i]);
    }
    printf("\n");
    printf("operations: ");
    for (int i = 0; i < trash_compactor->operations_size; i++) {
        printf("%c, ", operation_to_char(trash_compactor->operations[i]));
    }
    printf("\n");
}

int part1(int argc, char* argv[]) {
    arguments_t args = { 0 };
    if (parse_arguments(argc, argv, &args) != RES_SUCCESS) {
        return(EXIT_FAILURE);
    }
    trash_compactor_t trash_compactor = {0};
    parse_trash_compactor(args.file, &trash_compactor);
    //print_trash_compactor(&trash_compactor);

    uint64_t grand_total = 0;
    for (int i = 0; i < trash_compactor.operations_size; i++) {
        uint64_t problem_result = 0;
        switch(trash_compactor.operations[i]) {
            case OP_PLUS: {

                problem_result = (uint64_t) trash_compactor.operands1[i]
                    + trash_compactor.operands2[i]
                    + trash_compactor.operands3[i]
                    + trash_compactor.operands4[i];
                //printf("[%d] problem_result: %llu\n", i, problem_result);
                break;
            }
            case OP_MULTIPLY: {
                problem_result = (uint64_t) trash_compactor.operands1[i]
                    * trash_compactor.operands2[i]
                    * trash_compactor.operands3[i]
                    * trash_compactor.operands4[i];
                //printf("[%d] problem_result: %llu\n", i, problem_result);
                break;
            }
        }
        grand_total += problem_result;
    }
    printf("p1 grand total: %llu\n", grand_total);
    fclose(args.file);
    return(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    part1(argc, argv);
    // skipping part 2 today
}
