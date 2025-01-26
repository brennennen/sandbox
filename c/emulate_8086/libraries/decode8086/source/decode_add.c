/**
 * Responsible for decoding the "add" assembly instruction for the 8086. This assembly instruction
 * can be encoded into 3 different machine instructions. The order of these encodings will
 * follow the datasheet table 4-12.
 *
 * ADD 1 - I_ADD
 * ADD 2 - I_ADD_IMMEDIATE
 * ADD 3 - I_ADD_IMMEDIATE_TO_AX
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"
#include "libraries/decode8086/include/decode_utils.h"
#include "libraries/decode8086/include/decode_add.h"
#include "libraries/decode8086/include/decode_shared.h"

// MARK: ADD 1 - I_ADD
decode_result_t decode_add(
    decoder_t* decoder,
    uint8_t byte1,
    add_t* add)
{
    add->fields1 = byte1;
    direction_t direction = (add->fields1 & 0b00000010) >> 1;
    wide_t wide = add->fields1 & 0b00000001;
    decode_result_t read_byte2_result = dcd_read_byte(decoder, (uint8_t*) &add->fields2);
    if (read_byte2_result != DR_SUCCESS) {
        return read_byte2_result;
    }

    mod_t mod = (add->fields2 & 0b11000000) >> 6;
    reg_t reg = (add->fields2 & 0b00111000) >> 3;
    uint8_t rm = add->fields2 & 0b00000111;
    if (mod == MOD_MEMORY) {
        if (rm == 0b00000110) {
            decode_result_t read_displace_result = dcd_read_word(decoder, &add->displacement);
            if (read_displace_result != DR_SUCCESS) {
                return read_displace_result;
            }
        }
    } else if (mod == MOD_MEMORY_8BIT_DISPLACEMENT) {
        decode_result_t read_displace_result = dcd_read_byte(decoder, (uint8_t*) &add->displacement);
        if (read_displace_result != DR_SUCCESS) {
            return read_displace_result;
        }
    } else if (mod == MOD_MEMORY_16BIT_DISPLACEMENT) {
        decode_result_t read_displace_result = dcd_read_word(decoder, &add->displacement);
        if (read_displace_result != DR_SUCCESS) {
            return read_displace_result;
        }
    } else { // MOD_REGISTER
        // Don't have extra bytes for register to register movs. Nothing to do.
    }

    return SUCCESS;
}

void write_add(
    add_t* add,
    char* buffer,
    int* index,
    int buffer_size
) {
    direction_t direction = (add->fields1 & 0b00000010) >> 1;
    wide_t wide = add->fields1 & 0b00000001;
    mod_t mod = (add->fields2 & 0b11000000) >> 6;
    reg_t reg = (add->fields2 & 0b00111000) >> 3;
    uint8_t rm = add->fields2 & 0b00000111;

    write__common_register_or_memory_with_register_or_memory(direction, wide, mod, reg, rm,
        add->displacement, "add", 3, buffer, index, buffer_size);
}

// MARK: ADD 2 - I_ADD_IMMEDIATE_TO_REGISTER_OR_MEMORY

decode_result_t decode_add_immediate(
    decoder_t* decoder,
    uint8_t byte1,
    add_immediate_t* add
) {
    add->fields1 = byte1;
    uint8_t sign_extension = (add->fields1 & 0b00000010) >> 1;
    wide_t wide = add->fields1 & 0b00000001;
    decode_result_t read_byte2_result = dcd_read_byte(decoder, (uint8_t*) &add->fields2);
    if (read_byte2_result != DR_SUCCESS) {
        return read_byte2_result;
    }

    mod_t mod = (add->fields2 & 0b11000000) >> 6;
    uint8_t rm = add->fields2 & 0b00000111;
    if (mod == MOD_MEMORY) {
        if (rm == 0b00000110) {
            decode_result_t read_displace_result = dcd_read_word(decoder, &add->displacement);
            if (read_displace_result != DR_SUCCESS) {
                return read_displace_result;
            }
        }
    } else if (mod == MOD_MEMORY_8BIT_DISPLACEMENT) {
        decode_result_t read_displace_result = dcd_read_byte(decoder, (uint8_t*) &add->displacement);
        if (read_displace_result != DR_SUCCESS) {
            return read_displace_result;
        }
    } else if (mod == MOD_MEMORY_16BIT_DISPLACEMENT) {
        decode_result_t read_displace_result = dcd_read_word(decoder, &add->displacement);
        if (read_displace_result != DR_SUCCESS) {
            return read_displace_result;
        }
    } else { // MOD_REGISTER
        // Don't have extra bytes for register to register movs. Nothing to do.
    }

    if (wide == WIDE_BYTE) {
        decode_result_t read_data_result = dcd_read_byte(decoder, (uint8_t*) &add->immediate);
    } else {
        if (sign_extension == 0) {
            decode_result_t read_data_result = dcd_read_word(decoder, &add->immediate);
        } else {
            decode_result_t read_data_result = dcd_read_byte(decoder, (uint8_t*) &add->immediate);
        }
    }

    return DR_SUCCESS;
}

void write_add_immediate(
    add_immediate_t* add,
    char* buffer,
    int* index,
    int buffer_size
) {
    uint8_t is_signed = (add->fields1 & 0b00000010) >> 1;
    wide_t wide = add->fields1 & 0b00000001;
    mod_t mod = (add->fields2 & 0b11000000) >> 6;
    uint8_t rm = add->fields2 & 0b00000111;
    // TODO: refactor out similar code with mov_immediate

    if (mod == MOD_REGISTER) {
        char* reg_string = regb_strings[rm];
        if (wide) {
            reg_string = regw_strings[rm];
        }
        int written = snprintf(buffer + *index,  buffer_size - *index, "add %s, %d",
                                reg_string,
                                add->immediate);
        if (written < 0) {
            // TODO: propogate error
        }
        *index += written;
    } else {
        char effective_address_string[32] = { 0 };
        build_effective_address(effective_address_string, sizeof(effective_address_string),
                            mod, rm, add->displacement);
        int written = snprintf(buffer + *index,  buffer_size - *index, "add %s, %d",
                                effective_address_string,
                                add->immediate);
        if (written < 0) {
            // TODO: propogate error
        }
        *index += written;
    }
}

// MARK: ADD 3 - I_ADD_IMMEDIATE_TO_AX
