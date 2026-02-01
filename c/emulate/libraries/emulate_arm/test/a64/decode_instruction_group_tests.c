#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "a64/emu_a64_decode_instruction.h"
#include "a64/instructions_a64.h"

// MARK: Instruction Groups
Test(emu_a64_decode_group_tests, IG_DATA_PROCESSING) {
    cr_assert(
        IG_DATA_PROCESSING_IMMEDIATE == emu_a64_decode_instruction_group(0x91001400)
    );  // add x0, x0, #5
}
