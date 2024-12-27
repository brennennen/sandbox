/**
 * gcc ./bit_maps.c && ./a.out
 * 
 * Small exploration around setting and retrieving individual bits from bytes.
 * 
 * Using bitmaps is common in performance critical or constrained environments,
 * such as embedded or game engines (where I've spent most of my career).
 */

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

// Useful patterns to printf a single byte out console.
// use: printf("my byte: "BYTE_TO_BINARY_PATTERN" here\n", BYTE_TO_BINARY(my_byte));
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & (1 << 7) ? '1' : '0'), \
  ((byte) & (1 << 6) ? '1' : '0'), \
  ((byte) & (1 << 5) ? '1' : '0'), \
  ((byte) & (1 << 4) ? '1' : '0'), \
  ((byte) & (1 << 3) ? '1' : '0'), \
  ((byte) & (1 << 2) ? '1' : '0'), \
  ((byte) & (1 << 1) ? '1' : '0'), \
  ((byte) & (1 << 0) ? '1' : '0') 

/**
 * Challenge: write a function to retrieve a bit in a byte.
 * Note: This isn't really useful in production. This approach does extra work of getting the 
 * specific bit over to the "1"s decimal point place. For most scenarios, this is unnecessary
 * and just wastes operations. I could see an argument being made for readability though.
 * 
 * If you had a 1 byte bitmap of flags and "FOO" was the first bit, and "BAR" was the second
 * bit, you would typically access them with some kind of pattern like: 
 * `if ((my_data & FOO_MASK) == FOO_MASK)`, where "FOO_MASK" is `#define FOO_MASK 1`. That is,
 * it is the first bit in the bitmask.
 */
static inline int get_bit(uint8_t bitmap, uint8_t index) {
    return ((bitmap & (1 << index)) >> index);
}

/**
 * Challenge: write a function to set a bit high in a byte.
 * Note: Same caveat as above in `get_bit`. You'd probably be better off just using:
 * `my_data = my_data | FOO_MASK;` and saving the shifting operation.
 */
static inline void set_bit_high(uint8_t* bitmap, uint8_t index) {
    *bitmap = *bitmap | (1 << index);
}

/**
 * Challenge: write a function to set a bit low in a byte.
 * Note: Same caveat as above in `get_bit`. You'd probably be better off just using:
 * `my_data = my_data | FOO_MASK;` and saving the shifting operation.
 */
static inline void set_bit_low(uint8_t* bitmap, uint8_t index) {
    *bitmap &= (~(1 << index));
/*
// Even after ~8 years of professional experience, I can't write something like this in 1 shot, I have 
// to iterate through it and often make many silly mistakes. First pass, I wrote "set_bit_high" first and 
// figured it should look somewhat similar in structure, but would probably involve an inverted mask 
// of all bits set high except the index:
*bitmap = *bitmap | (0xFF & (1 << index)) // doesn't work

// I printed each step one at a time with a couple numbers to identify and workthrough problems
printf("(1 << index)"BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY((1 << index)));
printf("(0xFF & (1 << index))"BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY((0xFF & (1 << index)));
printf("(0xFF ^ (1 << index))"BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY((0xFF ^ (1 << index)));
// Once i had the inverted bitmap figured out, i realized the OR needed to be an AND and had a good enough solution.
*bitmap = *bitmap & (0xFF ^ (1 << index))

// Then thinking about it a bit, the idea of "inverting" binary made me think of the "not" operand
// which would allow getting rid of a constant.
*bitmap = *bitmap & (~(1 << index));
// code golfing a bit, leads to the following (I'd stop at the above in a production code base, but 
// this is for fun, so why not).
*bitmap &= (~(1 << index));
*/
}

void print_bitmap(uint8_t bitmap) {
    printf("0b");
    for(int i = 7; i >= 0; i--) {
        printf("%d", (bitmap & (1 << i)) >> i );
    }
    printf("\n");
}

/**
 * Challenge: write a "popcount" function.
 * Note: In a production environment, I'd argue to use a compiler intrinsic like 
 * gcc's `__builtin_popcount`. It uses a hardawre instruction when available and 
 * falls back to a (nightmarishly optimized) look up table when not available.
 * 
 * A first naive approach would be to move each bit to the "1"s decimal place and
 * check if that bit is high or low for each bit in the bitmap.
 */
int get_popcount(uint8_t bitmap) {
    int result = 0;
    for (int i = 0; i < 8; i++) {
        if ((bitmap & (1 << i)) >> i) {
            result += 1;
        }
    }
    return result;
}

/**
 * The above naive solution does some extra work moving the bit being checked to the 1s place.
 * In this solution, by shifting `i` one to the left each iteration, we can use it as a mask 
 * and skip all the work done above to move the current working bit to the ones 1s place.
 */
int get_popcount2(uint8_t bitmap) {
    int result = 0;
    for (int i = 1; i <= 256; i = i << 1) {
        if ((bitmap & i) == i) {
            result += 1;
        }
    }
    return result;
}

// Could go further with loop unrolling, look up tables, finding ways to avoid the branching, etc.
// Calling this good enough here for now.

void assert_test_popcounts(uint8_t bitmap, int expected) {
    printf("popcount tests: "BYTE_TO_BINARY_PATTERN" == %d\n", BYTE_TO_BINARY(bitmap), expected);
    assert(get_popcount(bitmap) == expected);
    assert(get_popcount2(bitmap) == expected);
}

void test_popcount() {
    assert_test_popcounts(0b00000000, 0);
    assert_test_popcounts(0b00000001, 1);
    assert_test_popcounts(0b10000000, 1);
    assert_test_popcounts(0b00100010, 2);
}

int main(char* argc, char* argv[]) {
    printf("Starting bit flag sandbox...\n");
    uint8_t test1 = 7;
    printf("test: "BYTE_TO_BINARY_PATTERN" here\n", BYTE_TO_BINARY(test1));
    uint8_t bitmap = 8;
    print_bitmap(bitmap);
    printf("popcount: %d\n", get_popcount(bitmap));
    set_bit_high(&bitmap, 0);
    print_bitmap(bitmap);
    printf("popcount: %d\n", get_popcount(bitmap));
    set_bit_high(&bitmap, 0);
    print_bitmap(bitmap);
    printf("popcount: %d\n", get_popcount(bitmap));

    set_bit_low(&bitmap, 0);
    print_bitmap(bitmap);
    printf("popcount: %d\n", get_popcount(bitmap));
    set_bit_low(&bitmap, 3);
    print_bitmap(bitmap);
    printf("popcount: %d\n", get_popcount(bitmap));

    printf("running popcount tests...\n");
    test_popcount();
    printf("Exiting bit flag sandbox...\n");
}
