/**
 * Responsible for decoding the "add" assembly instruction for the 8086. This assembly instruction
 * can be encoded into 3 different machine instructions. The order of these encodings will
 * follow the datasheet table 4-12.
 *
 * ADD 1 - I_ADD_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER
 * ADD 2 - I_ADD_IMMEDIATE_TO_REGISTER_OR_MEMORY
 * ADD 3 - I_ADD_IMMEDIATE_TO_ACCUMULATOR
 */

#include <string.h>

#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"
#include "libraries/decode8086/include/decode_utils.h"
#include "libraries/decode8086/include/decode_add.h"

// MARK: ADD 1 - I_ADD_REGISTER_OR_MEMORY_WITH_REGISTER_TO_EITHER
decode_result_t decode__add_register_or_memory_with_register_to_either(
    decoder_t* decoder,
    uint8_t byte1,
    add_register_or_memory_with_register_to_either_t* add)
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

void write__add_register_or_memory_with_register_to_either(
    add_register_or_memory_with_register_to_either_t* add,
    char* buffer,
    int* index,
    int buffer_size
) {
    // TODO: refactor out a bunch of the "write__mov_xxxx" code to be able to use it here too.
    // lot of redundant work...
    int written = snprintf(buffer + *index, buffer_size - *index, "add ?, ?");
    if (written < 0) {
        // TODO: propogate error
    }
    *index += written;


}