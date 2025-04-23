/**
 * `POP destination`
 * POP transfers the word at the current top of stack (pointed to by SP) to the
 * destination operand, and then increments SP by two to point to the new top of stack.
 * POP can be used to move temporary variables from the stack to registers or memory
 * (8086 Family Users Manual, page 2-31, pdf page ~46).
 *
 * This instruction can be mapped into 3 different opcode encodings. The order of these
 * encodings will follow the datasheet table 4-12 (8086 Family Users Manual, page 4-22,
 * pdf page ~164).
 *
 * 1. I_POP
 * 2. I_POP_REGISTER
 * 3. I_POP_SEGMENT_REGISTER
 *
 */
#ifndef EMU_POP_H
#define EMU_POP_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/emulate_intel/include/emulate.h"

// MARK: 1. I_POP

// MARK: 2. I_POP_REGISTER
/**
 * Decodes the second x86 pop machine code instruction described as "Register"
 * into it's assembly code representation.
 * Ex: 01011001 -> "pop cx"
 *
 * @param emulator Emulator holding the machine code to decode.
 * @param byte1 First byte in the machine code instruction.
 * @param out_buffer Where to write the assembly code instruction to.
 * @param index Index into `out_buffer` to write at.
 * @param out_buffer_size Size of the output buffer.
 */
emu_result_t decode_pop_register(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size);

/**
 * Emulates the second x86 pop machine code instruction described as "Register",
 * modifying registers and memory addresses accordingly.
 *
 * @param emulator Emulator holding the pop instruction to perform, registers
 *                  to update, and any other peripheral data.
 * @param byet1 First byte in the machine code instruction being executed.
 */
emu_result_t emu_pop_register(emulator_t* emulator, uint8_t byte1);

// MARK: 3. I_POP_SEGMENT_REGISTER


#endif // EMU_POP_H