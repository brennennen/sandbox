/**
 * 8086 instruction stream decoder. Reads in a set of machine instructions, executes them,
 * and then prints the emulated register and flag values.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"
#include "shared/include/arch.h"

#include "libraries/emulate_intel/include/8086/emulate_8086.h"
#include "libraries/emulate_riscv/include/rv64/rv64_emulate.h"

#include "emulate.h"
#include "8086/emu_8086_registers.h"

void print_help() {
    printf("Disassembles a program to standard out.\n");
    printf("usage: decode {arch} {my_assembled_file}\n");
    printf("architectures: ");
    for (int i = 0; i < ARCH_COUNT; i++) {
        printf("%s, ", arch_names[i]);
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    #ifdef __STDC_VERSION__
        printf("__STDC_VERSION__ = %ld\n", __STDC_VERSION__);
    #else
        printf("__STDC_VERSION__ is not defined\n");
    #endif

    if (argc < 3) {
        printf("Missing arguments!\n");
        print_help();
        return 1;
    } else if (argc > 3) {
        printf("Too many arguments!\n");
        print_help();
        return 1;
    }

    char* arch_name = argv[1];
    arch_t arch = arch_get_by_name(arch_name);
    char* input_path = argv[2];
    printf("Starting emulation on arch: '%s' (%d), input:'%s'\n",
        arch_name, arch, input_path);

    result_t result = FAILURE;
    switch(arch) {
        case ARCH_I8086: {
            emulator_8086_t emulator_8086;
            emu_8086_init(&emulator_8086);
            result = emu_8086_emulate_file(&emulator_8086, input_path);
            printf("instruction count: %d\n", emulator_8086.instructions_count);
            emu_8086_print_registers_condensed(&emulator_8086);
            emu_8086_print_flags_condensed(emulator_8086.registers.flags);
            break;
        }
        case ARCH_RV64: {
            emulator_rv64_t emulator_rv64;
            emu_rv64_init(&emulator_rv64);
            result = emu_rv64_emulate_file(&emulator_rv64, input_path);
            printf("instruction count: %d\n", emulator_rv64.instructions_count);
            emu_rv64_print_registers_condensed(&emulator_rv64);
            break;
        }
        default: {
            printf("Arch not supported!\n");
            return 1;
        }
    }
    printf("Emulation result: %s\n", result_strings[result]);
}
