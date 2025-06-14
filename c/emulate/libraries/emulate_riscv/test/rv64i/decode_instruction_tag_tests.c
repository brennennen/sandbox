#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64i/rv64i_instructions.h"
#include "rv64i/rv64i_decode_instruction.h"

// MARK: Instructions
// Data Processing -- Immediate
Test(emu_rv64i_decode_tag_tests, I_ADD_IMMEDIATE) {
    cr_assert(I_RV64I_ADDI == rv64i_decode_instruction_tag(0x00550593)); // addi a1, a0, 5
    cr_assert(I_RV64I_ADD == rv64i_decode_instruction_tag(0x007302b3)); // add t0, t1, t2
    cr_assert(I_RV64I_SLTI == rv64i_decode_instruction_tag(0x0003ae13)); // slti t3, t2, 0
}
