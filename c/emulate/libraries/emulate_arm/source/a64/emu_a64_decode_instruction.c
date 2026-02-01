
#include <stdint.h>

#include "logger.h"

#include "a64/emu_a64_decode_instruction.h"
#include "a64/instructions_a64.h"

/**
 * Decodes the instruction group/class from an arm64 machine encoded instruction.
 * https://developer.arm.com/documentation/ddi0602/2025-03/Index-by-Encoding?lang=en
 *
 * The group can be decoded from the first 8 bits.
 * 31    30    29    28    27    26    25    24 ...
 * [op0]             [        op1       ]
 *
 * | op0      | op1     | Instruction Group |
 * | -------- | ------- | ----------------- |
 * | 0        | 0000    | RESERVED          |
 * | 1        | 0000    | SME               |
 * | x        | 0010    | SVE               |
 * | x        | 00x1    | UNALLOCATED       |
 * | x        | 100x    | Data Processing -- Immediate |
 * | x        | 101x    | Branches, Exception Generating, and System Instructions |
 * | x        | x101    | Data Processing -- Register |
 * | x        | x111    | Data Processing -- Scalar Floating-Point and Advanced SIMD |
 * | x        | x1x0    | Loads and Stores |
 */
instruction_group_a64_t emu_a64_decode_instruction_group(uint32_t instruction) {
    uint8_t op0_and_op1_mask =
        0b10011110;  // Masks out both op0 and op1, "0x000yyyy0" where op0 = x, op1 = yyyy
    uint8_t op0_and_op1 = (instruction >> 24) & op0_and_op1_mask;

    switch (op0_and_op1) {
        // Reserved
        // op0: 0, op1: 0000
        case 0b00000000: {
            return (IG_RESERVED);
        }
        // SME encodings
        // op0: 1, op1: 0000
        case 0b10000000: {
            return (IG_SME);
        }
        // SVE encodings
        // op0: x, op1: 0010
        case 0b00000100:
        case 0b10000100: {
            return (IG_SVE);
        }
        // UNALLOCATED
        // op0: x, op1: 00x1
        case 0b00000010:
        case 0b00000110:
        case 0b10000010:
        case 0b10000110: {
            return (IG_UNALLOCATED);
        }
        // Data Processing -- Immediate
        // op0: x, op1: 100x
        case 0b00010000:
        case 0b00010010:
        case 0b10010000:
        case 0b10010010: {
            return (IG_DATA_PROCESSING_IMMEDIATE);
        }
        // Branches, Exception Generating and System Instructions
        // op0: x, op: 101x
        case 0b00010100:
        case 0b00010110:
        case 0b10010100:
        case 0b10010110: {
            return (IG_BRANCHES_EXCEPTIONS_SYSTEM);
        }
        // Data Processing -- Register
        // op0: x, op: x101
        case 0b00001010:
        case 0b00011010:
        case 0b10001010:
        case 0b10011010: {
            return (IG_DATA_PROCESSING_REGISTER);
        }
        // Data Processing -- Scalar Floating-Point and Advanced SIMD
        // op0: x, op: x111
        case 0b00001110:
        case 0b00011110:
        case 0b10001110:
        case 0b10011110: {
            return (IG_DATA_PROCESSING_SCALAR_FP_AND_SIMD);
        }
        // Loads and Stores
        // op0: x, op: x1x0
        case 0b00001000:
        case 0b00001100:
        case 0b00011000:
        case 0b00011100:
        case 0b10001000:
        case 0b10001100:
        case 0b10011000:
        case 0b10011100: {
            return (IG_LOADS_AND_STORES);
        }
    }

    LOG(LOG_ERROR, "instruction group not supported: 0x%04x\n", instruction);
    return (IG_INVALID);
}

static instruction_tag_a64_t emu_a65_decode_instruction_tag_add_subtract_immediate(
    uint32_t instruction
) {
    // https://developer.arm.com/documentation/ddi0602/2025-03/Index-by-Encoding/Data-Processing----Immediate?lang=en#addsub_imm
    // uint8_t sf = (instruction >> 31);
    // uint8_t op = (instruction >> 30) & 0b1;
    // uint8_t s = (instruction >> 29) & 0b1;
    // all 3 bits relevant to determining the instruction are all contiguous, so we
    // can just mask them all together.
    uint8_t sf_op_s = (instruction >> 29);

    // TODO: create different tags for 32 bit and 64 bit modes? if not, can cut off "sf" and
    // cut this down to 4 cases.
    switch (sf_op_s) {
        case 0b000: {
            return (I_A64_ADD_IMMEDIATE);
        }
        case 0b001: {
            return (I_A64_ADDS_IMMEDIATE);
        }
        case 0b010: {
            return (I_A64_SUB_IMMEDIATE);
        }
        case 0b011: {
            return (I_A64_SUBS_IMMEDIATE);
        }
        case 0b100: {
            return (I_A64_ADD_IMMEDIATE);
        }
        case 0b101: {
            return (I_A64_ADDS_IMMEDIATE);
        }
        case 0b110: {
            return (I_A64_SUB_IMMEDIATE);
        }
        case 0b111: {
            return (I_A64_SUBS_IMMEDIATE);
        }
    }
    return (I_INVALID);
}

static instruction_tag_a64_t emu_a64_decode_instruction_tag_process_immediate(
    uint32_t instruction
) {
    // https://developer.arm.com/documentation/ddi0602/2025-03/Index-by-Encoding/Data-Processing----Immediate
    uint8_t op0 = (instruction >> 29) & 0b011;
    uint8_t op1 = (instruction >> 22) & 0b1111;
    // create a key of op0 and op1 to allow for a jump table.
    // key = 0bxx00yyyy, where op0 = xx, op1 = yyyy
    // most cases don't look at op0 at all... need to think about this structuring more
    LOGD("op0: %x, op1: %x", op0, op1);

    switch (op1) {
        // Add/subtract (immediate)
        // op0: xx, op1: 010x
        case 0b0100:
        case 0b0101: {
            return (emu_a65_decode_instruction_tag_add_subtract_immediate(instruction));
        }
    }

    uint8_t op0_and_op1 = (op0 << 6) | op1;
    LOGD("op0: %x, op1: %x, op1_and_op2: %x", op0, op1, op0_and_op1);

    switch (op0_and_op1) {
        // Data-processing (1 source immediate)
        // op0 = 11, op1 = 111x
        case 0b11001110:
        case 0b11001111: {
            // TODO: Data-processing (1 source immediate)
            return (I_INVALID);
        }
    }

    return (I_INVALID);
}

instruction_tag_a64_t emu_a64_decode_instruction_tag(uint32_t instruction) {
    printf("emu_a64_decode_instruction_tag: data: 0x%04x\n", instruction);

    instruction_group_a64_t group = emu_a64_decode_instruction_group(instruction);
    switch (group) {
        // IG_RESERVED
        // ...
        case IG_DATA_PROCESSING_IMMEDIATE: {
            return emu_a64_decode_instruction_tag_process_immediate(instruction);
        }
        // ...
        default: {
            printf(
                "emu_a64_decode_instruction_tag: instruction tag not supported yet: 0x%04x\n",
                instruction
            );
            return (I_INVALID);
        }
    }
}
