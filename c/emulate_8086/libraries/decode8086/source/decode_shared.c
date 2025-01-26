
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libraries/decode8086/include/decode_shared.h"

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

void build_effective_address(char* buffer, size_t buffer_size,
                             mod_t mod, uint8_t rm, uint16_t displacement) {
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
            
        default:
            memcpy(buffer, "INVALID", sizeof("INVALID"));
            break;
    };
}

void write__common_register_or_memory_with_register_or_memory(
    direction_t direction,
    wide_t wide,
    mod_t mod,
    uint16_t reg,
    uint16_t rm,
    uint16_t displacement,
    char* mnemonic,
    uint8_t mnemonic_size,
    char* buffer,
    int* index,
    int buffer_size)
{
    if (mod == MOD_REGISTER) {
        uint8_t* left = &rm;
        uint8_t* right = (uint8_t*)&reg;

        if (direction == DIR_FROM_REGISTER) {
            left = (uint8_t*)&reg;
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
                                        mod, rm, displacement);
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
                                        mod, rm, displacement);
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
}
