/**
 * Responsible for decoding the "mov" assembly instruction for the 8086. This instruction
 * can be mapped into 7 different opcode encodings. The order of these encodings will
 * follow the datasheet table 4-12.
 *
 * 1. I_MOVE
 * 2. I_MOVE_IMMEDIATE
 * 3. I_MOVE_IMMEDIATE_TO_REGISTER
 * 4. I_MOVE_TO_AX
 * 5. I_MOVE_AX
 * 6. I_MOVE_TO_SEGMENT_REGISTER
 * 7. I_MOVE_SEGMENT_REGISTER
 *
 */
#ifndef EMU_MOV_H
#define EMU_MOV_H

#include <stdint.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"

// MARK: 1. I_MOVE
/**
 * Decodes the first x86 mov machine code instruction described as "Register/memory
 * to/from register" into it's assembly code representation.
 * Ex: 10001001 11011001 -> "mov cx, bx"
 *
 * @param emulator Emulator holding the machine code to decode.
 * @param byte1 First byte in the machine code instruction.
 * @param out_buffer Where to write the assembly code instruction to.
 * @param index Index into `out_buffer` to write at.
 * @param out_buffer_size Size of the output buffer.
 */
emu_result_t decode_move(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size);

/**
 * Emulates the first x86 mov machine code instruction described as "Register/memory
 * to/from register", modifying registers and memory addresses accordingly.
 *
 * @param emulator Emulator holding the mov instruction to perform, registers
 *                  to update, and any other peripheral data.
 * @param byet1 First byte in the machine code instruction being executed.
 */
emu_result_t emu_move(emulator_t* emulator, uint8_t byte1);

// MARK: 2. I_MOVE_IMMEDIATE

emu_result_t decode_move_immediate(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size);


emu_result_t emu_move_immediate(emulator_t* emulator, uint8_t byte1);

// MARK: 3. I_MOVE_IMMEDIATE_TO_REGISTER

emu_result_t decode_move_immediate_to_register(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size);
emu_result_t emu_move_immediate_to_register(emulator_t* emulator, uint8_t byte1);
void write_move_immediate_to_register(
    wide_t wide,
    uint8_t reg,
    uint16_t immediate,
    char* buffer,
    int* index,
    int buffer_size);

// MARK: 4. I_MOVE_TO_AX
emu_result_t decode_move_to_ax(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size);
emu_result_t emu_move_to_ax(emulator_t* emulator, uint8_t byte1);
void write_move_to_ax(
    wide_t wide,
    uint16_t address,
    char* buffer,
    int* index,
    int buffer_size);

// MARK: 5. I_MOVE_AX
emu_result_t decode_move_ax(
    emulator_t* emulator,
    uint8_t byte1,
    char* out_buffer,
    int* index,
    size_t out_buffer_size);
emu_result_t emu_move_ax(emulator_t* emulator, uint8_t byte1);
void write_move_ax(
    wide_t wide,
    uint16_t address,
    char* buffer,
    int* index,
    int buffer_size);

#endif // EMU_MOV_H