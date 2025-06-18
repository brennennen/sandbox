#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode_instruction.h"

// MARK: Instructions
// Data Processing -- Immediate
Test(emu_rv64_decode_tag_tests, I_ADD_IMMEDIATE) {
    // ...
    // "I" Type
    cr_assert(I_RV64I_ADDI == rv64_decode_instruction_tag(0x00550593)); // addi a1, a0, 5
    cr_assert(I_RV64I_ADD == rv64_decode_instruction_tag(0x007302b3)); // add t0, t1, t2
    cr_assert(I_RV64I_SLTI == rv64_decode_instruction_tag(0x0003ae13)); // slti t3, t2, 0
    cr_assert(I_RV64I_SLTIU == rv64_decode_instruction_tag(0x0ff33293)); // sltiu t0, t1, 255
    cr_assert(I_RV64I_XORI == rv64_decode_instruction_tag(0x01034293)); // xori t0, t1, 16
    cr_assert(I_RV64I_ORI == rv64_decode_instruction_tag(0x020fef13)); // ori t5, t6, 32
    cr_assert(I_RV64I_ANDI == rv64_decode_instruction_tag(0x040efe13)); // andi t3, t4, 64
    // "R" Type
    cr_assert(I_RV64I_ADD == rv64_decode_instruction_tag(0x007302b3)); // add t0, t1, t2
    cr_assert(I_RV64I_SUB == rv64_decode_instruction_tag(0x41ff0eb3)); // sub t4, t5, t6
    cr_assert(I_RV64I_SLL == rv64_decode_instruction_tag(0x00629fb3)); // sll t6, t0, t1
    cr_assert(I_RV64I_SLT == rv64_decode_instruction_tag(0x007322b3)); // slt t0, t1, t2
    cr_assert(I_RV64I_SLTU == rv64_decode_instruction_tag(0x00c5b533)); // sltu a0, a1, a2
    cr_assert(I_RV64I_XOR == rv64_decode_instruction_tag(0x0093c533)); // xor a0, t2, s1




    // ...
}
