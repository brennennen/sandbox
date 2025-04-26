/**
 * 8086 instruction stream decoder. Reads in a set of machine instructions, executes them,
 * and then prints the emulated register and flag values.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"

#include "emulate.h"
#include "8086/emu_8086_registers.h"

void print_help() {
    printf("Decodes an assembled x86 program to standard out.\n");
    printf("usage: emulate {my_assembled_file}\n");
}

int main(int argc, char* argv[]) {
    #ifdef __STDC_VERSION__
        printf("__STDC_VERSION__ = %ld\n", __STDC_VERSION__);
    #else
        printf("__STDC_VERSION__ is not defined\n");
    #endif

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
    emu_init(&emulator, ARCH_8086);
    result_t result = emu_emulate_file(&emulator, argv[1]);
    printf("Emulate result: %s, instructions: %d\n", result_strings[result], emulator.emulator_8086.instructions_count);

    print_registers_condensed(&emulator.emulator_8086);
    print_flags_condensed(emulator.emulator_8086.registers.flags);
}
