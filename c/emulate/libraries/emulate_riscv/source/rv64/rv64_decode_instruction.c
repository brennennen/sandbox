#include <stdint.h>

#include "logger.h"

#include "rv64/rv64_opcodes.h"
#include "rv64/rv64_instructions.h"
#include "rv64/rv64_decode_instruction.h"




/**
 * A full listing of opcodes and the mnemonic they associate with can be found
 * in the specification under the "RV32/64G Instruction Set Listings" table ("RISC-V
 * Instruction Set Manual Volume 1", Chapter 35, page 608).
 */
instruction_tag_rv64_t rv64_decode_instruction_tag(uint32_t instruction) {
    uint8_t opcode = instruction & 0b01111111; // opcodes are 7 bits
    switch(opcode) {
        case OP_LUI:                                return(I_RV64I_LUI);
        case OP_AUIPC:                              return(I_RV64I_AUIPC);
        case OP_JAL:                                return(I_RV64I_JAL);
        case OP_JALR:                               return(I_RV64I_JALR);
        case OPGRP_BRANCH: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case OPGRP_BRANCH__F3_BEQ:          return(I_RV64I_BEQ);
                case OPGRP_BRANCH__F3_BNE:          return(I_RV64I_BNE);
                case OPGRP_BRANCH__F3_BLT:          return(I_RV64I_BLT);
                case OPGRP_BRANCH__F3_BGE:          return(I_RV64I_BGE);
                case OPGRP_BRANCH__F3_BLTU:         return(I_RV64I_BLTU);
                case OPGRP_BRANCH__F3_BGEU:         return(I_RV64I_BGEU);
                default: return(I_RV64_INVALID);
            }
        }
        case OPGRP_LOAD: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case OPGRP_LOAD__F3_LB:             return(I_RV64I_LB);
                case OPGRP_LOAD__F3_LH:             return(I_RV64I_LH);
                case OPGRP_LOAD__F3_LW:             return(I_RV64I_LW);
                case OPGRP_LOAD__F3_LBU:            return(I_RV64I_LBU);
                case OPGRP_LOAD__F3_LHU:            return(I_RV64I_LHU);
                case OPGRP_LOAD__F3_LWU:            return(I_RV64I_LWU); // RV64I Addition (not in RV32)
                case OPGRP_LOAD__F3_LD:             return(I_RV64I_LD); // RV64I Addition
                default: return(I_RV64_INVALID);
            }
        }
        case OPGRP_STORE: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case OPGRP_STORE__F3_SB:            return(I_RV64I_SB);
                case OPGRP_STORE__F3_SH:            return(I_RV64I_SH);
                case OPGRP_STORE__F3_SW:            return(I_RV64I_SW);
                case OPGRP_STORE__F3_SD:            return(I_RV64I_SD); // RV64I Addition
                default: return(I_RV64_INVALID);
            }
        }
        case OPGRP_ALU_IMMED: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case OPGRP_ALU_IMMED__F3_ADDI:        return(I_RV64I_ADDI);
                case OPGRP_ALU_IMMED__F3_SLTI:        return(I_RV64I_SLTI);
                case OPGRP_ALU_IMMED__F3_SLTIU:       return(I_RV64I_SLTIU);
                case OPGRP_ALU_IMMED__F3_XORI:        return(I_RV64I_XORI);
                case OPGRP_ALU_IMMED__F3_ORI:         return(I_RV64I_ORI);
                case OPGRP_ALU_IMMED__F3_ANDI:        return(I_RV64I_ANDI);
                case OPGRP_ALU_IMMED__F3_SLLI:        return(I_RV64I_SLLI); // RV64I adds SHAMT=31 as a valid value
                case OPGRP_ALU_IMMED__F3GRP_SHIFT_RIGHT: {
                    uint8_t funct7 = (instruction >> 25) & 0b1111111;
                    switch(funct7) {
                        case OPGRP_ALU_IMMED__F3GRP_SHIFT_RIGHT__F7_SRLI: return(I_RV64I_SRLI); // Noted as some features being extra in RV64I, but not clear what exactly is.
                        case OPGRP_ALU_IMMED__F3GRP_SHIFT_RIGHT__F7_SRAI: return(I_RV64I_SRAI); // RV64I adds SHAMT=7 as a valid value
                        default: return(I_RV64_INVALID);
                    }
                }
                // No default needed, all cases covered.
            }
        }
        case OPGRP_MATH_REG: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            uint8_t funct7 = (instruction >> 25) & 0b1111111;
            switch(funct3) {
                case OPGRP_MATH_REG__F3GRP_MATH: {
                    switch(funct7) {
                        case OPGRP_MATH_REG__F3GRP_MATH__F7_ADD: return(I_RV64I_ADD);
                        case OPGRP_MATH_REG__F3GRP_MATH__F7_SUB: return(I_RV64I_SUB);
                        case OPGRP_MATH_REG__F3GRP_MATH__F7_MUL: return(I_RV64M_MUL);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b001: {
                    switch (funct7) {
                        case 0b0000000: return(I_RV64I_SLL);
                        case 0b0000001: return(I_RV64M_MULH);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b010: {
                    switch (funct7) {
                        case 0b0000000: return(I_RV64I_SLT);
                        case 0b0000001: return(I_RV64M_MULHSU);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b011: {
                    switch (funct7) {
                        case 0b0000000: return(I_RV64I_SLTU);
                        case 0b0000001: return(I_RV64M_MULHU);
                        default: return(I_RV64_INVALID);
                    }
                }
                case OPGRP_MATH_REG__F3GRP100: {
                    switch (funct7) {
                        case OPGRP_MATH_REG__F3GRP100__F7_XOR: return(I_RV64I_XOR);
                        case OPGRP_MATH_REG__F3GRP100__F7_DIV: return(I_RV64M_DIV);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b101: {
                    switch (funct7) {
                        case 0b0000000: return(I_RV64I_SRL);
                        case 0b0100000: return(I_RV64I_SRA);
                        case 0b0000001: return(I_RV64M_DIVU);
                        default: return(I_RV64_INVALID);
                    }
                }
                case OPGRP_MATH_REG__F3GRP110: {
                    switch (funct7) {
                        case OPGRP_MATH_REG__F3GRP110__F7_OR: return(I_RV64I_OR);
                        case OPGRP_MATH_REG__F3GRP110__F7_REM: return(I_RV64M_REM);
                        default: return(I_RV64_INVALID);
                    }
                }
                case OPGRP_MATH_REG__F3GRP111: {
                    switch (funct7) {
                        case OPGRP_MATH_REG__F3GRP111__F7_AND: return(I_RV64I_AND);
                        case OPGRP_MATH_REG__F3GRP111__F7_REMU: return(I_RV64M_REMU);
                        default: return(I_RV64_INVALID);
                    }
                }
                // No default needed, all cases covered.
            }
        }
        case 0b0001111: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case 0b000: {
                    // TODO: check the below, doesn't seem right, but also not using these ops yet
                    uint8_t funct4_1 = (instruction >> 28) & 0b1111;
                    uint8_t funct4_2 = (instruction >> 24) & 0b1111;
                    uint8_t funct4_3 = (instruction >> 20) & 0b1111;
                    if (funct4_1 == 0b1000 && funct4_2 == 0b0011 && funct4_3 == 0b0011) {
                        return(I_RV64I_FENCE_TSO);
                    } else if (funct4_1 == 0b0000 && funct4_2 == 0b0001 && funct4_3 == 0b0000) {
                        return(I_RV64I_PAUSE);
                    } else {
                        return(I_RV64I_FENCE);
                    }
                }
                case 0b001: return(I_RV64ZIFENCEI_FENCE_I);
                default: return(I_RV64_INVALID);
            }

        }
        case 0b1110011: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case 0b000: {
                    uint8_t funct12 = (instruction >> 20) & 0b111111111111;
                    switch(funct12) {
                        case 0b000000000000: return(I_RV64I_ECALL);
                        case 0b000000000001: return(I_RV64I_EBREAK);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b001: return(I_RV64ZICSR_CSRRW);
                case 0b010: return(I_RV64ZICSR_CSRRS);
                case 0b011: return(I_RV64ZICSR_CSRRC);
                case 0b101: return(I_RV64ZICSR_CSRRWI);
                case 0b110: return(I_RV64ZICSR_CSRRSI);
                case 0b111: return(I_RV64ZICSR_CSRRCI);
                default: return(I_RV64_INVALID);
            }
        }
        case OPGRP_IMMED_WIDE: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case OPGRP_IMMED_WIDE__F3_ADDIW: return(I_RV64I_ADDIW);
                case OPGRP_IMMED_WIDE__F3_SLLIW: return(I_RV64I_SLLIW);
                case 0b101: {
                    uint8_t funct7 = (instruction >> 25) & 0b1111111;
                    switch(funct7) {
                        case OPGRP_IMMED_WIDE__F3GRP_SHIFT_RIGHT__F7_SRLIW: return(I_RV64I_SRLIW);
                        case OPGRP_IMMED_WIDE__F3GRP_SHIFT_RIGHT__F7_SRAIW: return(I_RV64I_SRAIW);
                        default: return(I_RV64_INVALID);
                    }
                }
                default: return(I_RV64_INVALID);
            }
        }
        case OPGRP_REG_WIDE: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            uint8_t funct7 = (instruction >> 25) & 0b1111111;
            switch(funct3) {
                case OPGRP_REG_WIDE__F3GRP000: {
                    switch(funct7) {
                        case OPGRP_REG_WIDE__F3GRP000__F7_ADDW: return(I_RV64I_ADDW);
                        case OPGRP_REG_WIDE__F3GRP000__F7_MULW: return(I_RV64M_MULW);
                        case OPGRP_REG_WIDE__F3GRP000__F7_SUBW: return(I_RV64I_SUBW);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b001: return(I_RV64I_SLLW);
                case 0b100: return(I_RV64M_DIVW);
                case 0b101: {
                    switch(funct7) {
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
                        case 0b00001: return(I_RV64ZAAMO_AMOSWAP_W);
                        case 0b00000: return(I_RV64ZAAMO_AMOADD_W);
                        case 0b00100: return(I_RV64ZAAMO_AMOXOR_W);
                        case 0b01100: return(I_RV64ZAAMO_AMOAND_W);
                        case 0b01000: return(I_RV64ZAAMO_AMOOR_W);
                        case 0b10000: return(I_RV64ZAAMO_AMOMIN_W);
                        case 0b10100: return(I_RV64ZAAMO_AMOMAX_W);
                        case 0b11000: return(I_RV64ZAAMO_AMOMINU_W);
                        case 0b11100: return(I_RV64ZAAMO_AMOMAXU_W);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b011: {
                    switch(funct5) {
                        case 0b00010: return(I_RV64ZALRSC_LR_D);
                        case 0b00011: return(I_RV64ZALRSC_SC_D);
                        case 0b00001: return(I_RV64ZAAMO_AMOSWAP_D);
                        case 0b00000: return(I_RV64ZAAMO_AMOADD_D);
                        case 0b00100: return(I_RV64ZAAMO_AMOXOR_D);
                        case 0b01100: return(I_RV64ZAAMO_AMOAND_D);
                        case 0b01000: return(I_RV64ZAAMO_AMOOR_D);
                        case 0b10000: return(I_RV64ZAAMO_AMOMIN_D);
                        case 0b10100: return(I_RV64ZAAMO_AMOMAX_D);
                        case 0b11000: return(I_RV64ZAAMO_AMOMINU_D);
                        case 0b11100: return(I_RV64ZAAMO_AMOMAXU_D);
                        default: return(I_RV64_INVALID);
                    }
                }
            }
        }
        case RV64_OPCODE_GROUP_FLOAT_LOAD: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case RV64_OPCODE_GROUP_FLOAT_LOAD_FUNCT3_FLW: return(I_RV64F_FLW);
                case RV64_OPCODE_GROUP_FLOAT_LOAD_FUNCT3_FLD: return(I_RV64D_FLD);
                case RV64_OPCODE_GROUP_FLOAT_LOAD_FUNCT3_FLQ: return(I_RV64Q_FLQ);
                case RV64_OPCODE_GROUP_FLOAT_LOAD_FUNCT3_FLH: return(I_RV64ZFH_FLH);
                case 0b111: return(I_RV64V_VLE64_V);
                case 0b110: return(I_RV64V_VLE32_V);
                case 0b101: return(I_RV64V_VLE16_V);
                case 0b000: return(I_RV64V_VLE8_V);
                default: return(I_RV64_INVALID);
            }
        }
        case RV64_OPCODE_GROUP_FLOAT_STORE: {
            uint8_t funct3 = (instruction >> 12) & 0b111;
            switch(funct3) {
                case RV64_OPCODE_GROUP_FLOAT_STORE_FUNCT3_FSW: return(I_RV64F_FSW);
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
        case RV64_OPCODE_GROUP_FLOAT_FMADD: {
            uint8_t funct2 = (instruction >> 25) & 0b11;
            switch(funct2) {
                case RV64_OPCODE_GROUP_FLOAT_FMADD_FUNCT2_S: return(I_RV64F_FMADD_S);
                case 0b01: return(I_RV64D_FMADD_D);
                case 0b11: return(I_RV64Q_FMADD_Q);
                case 0b10: return(I_RV64ZFH_FMADD_H);
                default: return(I_RV64_INVALID);
            }
        }
        case RV64_OPCODE_GROUP_FLOAT_FMSUB: {
            uint8_t funct2 = (instruction >> 25) & 0b11;
            switch(funct2) {
                case RV64_OPCODE_GROUP_FLOAT_FMSUB_FUNCT2_S: return(I_RV64F_FMSUB_S);
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
                        case 0b000000: return(I_RV64V_VADD_IVV);
                        case 0b000010: return(I_RV64V_VSUB_IVV);
                        default: return(I_RV64_INVALID);
                    }
                }
                case 0b001: return(I_RV64_INVALID); // OPFVV - TODO
                case 0b010: return(I_RV64_INVALID); // OPMVV - TODO
                case 0b011: { // OPIVI
                    switch(funct6) {
                        case 0b000000: return(I_RV64V_VADD_IVI);
                        case 0b000011: return(I_RV64V_VRSUB_IVI);
                        default: return(I_RV64_INVALID);
                    }
                    return(I_RV64_INVALID);
                }
                case 0b100: { // OPIVX
                    switch(funct6) {
                        case 0b000000: return(I_RV64V_VADD_IVX);
                        case 0b000010: return(I_RV64V_VSUB_IVX);
                        case 0b000011: return(I_RV64V_VRSUB_IVX);
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
