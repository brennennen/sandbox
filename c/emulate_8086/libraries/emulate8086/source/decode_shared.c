
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared/include/result.h"
#include "libraries/emulate8086/include/emulate8086.h"

#include "libraries/emulate8086/include/decode_shared.h"

/**
 *
 */
emu_result_t read_displacement(emulator_t* emulator, mod_t mod, uint8_t rm, uint16_t* displacement) {
    if (mod == MOD_MEMORY) {
        if (rm == 0b00000110) {
            emu_result_t read_displace_result = dcd_read_word(emulator, displacement);
            if (read_displace_result != ER_SUCCESS) {
                return read_displace_result;
            }
        }
    } else if (mod == MOD_MEMORY_8BIT_DISPLACEMENT) {
        emu_result_t read_displace_result = dcd_read_byte(emulator, (uint8_t*) displacement);
        if (read_displace_result != ER_SUCCESS) {
            return read_displace_result;
        }
    } else if (mod == MOD_MEMORY_16BIT_DISPLACEMENT) {
        emu_result_t read_displace_result = dcd_read_word(emulator, displacement);
        if (read_displace_result != ER_SUCCESS) {
            return read_displace_result;
        }
    } else {
        *displacement = 0;
    }
    return ER_SUCCESS;
}

/**
 *
 *
 *
 * Decodes the standard 4 byte format: [opcode dw][mod reg r/m][disp-lo][disp-high]
 * decode__opcode_d_w__mod_reg_rm__disp_lo__disp_hi
 */
emu_result_t emu_decode_common_standard_format(
    emulator_t* emulator,
    uint8_t byte1,
    direction_t* direction,
    wide_t* wide,
    mod_t* mod,
    uint8_t* reg,
    uint8_t* rm,
    uint16_t* displacement
) {
    //*fields1 = byte1;
    *direction = (byte1 & 0b00000010) >> 1;
    *wide = byte1 & 0b00000001;

    uint8_t byte2 = 0;
    emu_result_t read_byte2_result = dcd_read_byte(emulator, &byte2);
    if (read_byte2_result != ER_SUCCESS) {
        return read_byte2_result;
    }

    *mod = (byte2 & 0b11000000) >> 6;
    *reg = (byte2 & 0b00111000) >> 3;
    *rm = byte2 & 0b00000111;
    emu_result_t read_displacement_result = read_displacement(emulator, *mod, *rm, displacement);
    if (read_displacement_result != ER_SUCCESS) {
        return read_displacement_result;
    }

    return SUCCESS;
}


emu_result_t emu_decode_common_immediate_format(
    emulator_t* emulator,
    uint8_t byte1,
    uint8_t* sign,
    wide_t* wide,
    mod_t* mod,
    uint8_t* subcode,
    uint8_t* rm,
    uint16_t* displacement,
    uint16_t* data
) {
    *sign = (byte1 & 0b00000010) >> 1;
    *wide = byte1 & 0b00000001;
    uint8_t byte2 = 0;
    emu_result_t read_byte2_result = dcd_read_byte(emulator, (uint8_t*) &byte2);
    if (read_byte2_result != ER_SUCCESS) {
        return read_byte2_result;
    }

    *mod = (byte2 & 0b11000000) >> 6;
    *rm = byte2 & 0b00000111;
    if (*mod == MOD_MEMORY) {
        if (*rm == 0b00000110) {
            emu_result_t read_displace_result = dcd_read_word(emulator, displacement);
            if (read_displace_result != ER_SUCCESS) {
                return read_displace_result;
            }
        }
    } else if (*mod == MOD_MEMORY_8BIT_DISPLACEMENT) {
        emu_result_t read_displace_result = dcd_read_byte(emulator, (uint8_t*) displacement);
        if (read_displace_result != ER_SUCCESS) {
            return read_displace_result;
        }
    } else if (*mod == MOD_MEMORY_16BIT_DISPLACEMENT) {
        emu_result_t read_displace_result = dcd_read_word(emulator, displacement);
        if (read_displace_result != ER_SUCCESS) {
            return read_displace_result;
        }
    } else { // MOD_REGISTER
        // Don't have extra bytes for register to register movs. Nothing to do.
    }

    if (*wide == WIDE_BYTE) {
        emu_result_t read_data_result = dcd_read_byte(emulator, (uint8_t*) data);
    } else {
        if (*sign == 0) {
            emu_result_t read_data_result = dcd_read_word(emulator, data);
        } else {
            emu_result_t read_data_result = dcd_read_byte(emulator, (uint8_t*) data);
        }
    }

    return ER_SUCCESS;
}

void write_uint8(char* buffer, int* index, size_t buffer_size, uint8_t num) {
    // TODO: write something optimized. just use sprintf for now
    snprintf(buffer + *index, buffer_size - *index, "%d", num);
}

void write_uint16(char* buffer, int* index, size_t buffer_size, uint16_t num) {
    // TODO: write something optimized. just use sprintf for now
    snprintf(buffer + *index, buffer_size - *index, "%d", num);
}

char* mod_memory_effective_address_lookup(uint8_t rm) {
    switch(rm) {
        case RM_BX_SI: // 0
            return "[bp + si]";
        case RM_BX_DI: // 1
            return "[bx + di]";
        case RM_BP_SI:
            return "[bp + si]";
        case RM_BP_DI:
            return "[bp + di]";
        case RM_SI:
            return "[si]";
        case RM_DI:
            return "[di]";
        case RM_BP:
            return "INVALID";
        case RM_BX:
            return "[bx]";
        default:
            return "INVALID";
    };
}

char* map_register_field_encoding(reg_wide_t reg) {
    switch(reg) {
        case REGW_AX:
            return "ax";
        case REGW_CX:
            return "cx";
        case REGW_DX:
            return "dx";
        case REGW_BX:
            return "bx";
        case REGW_SP:
            return "sp";
        case REGW_BP:
            return "bp";
        case REGW_SI:
            return "si";
        case REGW_DI:
            return "di";
        default:
            return "??";
    }
}

/**
 * Maps the R/M field encodings to a string format. This function can handle
 * all R/M field encodings, but should only be used for "Effective Address"
 * encodings. There are simpler solutions for the standard register encodings.
 * See "Table 4-10. R/M (Register/Memory) Field Encoding".
 *
 * @param buffer Where to write the string encoding of the effective address to.
 * @param buffer_size Size of the output buffer being written to.
 * @param wide Whether the R/M field represents a byte or word.
 * @param mod Mode field encoding, used to determine how to parse the R/M field.
 *              See tables 4-8 and 4-10. for more details.
 * @param rm Register/Memory field encoding being processed. See table 4-10.
 * @param displacement Offset to add to the address when mod is 0b01 or 0b10.
 */
void build_effective_address(char* buffer, size_t buffer_size,
                             wide_t wide, mod_t mod, uint8_t rm,
                             uint16_t displacement) {
    switch(mod) {
        case MOD_MEMORY:
            switch(rm) {
                case RM_BX_SI:
                    memcpy(buffer, "[bx + si]", sizeof("[bx + si]"));
                    break;
                case RM_BX_DI:
                    memcpy(buffer, "[bx + di]", sizeof("[bx + di]"));
                    break;
                case RM_BP_SI:
                    memcpy(buffer, "[bp + si]", sizeof("[bp + si]"));
                    break;
                case RM_BP_DI:
                    memcpy(buffer, "[bp + di]", sizeof("[bp + di]"));
                    break;
                case RM_SI:
                    memcpy(buffer, "[si]", sizeof("[si]"));
                    break;
                case RM_DI:
                    memcpy(buffer, "[di]", sizeof("[di]"));
                    break;
                case RM_BP:
                    memcpy(buffer, "INVALID", sizeof("INVALID"));
                    break;
                case RM_BX:
                    memcpy(buffer, "[bx]", sizeof("[bx]"));
                    break;
                default:
                    memcpy(buffer, "INVALID", sizeof("INVALID"));
                    break;
            };
            break;
        case MOD_MEMORY_8BIT_DISPLACEMENT:
        // TODO: idea for performance, instead of falling through here, could build the string
        // with a "int to string" function tuned for each 8 bits and a different one tuned and
        // called for 16 bits and build up the string instead of using sprintf.
        case MOD_MEMORY_16BIT_DISPLACEMENT:
            switch(rm) {
                case RM_BX_SI:
                    snprintf(buffer, buffer_size, "[bx + si + %d]", displacement);
                    break;
                case RM_BX_DI:
                    snprintf(buffer, buffer_size, "[bx + di + %d]", displacement);
                    break;
                case RM_BP_SI:
                    snprintf(buffer, buffer_size, "[bp + si + %d]", displacement);
                    break;
                case RM_BP_DI:
                    snprintf(buffer, buffer_size, "[bp + di + %d]", displacement);
                    break;
                case RM_SI:
                    snprintf(buffer, buffer_size, "[si + %d]", displacement);
                    break;
                case RM_DI:
                    snprintf(buffer, buffer_size, "[di + %d]", displacement);
                    break;
                case RM_BP:
                    // BP with memory mode/no displacement is hijacked for direct immediates,
                    // so just [bp] gets converted to [bp + 0] displacement.
                    // ex: "mov dx, [bp]" is actually: "mov dx, [bp + 0]"
                    if (displacement == 0) {
                        snprintf(buffer, buffer_size, "[bp]");
                    } else {
                        snprintf(buffer, buffer_size, "[bp + %d]", displacement);
                    }
                    break;
                case RM_BX:
                    snprintf(buffer, buffer_size, "[bx + %d]", displacement);
                    break;
                default:
                    memcpy(buffer, "INVALID", sizeof("INVALID"));
                    break;
            }
            break;
        case MOD_REGISTER:
            switch(wide) {
                case WIDE_BYTE:
                    switch(rm) {
                        case RM_BX_SI: // 0
                            snprintf(buffer, buffer_size, "al");
                            break;
                        case RM_BX_DI: // 1
                            snprintf(buffer, buffer_size, "cl");
                            break;
                        case RM_BP_SI:
                            snprintf(buffer, buffer_size, "dl");
                            break;
                        case RM_BP_DI:
                            snprintf(buffer, buffer_size, "bl");
                            break;
                        case RM_SI:
                            snprintf(buffer, buffer_size, "ah");
                            break;
                        case RM_DI:
                            snprintf(buffer, buffer_size, "ch");
                            break;
                        case RM_BP:
                            snprintf(buffer, buffer_size, "dh");
                            break;
                        case RM_BX:
                            snprintf(buffer, buffer_size, "bh");
                            break;
                        default:
                            memcpy(buffer, "INVALID", sizeof("INVALID"));
                            break;
                    }
                    break;
                case WIDE_WORD:
                    switch(rm) {
                        case RM_BX_SI: // 0
                            memcpy(buffer, "ax", sizeof("ax"));
                            break;
                        case RM_BX_DI: // 1
                            memcpy(buffer, "cx", sizeof("cx"));
                            break;
                        case RM_BP_SI:
                            memcpy(buffer, "dx", sizeof("dx"));
                            break;
                        case RM_BP_DI:
                            memcpy(buffer, "bx", sizeof("bx"));
                            break;
                        case RM_SI:
                            memcpy(buffer, "sp", sizeof("sp"));
                            break;
                        case RM_DI:
                            memcpy(buffer, "bp", sizeof("bp"));
                            break;
                        case RM_BP:
                            memcpy(buffer, "si", sizeof("ax"));
                            break;
                        case RM_BX:
                            memcpy(buffer, "di", sizeof("di"));
                            break;
                        default:
                            memcpy(buffer, "INVALID", sizeof("INVALID"));
                            break;
                    }
                    break;
                default:
                    memcpy(buffer, "INVALID", sizeof("INVALID"));
                    break;
            }
            break;
        default:
            memcpy(buffer, "INVALID", sizeof("INVALID"));
            break;
    };
}

uint8_t* emu_get_byte_register(registers_t* registers, reg_t reg) {
    switch(reg) {
        case(REG_AL_AX): {
            return (uint8_t*) &registers->ax;
        }
        case(REG_CL_CX): {
            return (uint8_t*) &registers->cx;
        }
        case(REG_DL_DX): {
            return (uint8_t*) &registers->dx;
        }
        case(REG_BL_BX): {
            return (uint8_t*) &registers->bx;
        }
        case(REG_AH_SP): {
            return (uint8_t*) &registers->ax + 1;
        }
        case(REG_CH_BP): {
            return (uint8_t*) &registers->cx + 1;
        }
        case(REG_DH_SI): {
            return (uint8_t*) &registers->di + 1;
        }
        case(REG_BH_DI): {
            return (uint8_t*) &registers->bx + 1;
        }
    }
}

uint16_t* emu_get_word_register(registers_t* registers, reg_t reg) {
    switch(reg) {
        case(REG_AL_AX): {
            return &registers->ax;
        }
        case(REG_CL_CX): {
            return &registers->cx;
        }
        case(REG_DL_DX): {
            return &registers->dx;
        }
        case(REG_BL_BX): {
            return &registers->bx;
        }
        case(REG_AH_SP): {
            return &registers->sp;
        }
        case(REG_CH_BP): {
            return &registers->bp;
        }
        case(REG_DH_SI): {
            return &registers->si;
        }
        case(REG_BH_DI): {
            return &registers->di;
        }
    }
}

void write__common_register_or_memory_with_register_or_memory(
    direction_t direction,
    wide_t wide,
    mod_t mod,
    uint8_t reg,
    uint8_t rm,
    uint16_t displacement,
    char* mnemonic,
    uint8_t mnemonic_size,
    char* buffer,
    int* index,
    int buffer_size)
{
    if (mod == MOD_REGISTER) {
        // TODO: change both of these to uint16_t
        //uint8_t* left = &rm;
        uint8_t* left = &rm;
        //uint8_t* right = (uint8_t*)&reg;
        uint8_t* right = &reg;

        if (direction == DIR_FROM_REGISTER) {
            left = &reg;
            right = &rm;
        }

        char* left_string = regb_strings[*left];
        char* right_string = regb_strings[*right];

        if (wide == WIDE_WORD) {
            left_string = regw_strings[*left];
            right_string = regw_strings[*right];
        }

        int written = snprintf(buffer + *index, buffer_size - *index, "%s %s, %s",
                                mnemonic,
                                left_string,
                                right_string);
        if (written < 0) {
            // TODO: propogate error
        }
        *index += written;
    } else {
        // Special hi-jacked case for immediate value address "direct loading"
        if (mod == MOD_MEMORY && rm == RM_BP) {
            if (direction == DIR_TO_REGISTER) {
                char* reg_string = regb_strings[reg];
                if (wide == WIDE_WORD) {
                    reg_string = regw_strings[reg];
                }
                int written = snprintf(buffer + *index,  buffer_size - *index, "%s [%d], %s",
                                            mnemonic,
                                            displacement,
                                            reg_string);
                if (written < 0) {
                    // TODO: propogate error
                }
                *index += written;
            } else {
                char* reg_string = regb_strings[reg];
                if (wide == WIDE_WORD) {
                    reg_string = regw_strings[reg];
                }
                int written = snprintf(buffer + *index, buffer_size - *index, "%s %s, [%d]",
                                        mnemonic,
                                        reg_string,
                                        displacement);
                if (written < 0) {
                    // TODO: propogate error
                }
                *index += written;
            }

        } else {
            if (direction == DIR_TO_REGISTER) {
                char* reg_string = regb_strings[reg];
                if (wide == WIDE_WORD) {
                    reg_string = regw_strings[reg];
                }
                char effective_address_string[32] = { 0 };
                build_effective_address(effective_address_string, sizeof(effective_address_string),
                                        wide, mod, rm, displacement);
                int written = snprintf(buffer + *index,  buffer_size - *index, "%s %s, %s",
                                        mnemonic,
                                        effective_address_string,
                                        reg_string);
                if (written < 0) {
                    // TODO: propogate error
                }
                *index += written;
            } else {
                char* reg_string = regb_strings[reg];
                if (wide == WIDE_WORD) {
                    reg_string = regw_strings[reg];
                }
                char effective_address_string[32] = { 0 };
                build_effective_address(effective_address_string, sizeof(effective_address_string),
                                        wide, mod, rm, displacement);
                int written = snprintf(buffer + *index, buffer_size - *index, "%s %s, %s",
                                        mnemonic,
                                        reg_string,
                                        effective_address_string);
                if (written < 0) {
                    // TODO: propogate error
                }
                *index += written;
            }
            //snprintf(buffer, buffer_size, "NOT IMPLEMENTED!");
        }
    }
    // TODO: bounds safety checks
    snprintf(buffer + *index, buffer_size - *index, "\n");
    *index += 1;
}

void write__common_immediate_to_register_or_memory(
    uint8_t sign,
    wide_t wide,
    mod_t mod,
    uint8_t rm,
    uint16_t displacement,
    uint16_t immediate,
    char* mnemonic,
    uint8_t mnemonic_size,
    char* buffer,
    int* index,
    int buffer_size)
{
    printf("rm: %d, mod: %d\n", rm, mod);
    char effective_address_string[32] = { 0 };
    build_effective_address(effective_address_string, sizeof(effective_address_string),
                            wide, mod, rm, displacement);
    int written = snprintf(buffer + *index,  buffer_size - *index, "%s %s, %d",
                            mnemonic,
                            effective_address_string,
                            immediate);
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;
    snprintf(buffer + *index, buffer_size - *index, "\n");
    *index += 1;
}
