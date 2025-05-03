

#ifndef EMU_A64_DECODE_INSTRUCTION
#define EMU_A64_DECODE_INSTRUCTION

#include "a64/instructions_a64.h"

/**
 * Decodes the instruction group/class from an arm64 machine encoded instruction.
 * https://developer.arm.com/documentation/ddi0602/2025-03/Index-by-Encoding?lang=en
 */
instruction_group_a64_t emu_a64_decode_instruction_group(uint32_t instruction);


instruction_tag_a64_t emu_a64_decode_instruction_tag(uint32_t instruction);

#endif // EMU_A64_DECODE_INSTRUCTION
