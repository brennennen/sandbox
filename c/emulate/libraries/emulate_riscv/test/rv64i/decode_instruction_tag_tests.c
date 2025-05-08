#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64i/instructions_rv64i.h"
#include "rv64i/emu_rv64i_decode_instruction.h"

// MARK: Instructions
// Data Processing -- Immediate
Test(emu_rv64i_decode_tag_tests, I_ADD_IMMEDIATE) {
    cr_assert(I_RV64I_ADD_IMMEDIATE == emu_rv64i_decode_instruction_tag(0x00550593)); // addi a1, a0, 5
}
