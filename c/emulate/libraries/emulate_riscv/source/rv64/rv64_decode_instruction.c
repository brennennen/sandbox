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
                default: return(I_RV64_INVALID);
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
                default: return(I_RV64_INVALID);
            }
        }
        case 0b0100011: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case 0b000: return(I_RV64I_SB);
                case 0b001: return(I_RV64I_SH);
                case 0b010: return(I_RV64I_SW);
                case 0b100: return(I_RV64I_SD); // RV64I Addition
                default: return(I_RV64_INVALID);
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
                        default: return(I_RV64_INVALID);
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
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b001: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_SLL);
                        case 0b0000001: return(I_RV64M_MULH);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b010: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_SLT);
                        case 0b0000001: return(I_RV64M_MULHSU);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b011: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_SLTU);
                        case 0b0000001: return(I_RV64M_MULHU);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b100: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_XOR);
                        case 0b0000001: return(I_RV64M_DIV);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b101: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_SRL);
                        case 0b0100000: return(I_RV64I_SRA);
                        case 0b0000001: return(I_RV64M_DIVU);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b110: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_OR);
                        case 0b0000001: return(I_RV64M_REM);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b111: {
                    switch (func7) {
                        case 0b0000000: return(I_RV64I_AND);
                        case 0b0000001: return(I_RV64M_REMU);
                        default: return(I_RV64_INVALID);
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
                default: return(I_RV64_INVALID);
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
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b001: return(I_RV64I_SLLW);
                case 0b100: return(I_RV64M_DIVW);
                case 0b101: {
                    switch(func7) {
                        case 0b0000000: return(I_RV64I_SRLW);
                        case 0b0100000: return(I_RV64I_SRAW);
                        case 0b0000001: return(I_RV64M_DIVUW);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b110: return(I_RV64M_REMW);
                case 0b111: return(I_RV64M_REMUW);
                default: return(I_RV64_INVALID);
            }
        }
        case 0b0101111: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            uint8_t funct5 = (instruction >> 27) & 0b11111;
            switch(funct3) {
                case 0b010: {
                    switch(funct5) {
                        case 0b00010: return(I_RV64ZALRSC_LR_W);
                        case 0b00011: return(I_RV64ZALRSC_SC_W);
                        case 0b00001: return(I_RV64ZAWRS_AMOSWAP_W);
                        case 0b00000: return(I_RV64ZAWRS_AMOADD_W);
                        case 0b00100: return(I_RV64ZAWRS_AMOXOR_W);
                        case 0b01100: return(I_RV64ZAWRS_AMOAND_W);
                        case 0b01000: return(I_RV64ZAWRS_AMOOR_W);
                        case 0b10000: return(I_RV64ZAWRS_AMOMIN_W);
                        case 0b10100: return(I_RV64ZAWRS_AMOMAX_W);
                        case 0b11000: return(I_RV64ZAWRS_AMOMINU_W);
                        case 0b11100: return(I_RV64ZAWRS_AMOMAXU_W);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b011: {
                    switch(funct5) {
                        case 0b00010: return(I_RV64ZALRSC_LR_D);
                        case 0b00011: return(I_RV64ZALRSC_SC_D);
                        case 0b00001: return(I_RV64ZAWRS_AMOSWAP_D);
                        case 0b00000: return(I_RV64ZAWRS_AMOADD_D);
                        case 0b00100: return(I_RV64ZAWRS_AMOXOR_D);
                        case 0b01100: return(I_RV64ZAWRS_AMOAND_D);
                        case 0b01000: return(I_RV64ZAWRS_AMOOR_D);
                        case 0b10000: return(I_RV64ZAWRS_AMOMIN_D);
                        case 0b10100: return(I_RV64ZAWRS_AMOMAX_D);
                        case 0b11000: return(I_RV64ZAWRS_AMOMINU_D);
                        case 0b11100: return(I_RV64ZAWRS_AMOMAXU_D);
                        default: return(I_RV64_INVALID);
                    }
                }
            }
        }
        case 0b0000111: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case 0b010: return(I_RV64F_FLW);
                case 0b011: return(I_RV64D_FLD);
                case 0b100: return(I_RV64Q_FLQ);
                case 0b001: return(I_RV64ZFH_FLH);
                case 0b111: return(I_RV64V_VLE64_V);
                case 0b110: return(I_RV64V_VLE32_V);
                case 0b101: return(I_RV64V_VLE16_V);
                case 0b000: return(I_RV64V_VLE8_V);
                default: return(I_RV64_INVALID);
            }
        }
        case 0b0100111: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case 0b010: return(I_RV64F_FSW);
                case 0b011: return(I_RV64D_FSD);
                case 0b100: return(I_RV64Q_FSQ);
                case 0b001: return(I_RV64ZFH_FSH);

                case 0b111: return(I_RV64V_VSE64_V);
                case 0b110: return(I_RV64V_VSE32_V);
                case 0b101: return(I_RV64V_VSE16_V);
                case 0b000: return(I_RV64V_VSE8_V);
                default: return(I_RV64_INVALID);
            }
        }
        case 0b1000011: {
            uint8_t funct2 = (instruction >> 25) & 0b11;
            switch(funct2) {
                case 0b00: return(I_RV64F_FMADD_S);
                case 0b01: return(I_RV64D_FMADD_D);
                case 0b11: return(I_RV64Q_FMADD_Q);
                case 0b10: return(I_RV64ZFH_FMADD_H);
                default: return(I_RV64_INVALID);
            }
        }
        case 0b1000111: {
            uint8_t funct2 = (instruction >> 25) & 0b11;
            switch(funct2) {
                case 0b00: return(I_RV64F_FMSUB_S);
                case 0b01: return(I_RV64D_FMSUB_D);
                case 0b11: return(I_RV64Q_FMSUB_Q);
                case 0b10: return(I_RV64ZFH_FMSUB_H);
                default: return(I_RV64_INVALID);
            }
        }
        case 0b1001011: {
            uint8_t funct2 = (instruction >> 25) & 0b11;
            switch(funct2) {
                case 0b00: return(I_RV64F_FNMSUB_S);
                case 0b01: return(I_RV64D_FNMSUB_D);
                case 0b11: return(I_RV64Q_FNMSUB_Q);
                case 0b10: return(I_RV64ZFH_FNMSUB_H);
                default: return(I_RV64_INVALID);
            }
        }
        case 0b1001111: {
            uint8_t funct2 = (instruction >> 25) & 0b11;
            switch(funct2) {
                case 0b00: return(I_RV64F_FNMADD_S);
                case 0b01: return(I_RV64D_FNMADD_D);
                case 0b11: return(I_RV64Q_FNMADD_Q);
                case 0b10: return(I_RV64ZFH_FNMADD_H);
                default: return(I_RV64_INVALID);
            }
        }
        case 0b1010011: {
            uint8_t funct7 = (instruction >> 25) & 0b1111111;
            switch(funct7) {
                case 0b0000000: return(I_RV64F_FADD_S);
                case 0b0000001: return(I_RV64D_FADD_D);
                case 0b0000011: return(I_RV64Q_FADD_Q);
                case 0b0000010: return(I_RV64ZFH_FADD_H);
                case 0b0000100: return(I_RV64F_FSUB_S);
                case 0b0000101: return(I_RV64D_FSUB_D);
                case 0b0000111: return(I_RV64Q_FSUB_Q);
                case 0b0000110: return(I_RV64ZFH_FSUB_H);
                case 0b0001000: return(I_RV64F_FMUL_S);
                case 0b0001001: return(I_RV64D_FMUL_D);
                case 0b0001011: return(I_RV64Q_FMUL_Q);
                case 0b0001010: return(I_RV64ZFH_FMUL_H);
                case 0b0001100: return(I_RV64F_FDIV_S);
                case 0b0001101: return(I_RV64D_FDIV_D);
                case 0b0001111: return(I_RV64Q_FDIV_Q);
                case 0b0001110: return(I_RV64ZFH_FDIV_H);
                case 0b0101100: return(I_RV64F_FSQRT_S);
                case 0b0101101: return(I_RV64D_FSQRT_D);
                case 0b0101111: return(I_RV64Q_FSQRT_Q);
                case 0b0101110: return(I_RV64ZFH_FSQRT_H);
                case 0b0010000: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b000: return(I_RV64F_FSGNJ_S);
                        case 0b001: return(I_RV64F_FSGNJN_S);
                        case 0b010: return(I_RV64F_FSGNJX_S);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b0010001: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b000: return(I_RV64D_FSGNJ_D);
                        case 0b001: return(I_RV64D_FSGNJN_D);
                        case 0b010: return(I_RV64D_FSGNJX_D);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b0010011: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b000: return(I_RV64Q_FSGNJ_Q);
                        case 0b001: return(I_RV64Q_FSGNJN_Q);
                        case 0b010: return(I_RV64Q_FSGNJX_Q);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b0010010: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b000: return(I_RV64ZFH_FSGNJ_H);
                        case 0b001: return(I_RV64ZFH_FSGNJN_H);
                        case 0b010: return(I_RV64ZFH_FSGNJX_H);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b0010100: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b000: return(I_RV64F_FMIN_S);
                        case 0b001: return(I_RV64F_FMAX_S);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b0010101: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b000: return(I_RV64D_FMIN_D);
                        case 0b001: return(I_RV64D_FMAX_D);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b0010111: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b000: return(I_RV64Q_FMIN_Q);
                        case 0b001: return(I_RV64Q_FMAX_Q);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b0010110: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b000: return(I_RV64ZFH_FMIN_H);
                        case 0b001: return(I_RV64ZFH_FMAX_H);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1100000: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64F_FCVT_W_S);
                        case 0b00001: return(I_RV64F_FCVT_WU_S);
                        case 0b00010: return(I_RV64F_FCVT_L_S);
                        case 0b00011: return(I_RV64F_FCVT_LU_S);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1100001: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64D_FCVT_W_D);
                        case 0b00001: return(I_RV64D_FCVT_WU_D);
                        case 0b00010: return(I_RV64D_FCVT_L_D);
                        case 0b00011: return(I_RV64D_FCVT_LU_D);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b0100000: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00001: return(I_RV64D_FCVT_S_D);
                        case 0b00011: return(I_RV64Q_FCVT_S_Q);
                        case 0b00010: return(I_RV64ZFH_FCVT_S_H);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b0100010: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64ZFH_FCVT_H_S);
                        case 0b00001: return(I_RV64ZFH_FCVT_H_D);
                        case 0b00011: return(I_RV64ZFH_FCVT_H_Q);
                        default: return(I_RV64_INVALID);
                    }
                }

                case 0b0100001: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64D_FCVT_D_S);
                        case 0b00011: return(I_RV64Q_FCVT_D_Q);
                        case 0b00010: return(I_RV64ZFH_FCVT_D_H);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b0100011: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64Q_FCVT_Q_S);
                        case 0b00001: return(I_RV64Q_FCVT_Q_D);
                        case 0b00010: return(I_RV64ZFH_FCVT_Q_H);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1100010: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64ZFH_FCVT_W_H);
                        case 0b00001: return(I_RV64ZFH_FCVT_WU_H);
                        case 0b00010: return(I_RV64ZFH_FCVT_L_H);
                        case 0b00011: return(I_RV64ZFH_FCVT_LU_H);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1110000: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b000: return(I_RV64F_FMV_X_W);
                        case 0b001: return(I_RV64F_FCLASS_S);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1110001: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b000: return(I_RV64D_FMV_X_D);
                        case 0b001: return(I_RV64D_FCLASS_D);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1110011: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b001: return(I_RV64Q_FCLASS_Q);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1110010: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b001: return(I_RV64ZFH_FCLASS_H);
                        case 0b000: return(I_RV64ZFH_FMV_X_H);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1010000: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b010: return(I_RV64F_FEQ_S);
                        case 0b001: return(I_RV64F_FLT_S);
                        case 0b000: return(I_RV64F_FLE_S);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1010001: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b010: return(I_RV64D_FEQ_D);
                        case 0b001: return(I_RV64D_FLT_D);
                        case 0b000: return(I_RV64D_FLE_D);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1010011: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b010: return(I_RV64Q_FEQ_Q);
                        case 0b001: return(I_RV64Q_FLT_Q);
                        case 0b000: return(I_RV64Q_FLE_Q);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1010010: {
                    uint8_t funct3 = (instruction >> 12) & 0b111;
                    switch(funct3) {
                        case 0b010: return(I_RV64ZFH_FEQ_H);
                        case 0b001: return(I_RV64ZFH_FLT_H);
                        case 0b000: return(I_RV64ZFH_FLE_H);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1101000: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64F_FCVT_S_W);
                        case 0b00001: return(I_RV64F_FCVT_S_WU);
                        case 0b00010: return(I_RV64F_FCVT_S_L);
                        case 0b00011: return(I_RV64F_FCVT_S_LU);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1101001: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64D_FCVT_D_W);
                        case 0b00001: return(I_RV64D_FCVT_D_WU);
                        case 0b00010: return(I_RV64D_FCVT_D_L);
                        case 0b00011: return(I_RV64D_FCVT_D_LU);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1100011: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64Q_FCVT_W_Q);
                        case 0b00001: return(I_RV64Q_FCVT_WU_Q);
                        case 0b00010: return(I_RV64Q_FCVT_L_Q);
                        case 0b00011: return(I_RV64Q_FCVT_LU_Q);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1101011: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64Q_FCVT_Q_W);
                        case 0b00001: return(I_RV64Q_FCVT_Q_WU);
                        case 0b00010: return(I_RV64Q_FCVT_Q_L);
                        case 0b00011: return(I_RV64Q_FCVT_Q_LU);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1101010: {
                    uint8_t funct5 = (instruction >> 20) & 0b11111;
                    switch(funct5) {
                        case 0b00000: return(I_RV64ZFH_FCVT_H_W);
                        case 0b00001: return(I_RV64ZFH_FCVT_H_WU);
                        case 0b00010: return(I_RV64ZFH_FCVT_H_L);
                        case 0b00011: return(I_RV64ZFH_FCVT_H_LU);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b1111010: return(I_RV64ZFH_FMV_H_X);
                case 0b1111000: return(I_RV64F_FMV_W_X);
                case 0b1111001: return(I_RV64D_FMV_D_X);
                default: return(I_RV64_INVALID);
            }
        }
        // RV64V
        case 0b1010111: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            uint8_t funct6 = (instruction >> 26) & 0b111111;
            switch(funct3) {
                // 30.10
                case 0b000: { // OPIVV
                    switch(funct6) {
                        case 0b000000: return(I_RV64V_VADD_VV);
                        case 0b000010: return(I_RV64V_VSUB_VV);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b001: return(I_RV64_INVALID); // OPFVV - TODO
                case 0b010: return(I_RV64_INVALID); // OPMVV - TODO
                case 0b011: { // OPIVI
                    switch(funct6) {
                        case 0b000000: return(I_RV64V_VADD_VI);
                        case 0b000011: return(I_RV64V_VRSUB_VI);
                        default: return(I_RV64_INVALID);
                    }
                    return(I_RV64_INVALID);
                }
                case 0b100: { // OPIVX
                    switch(funct6) {
                        case 0b000000: return(I_RV64V_VADD_VX);
                        case 0b000010: return(I_RV64V_VSUB_VX);
                        case 0b000011: return(I_RV64V_VRSUB_VX);
                        default: return(I_RV64_INVALID);
                    }
                    return(I_RV64_INVALID);
                }
                case 0b101: return(I_RV64_INVALID); // OPFVF - TODO
                case 0b110: { // OPMVX
                    return(I_RV64_INVALID);
                }
                case 0b111: {
                    uint8_t funct2 = (instruction >> 30) & 0b11; // last 2 bits
                    switch(funct2) {
                        case 0b00: return(I_RV64V_VSETVLI);
                        case 0b11: return(I_RV64V_VSETIVLI);
                        case 0b10: return(I_RV64V_VSETVL);
                        default: return(I_RV64_INVALID);
                    }
                }
                default: return(I_RV64_INVALID);
            }
        }
    }
    return(I_RV64_INVALID);
}
