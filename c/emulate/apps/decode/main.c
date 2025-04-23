/**
 * 8086 instruction stream decoder. Reads in a set of machine instructions and
 * outputs assembly insutrctions.
 *
 * Example usage: `decode ./my_asm`
 *
 * Use NASM or a similar assembler to create assembly files. ex: `nasm my_file.asm`
 * my_file.asm:
```asm
bits 16
mov cx, 5
my_label:
add cx, 5
sub cx, 10
jne my_label
```
 * NASM or any other assembler will output bytecode, ex: 0b00000001 0b11011001 ...
 * Running this decode program takes this bytecode as input and outputs back assembly.
 * Don't always expect a 1 to 1 mapping, JNE and JNZ are the exact same opcode for example, so
the decoder needs to just pick 1. You may pass a "JNE" instruction to NASM and get a "JNE" back
from an opcode decoder for example. Also label names get lost when creating opcodes, rather than
inputting made up label names, I chose to just use the relative offset syntax for decode output.
 * Example output:
```
mov cx, 5
add cx, 5
sub cx, 10
jne $-6
```
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"

#include "libraries/emulate_intel/include/emulate.h"

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
    emulator_t emulator;
    emu_init(&emulator);
    char out_buffer[4096] = { 0x00 };
    result_t result = emu_decode_file(&emulator, argv[1], out_buffer, sizeof(out_buffer));
    printf("Decode result: %s, instructions: %d\n", result_strings[result], emulator.instructions_count);
    printf("Assembly:\n%s\n", out_buffer);
}
