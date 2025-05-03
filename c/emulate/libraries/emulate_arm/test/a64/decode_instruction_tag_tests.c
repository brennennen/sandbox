#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "a64/instructions_a64.h"
#include "a64/emu_a64_decode_instruction.h"

// MARK: Instructions
// Data Processing -- Immediate
Test(emu_a64_decode_tag_tests, I_ADD_IMMEDIATE) {
    cr_assert(I_ADD_IMMEDIATE == emu_a64_decode_instruction_tag(0x91001400)); // add x0, x0, #5
}

Test(emu_a64_decode_tag_tests, I_ADDS_IMMEDIATE) {
    cr_assert(I_ADDS_IMMEDIATE == emu_a64_decode_instruction_tag(0xb1024000)); // adds x0, x0, #0x90
}

Test(emu_a64_decode_tag_tests, I_SUB_IMMEDIATE) {
    cr_assert(I_SUB_IMMEDIATE == emu_a64_decode_instruction_tag(0xd1024021)); // sub x1, x1, #0x90
}

Test(emu_a64_decode_tag_tests, I_SUBS_IMMEDIATE) {
    cr_assert(I_SUBS_IMMEDIATE == emu_a64_decode_instruction_tag(0xf1014042)); // subs x2, x2, #0x50
}

