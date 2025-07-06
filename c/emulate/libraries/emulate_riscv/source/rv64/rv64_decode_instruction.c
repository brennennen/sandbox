


#include <stdint.h>

#include "logger.h"

#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode_instruction.h"


/**
 * A full listing of opcodes and the mnemonic they associate with can be found
 * in the specification under the "RV32/64G Instruction Set Listings" table ("RISC-V
 * Instruction Set Manual Volume 1", Chapter 35, page 608).
 *
 * TODO: Could "flatten" out this jump table a bit. Combine opcode, funct3, and
 * funct7 into a single 32 bit int and lookup on that.
 */
instruction_tag_rv64_t rv64_decode_instruction_tag(uint32_t instruction) {
    uint8_t opcode = instruction & 0b01111111;
    switch(opcode) {
        case 0b0110111: return(I_RV64I_LUI);
        case 0b0010111: return(I_RV64I_AUIPC);
        case 0b1101111: return(I_RV64I_JAL);
        case 0b1100111: return(I_RV64I_JALR);
        case 0b1100011: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case 0b000: return(I_RV64I_BEQ);
                case 0b001: return(I_RV64I_BNE);
                case 0b100: return(I_RV64I_BLT);
                case 0b101: return(I_RV64I_BGE);
                case 0b110: return(I_RV64I_BLTU);
                case 0b111: return(I_RV64I_BGEU);
                default: return(I_RV64I_INVALID);
            }
        }
        case 0b0000011: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case 0b000: return(I_RV64I_LB);
                case 0b001: return(I_RV64I_LH);
                case 0b010: return(I_RV64I_LW);
                case 0b100: return(I_RV64I_LBU);
                case 0b101: return(I_RV64I_LHU);
                case 0b110: return(I_RV64I_LWU); // RV64I Addition
                case 0b011: return(I_RV64I_LD); // RV64I Addition
                default: return(I_RV64I_INVALID);
            }
        }
        case 0b0100011: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case 0b000: return(I_RV64I_SB);
                case 0b001: return(I_RV64I_SH);
                case 0b010: return(I_RV64I_SW);
                case 0b100: return(I_RV64I_SD); // RV64I Addition
                default: return(I_RV64I_INVALID);
            }
        }
        case 0b0010011: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case 0b000: return(I_RV64I_ADDI);
                case 0b010: return(I_RV64I_SLTI);
                case 0b011: return(I_RV64I_SLTIU);
                case 0b100: return(I_RV64I_XORI);
                case 0b110: return(I_RV64I_ORI);
                case 0b111: return(I_RV64I_ANDI);
                case 0b001: return(I_RV64I_SLLI); // RV64I adds SHAMT=31 as a valid value
                case 0b101: {
                    uint8_t func7 = (instruction >> 25) & 0b1111111;
                    switch(func7) {
                        case 0b0000000: return(I_RV64I_SRLI); // Noted as some features being extra in RV64I, but not clear what exactly is.
                        case 0b0100000: return(I_RV64I_SRAI); // RV64I adds SHAMT=7 as a valid value
                        default: return(I_RV64I_INVALID);
                    }
                }
                // No default needed, all cases covered.
            }
        }
        case 0b0110011: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            uint8_t func7 = (instruction >> 25) & 0b1111111;
            switch(funct3) {
                case 0b000: {
                    switch(func7) {
                        case 0b0000000: return(I_RV64I_ADD);
                        case 0b0100000: return(I_RV64I_SUB);
                        case 0b0000001: return(I_RV64M_MUL);
                        default: return(I_RV64I_INVALID);
                    }
                }
                case 0b001: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_SLL);
                        case 0b0000001: return(I_RV64M_MULH);
                        default: return(I_RV64I_INVALID);
                    }
                }
                case 0b010: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_SLT);
                        case 0b0000001: return(I_RV64M_MULHSU);
                        default: return(I_RV64I_INVALID);
                    }
                }
                case 0b011: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_SLTU);
                        case 0b0000001: return(I_RV64M_MULHU);
                        default: return(I_RV64I_INVALID);
                    }
                }
                case 0b100: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_XOR);
                        case 0b0000001: return(I_RV64M_DIV);
                        default: return(I_RV64I_INVALID);
                    }
                }
                case 0b101: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_SRL);
                        case 0b0100000: return(I_RV64I_SRA);
                        case 0b0000001: return(I_RV64M_DIVU);
                        default: return(I_RV64I_INVALID);
                    }
                }
                case 0b110: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_OR);
                        case 0b0000001: return(I_RV64M_REM);
                        default: return(I_RV64I_INVALID);
                    }
                }
                case 0b111: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_AND);
                        case 0b0000001: return(I_RV64M_REMU);
                        default: return(I_RV64I_INVALID);
                    }
                }
                // No default needed, all cases covered.
            }
        }
        case 0b0001111: {
            // TODO: FENCE, FENCE.TSO, PAUSE
        }
        case 0b1110011: {
            // TODO: ECALL, EBREAK
        }

        case 0b0011011: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case 0b000: return(I_RV64I_ADDIW);
                case 0b001: return(I_RV64I_SLLIW);
                case 0b101: return(I_RV64I_SRLIW);
                case 0b100: return(I_RV64I_SRAIW);
                default: return(I_RV64I_INVALID);
            }
        }
        case 0b0111011: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            uint8_t func7 = (instruction >> 25) & 0b1111111;
            switch(funct3) {
                case 0b000: {
                    switch(func7) {
                        case 0b0000000: return(I_RV64I_ADDW);
                        case 0b0100000: return(I_RV64I_SUBW);
                        case 0b0000001: return(I_RV64M_MULW);
                        default: return(I_RV64I_INVALID);
                    }
                }
                case 0b001: return(I_RV64I_SLLW);
                case 0b100: return(I_RV64M_DIVW);
                case 0b101: {
                    switch(func7) {
                        case 0b0000000: return(I_RV64I_SRLW);
                        case 0b0100000: return(I_RV64I_SRAW);
                        case 0b0000001: return(I_RV64M_DIVUW);
                        default: return(I_RV64I_INVALID);
                    }
                }
                case 0b110: return(I_RV64M_REMW);
                case 0b111: return(I_RV64M_REMUW);
                default: return(I_RV64I_INVALID);
            }
        }
        case 0b0101111: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            uint8_t funct5 = (instruction >> 27) & 0b11111;
            switch(funct3) {
                case 0b010: {
                    switch(funct5) {
                        case 0b00010: return(I_RV64A_LR_W);
                        case 0b00011: return(I_RV64A_SC_W);
                        case 0b00001: return(I_RV64A_AMOSWAP_W);
                        case 0b00000: return(I_RV64A_AMOADD_W);
                        case 0b00100: return(I_RV64A_AMOXOR_W);
                        case 0b01100: return(I_RV64A_AMOAND_W);
                        case 0b01000: return(I_RV64A_AMOOR_W);
                        case 0b10000: return(I_RV64A_AMOMIN_W);
                        case 0b10100: return(I_RV64A_AMOMAX_W);
                        case 0b11000: return(I_RV64A_AMOMINU_W);
                        case 0b11100: return(I_RV64A_AMOMAXU_W);
                        default: return(I_RV64I_INVALID);
                    }
                }
                case 0b011: {
                    switch(funct5) {
                        case 0b00010: return(I_RV64A_LR_D);
                        case 0b00011: return(I_RV64A_SC_D);
                        case 0b00001: return(I_RV64A_AMOSWAP_D);
                        case 0b00000: return(I_RV64A_AMOADD_D);
                        case 0b00100: return(I_RV64A_AMOXOR_D);
                        case 0b01100: return(I_RV64A_AMOAND_D);
                        case 0b01000: return(I_RV64A_AMOOR_D);
                        case 0b10000: return(I_RV64A_AMOMIN_D);
                        case 0b10100: return(I_RV64A_AMOMAX_D);
                        case 0b11000: return(I_RV64A_AMOMINU_D);
                        case 0b11100: return(I_RV64A_AMOMAXU_D);
                        default: return(I_RV64I_INVALID);
                    }
                }
            }
        }
    }

    return(I_RV64I_INVALID);
}
