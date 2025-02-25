#include <stdio.h>
#include <string.h>

#include <criterion/criterion.h>

#include "shared/include/instructions.h"

#include "libraries/emulate8086/include/emulate8086.h"
#include "libraries/emulate8086/include/decode_tag.h"

// MARK: MOV
Test(dcd_decode_tag_tests, MOVE) {
    cr_assert(I_MOVE == dcd_decode_tag(0b10001000, 0));
    cr_assert(I_MOVE_IMMEDIATE == dcd_decode_tag(0b11000110, 0));
    cr_assert(I_MOVE_IMMEDIATE_TO_REGISTER == dcd_decode_tag(0b10110000, 0));
    cr_assert(I_MOVE_TO_AX == dcd_decode_tag(0b10100000, 0));
    cr_assert(I_MOVE_AX == dcd_decode_tag(0b10100010, 0));
    cr_assert(I_MOVE_TO_SEGMENT_REGISTER == dcd_decode_tag(0b10001110, 0));
    cr_assert(I_MOVE_SEGMENT_REGISTER == dcd_decode_tag(0b10001100, 0));
}

// MARK: PUSH
Test(dcd_decode_tag_tests, PUSH) {
    cr_assert(I_PUSH == dcd_decode_tag(0b11111111, 0b00110000));
    cr_assert(I_PUSH_REGISTER == dcd_decode_tag(0b01010001, 0));
    //cr_assert(I_PUSH_SEGMENT_REGISTER == dcd_decode_tag(0b00000110, 0));
}

// MARK: POP
Test(dcd_decode_tag_tests, POP) {
    cr_assert(I_POP == dcd_decode_tag(0b10001111, 0b00000000));
    cr_assert(I_POP_REGISTER == dcd_decode_tag(0b01011001, 0));
    //cr_assert(I_POP_SEGMENT_REGISTER == dcd_decode_tag(0b00000111, 0));
}

// MARK: XCHG
Test(dcd_decode_tag_tests, XCHG) {
    cr_assert(I_EXCHANGE == dcd_decode_tag(0b10000110, 0));
    cr_assert(I_EXCHANGE_WITH_AX == dcd_decode_tag(0b10010000, 0));
}

// MARK: IN
Test(dcd_decode_tag_tests, IN) {
    cr_assert(I_INPUT_FROM_FIXED_PORT == dcd_decode_tag(0b11100100, 0));
    cr_assert(I_INPUT_FROM_VARIABLE_PORT == dcd_decode_tag(0b11101100, 0));
}

// MARK: OUT
Test(dcd_decode_tag_tests, OUT) {
    cr_assert(I_OUTPUT_TO_FIXED_PORT == dcd_decode_tag(0b11100110, 0));
    cr_assert(I_OUTPUT_TO_VARIABLE_PORT == dcd_decode_tag(0b11101110, 0));
}

// MARK: ARITHMETIC

// MARK: ADD
Test(dcd_decode_tag_tests, ADD) {
    cr_assert(I_ADD == dcd_decode_tag(0b00000000, 0));
    cr_assert(I_ADD_IMMEDIATE == dcd_decode_tag(0b10000000, 0b00000000));
    cr_assert(I_ADD_IMMEDIATE_TO_AX == dcd_decode_tag(0b00000100, 0));
}

// MARK: ADC
Test(dcd_decode_tag_tests, ADC) {
    cr_assert(I_ADC == dcd_decode_tag(0b00010000, 0));
    cr_assert(I_ADC_IMMEDIATE == dcd_decode_tag(0b10000000, 0b00010000));
    cr_assert(I_ADC_IMMEDIATE_TO_AX == dcd_decode_tag(0b00010100, 0));
}

// MARK: INC
Test(dcd_decode_tag_tests, INC) {
    cr_assert(I_INC == dcd_decode_tag(0b11111110, 0));
    cr_assert(I_INC_REGISTER == dcd_decode_tag(0b01000000, 0));
}

// MARK: AAA
Test(dcd_decode_tag_tests, AAA) {
    cr_assert(I_AAA_ASCII_ADJUST_FOR_ADD == dcd_decode_tag(0b00110111, 0));
}

// MARK: DAA
Test(dcd_decode_tag_tests, DAA) {
    cr_assert(I_DAA_DECIMAL_ADJUST_FOR_ADD == dcd_decode_tag(0b00100111, 0));
}

// MARK: SUB
Test(dcd_decode_tag_tests, SUB) {
    cr_assert(I_SUB == dcd_decode_tag(0b00101000, 0));
    cr_assert(I_SUB_IMMEDIATE == dcd_decode_tag(0b10000000, 0b00101000));
    cr_assert(I_SUB_IMMEDIATE_FROM_AX == dcd_decode_tag(0b00101100, 0));
}

// MARK: SBB
Test(dcd_decode_tag_tests, SBB) {
    cr_assert(I_SBB == dcd_decode_tag(0b00011000, 0));
    cr_assert(I_SBB_IMMEDIATE == dcd_decode_tag(0b10000000, 0b00011000));
    cr_assert(I_SBB_IMMEDIATE_FROM_AX == dcd_decode_tag(0b00011100, 0));
}

// MARK: DEC
Test(dcd_decode_tag_tests, DEC) {
    cr_assert(I_DEC_REGISTER_OR_MEMORY == dcd_decode_tag(0b11111110, 0b00001000));
    cr_assert(I_DEC_REGISTER == dcd_decode_tag(0b01001000, 0));
}

// MARK: NEG
Test(dcd_decode_tag_tests, NEG) {
    cr_assert(I_NEGATE_CHANGE_SIGN == dcd_decode_tag(0b11110110, 0b00011000));
}

// MARK: CMP
Test(dcd_decode_tag_tests, CMP) {
    cr_assert(I_COMPARE == dcd_decode_tag(0b00111000, 0));
    cr_assert(I_COMPARE_IMMEDIATE == dcd_decode_tag(0b10000000, 0b00111000));
    cr_assert(I_COMPARE_IMMEDIATE_WITH_AX == dcd_decode_tag(0b00111100, 0));
}

// LOGIC

// STRING MANIPULATION

// CONTROL TRANSFER

// MARK: PROCESSOR CONTROL
// MARK: CLC
Test(dcd_decode_tag_tests, CLC) {
    cr_assert(I_CLEAR_CARRY == dcd_decode_tag(0b11111000, 0));
}

// MARK: CMC
Test(dcd_decode_tag_tests, CMC) {
    cr_assert(I_COMPLEMENT_CARRY == dcd_decode_tag(0b11110101, 0));
}

// MARK: STC
Test(dcd_decode_tag_tests, STC) {
    cr_assert(I_SET_CARRY == dcd_decode_tag(0b11111001, 0));
}

// MARK: CLD
Test(dcd_decode_tag_tests, CLD) {
    cr_assert(I_CLEAR_DIRECTION == dcd_decode_tag(0b11111100, 0));
}

// MARK: STD
Test(dcd_decode_tag_tests, STD) {
    cr_assert(I_SET_DIRECTION == dcd_decode_tag(0b11111101, 0));
}

// MARK: CLI
Test(dcd_decode_tag_tests, CLI) {
    cr_assert(I_CLEAR_INTERRUPT == dcd_decode_tag(0b11111010, 0));
}

// MARK: STI
Test(dcd_decode_tag_tests, STI) {
    cr_assert(I_SET_INTERRUPT == dcd_decode_tag(0b11111011, 0));
}

// MARK: HLT
Test(dcd_decode_tag_tests, HLT) {
    cr_assert(I_HALT == dcd_decode_tag(0b11110100, 0));
}

// MARK: WAIT
Test(dcd_decode_tag_tests, WAIT) {
    cr_assert(I_WAIT == dcd_decode_tag(0b10011011, 0));
}

// MARK: ESC
Test(dcd_decode_tag_tests, ESC) {
    cr_assert(I_ESCAPE == dcd_decode_tag(0b11011000, 0));
}

// MARK: LOCK
Test(dcd_decode_tag_tests, LOCK) {
    cr_assert(I_LOCK == dcd_decode_tag(0b11110000, 0));
}

// MARK: SEGMENT
Test(dcd_decode_tag_tests, SEGMENT) {
    cr_assert(I_SEGMENT == dcd_decode_tag(0b00100110, 0));
}
