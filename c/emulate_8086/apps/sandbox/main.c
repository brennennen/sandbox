#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"

void print_help() {
    printf("Decodes an assembled x86 program to standard out.\n");
    printf("usage: decode {my_assembled_file}\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Missing filepath argument!\n");
        print_help();
        return 1;
    } else if (argc > 2) {
        printf("Too many arguments!\n");
        print_help();
        return 1;
    }

    char* input_path = argv[1];
    printf("Starting decode on: '%s'\n", input_path);
    decoder_t decoder;
    instruction_t instructions[4096] = {0};
    dcd_init(&decoder, instructions, 4096);
    result_t result = dcd_decode_file(&decoder, argv[1]);
    printf("Decode result: %s, instructions: %d\n", result_strings[result], decoder.instructions_count);
    char buffer[4096] = { 0x00 };
    dcd_write_all_assembly(decoder.instructions, decoder.instructions_count, buffer, 4096);
    printf("Assembly:\n%s\n", buffer);
    // for (int i = 0; i < decoder.instructions_count; i++) {
    //     char buffer[1024];
    //     dcd_write_assembly_instruction(&decoder.instructions[i], buffer, 1024);
    //     printf("%s\n", buffer);
    // }
}
