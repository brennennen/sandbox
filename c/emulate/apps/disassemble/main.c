/**
 * Instruction stream disassembler. Reads in a set of machine instructions and
 * outputs assembly instructions.
 *
 * Example usage: `disassemble 8086 ./my_asm`
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
the disassembler needs to just pick 1. You may pass a "JNE" instruction to NASM and get a "JNE" back
from an opcode disassembler for example. Also label names get lost when creating opcodes, rather than
inputting made up label names, I chose to just use the relative offset syntax for decode output.
 * Example output:
```
mov cx, 5
add cx, 5
sub cx, 10
jne $-6
```
 *
 *
 * For ARM and RISCV, use GNU tools:
 * Assemble some arbitrary assembly:
 * * riscv64-unknown-elf-as -o ./slti.o ./slti.asm
 * Extract just the text section:
 * * riscv64-unknown-elf-objcopy -O binary --only-section=.text slti.o slti.text
 * TODO: endianess is backwards from representation in objdump, figure out what spec says and flip this or flip processing.
 * (flip: riscv64-unknown-elf-objcopy -I binary -O binary --reverse-bytes=4 slti.text slti_reverse.text)
 *
 * Use this tool: `disassemble rv64i ./slti.text` to convert the machine code back into assembly.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/result.h"
#include "shared/include/arch.h"

#include "libraries/emulate_intel/include/8086/emulate_8086.h"
#include "libraries/emulate_riscv/include/rv64/rv64_emulate.h"

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
    printf("Starting disassemble on arch: '%s' (%d), input:'%s'\n",
        arch_name, arch, input_path);

    // TODO: use a stream instead of static buffer
    char out_buffer[4096] = { 0x00 };
    switch(arch) {
        case ARCH_I8086: {
            emulator_8086_t emulator_8086;
            emu_8086_init(&emulator_8086);
            result_t result = emu_8086_disassemble_file(&emulator_8086, input_path, out_buffer, sizeof(out_buffer));
            printf("Disassemble result: %s, instructions: %d\n", result_strings[result], emulator_8086.instructions_count);
            break;
        }
        case ARCH_RV64I: {
            emulator_rv64_t emulator_rv64;
            emu_rv64_init(&emulator_rv64);
            result_t result = emu_rv64_disassemble_file(&emulator_rv64, input_path, out_buffer, sizeof(out_buffer));
            printf("Disassemble result: %s, instructions: %d\n", result_strings[result], emulator_rv64.instructions_count);
            break;
        }
        default: {
            printf("Arch not supported!\n");
            return 1;
        }
    }
    printf("Assembly:\n%s\n", out_buffer);
}
