
#include "shared/include/binary_utilities.h"
#include "shared/include/instructions.h"
#include "shared/include/result.h"

#include "libraries/decode8086/include/decode8086.h"

decode_result_t dcd_read_byte(decoder_t* decoder, uint8_t* out_byte) {
    if (decoder->buffer_index >= decoder->buffer_size) {
        printf("dcd_read_byte: DR_OUT_OF_BOUNDS. i: %d >= s: %ld\n",
            decoder->buffer_index, decoder->buffer_size);
        return DR_OUT_OF_BOUNDS;
    }
    *out_byte = decoder->buffer[decoder->buffer_index];
    decoder->buffer_index += 1;
    return DR_SUCCESS;
}

decode_result_t dcd_read_word(decoder_t* decoder, uint16_t* out_word) {
    if (decoder->buffer_index + 1 >= decoder->buffer_size) {
        printf("dcd_read_word: DR_OUT_OF_BOUNDS. i: %d >= s: %ld\n",
            decoder->buffer_index, decoder->buffer_size);
        return DR_OUT_OF_BOUNDS;
    }
    *out_word = decoder->buffer[decoder->buffer_index] | (decoder->buffer[decoder->buffer_index + 1] << 8);
    decoder->buffer_index += 2;
    return DR_SUCCESS;
}
