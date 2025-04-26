/**
 * x64
 *
 * i386
 * (Intel 80386 Programmer's Reference Manual, 3.1.2, page 46).
 *
 * 8086
 * `PUSH source`
 * PUSH decrements SP (the stack pointer) by two and then transfers a word from the source
 * operand to the top of stack now pointed to by SP. PUSH often is used to place parameters
 * on the stack before calling a procedure; more generally, it is the basic means of storing
 * temporary data on the stack (8086 Family Users Manual, page 2-31, pdf page ~46).
 *
 * This instruction can be mapped into 3 different opcode encodings. The order of these
 * encodings will follow the datasheet table 4-12 (8086 Family Users Manual, page 4-22,
 * pdf page ~164).
 *
 * 1. I_PUSH
 * 2. I_PUSH_REGISTER
 * 3. I_PUSH_SEGMENT_REGISTER
 *
 */
#ifndef EMU_PUSH_H
#define EMU_PUSH_H

#include <stdint.h>

#include "8086/instruction_tags_8086.h"

#include "libraries/emulate_intel/include/emulate.h"

// MARK: 1. I_PUSH

// MARK: 2. I_PUSH_REGISTER
/**
 * Decodes the seconds x86 push machine code instruction described as "Register"
 * into it's assembly code representation.
 * Ex: 01010001 -> "push cx"
 *
 * @param emulator Emulator holding the machine code to decode.
 * @param byte1 First byte in the machine code instruction.
 * @param out_buffer Where to write the assembly code instruction to.
 * @param index Index into `out_buffer` to write at.
 * @param out_buffer_size Size of the output buffer.
 */
emu_result_t decode_push_register(
    emulator_8086_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size);

/**
 * Emulates the second x86 push machine code instruction described as "Register",
 * modifying registers and memory addresses accordingly.
 *
 * @param emulator Emulator holding the push instruction to perform, registers
 *                  to update, and any other peripheral data.
 * @param byet1 First byte in the machine code instruction being executed.
 */
emu_result_t emu_push_register(emulator_8086_t* emulator, uint8_t byte1);

// MARK: 3. I_PUSH_SEGMENT_REGISTER


#endif // EMU_PUSH_H