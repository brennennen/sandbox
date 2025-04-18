
#ifndef BINARY_UTILS_H
#define BINARY_UTILS_H

#include <stdio.h>
// GCC-14 bug. Release note says that with adding "-std=c23", __STDC_VERSION__
// is set to 202311L. __STDC_VERSION__ is "202000L".
//#if __STDC_VERSION__ >= 202311L
#if __STDC_VERSION__ >= 202000L
#include <stdbit.h>
#endif

// Useful pattern for printing out a byte in a binary representation.
// Usage: `printf("byte: "BYTE_TO_BINARY_PATTERN"\n", BYTE_TO_BINARY(byte));`
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
    ((byte) & 0x80 ? '1' : '0'), \
    ((byte) & 0x40 ? '1' : '0'), \
    ((byte) & 0x20 ? '1' : '0'), \
    ((byte) & 0x10 ? '1' : '0'), \
    ((byte) & 0x08 ? '1' : '0'), \
    ((byte) & 0x04 ? '1' : '0'), \
    ((byte) & 0x02 ? '1' : '0'), \
    ((byte) & 0x01 ? '1' : '0')

//#define ENABLE_ENUM_PACK_ATTRIBUTE
#ifdef ENABLE_ENUM_PACK_ATTRIBUTE
#define ENUM_PACK_ATTRIBUTE __attribute__((__packed__))
#else
#define ENUM_PACK_ATTRIBUTE
#endif

//#define ENABLE_PACK_ATTRIBUTE
#ifdef ENABLE_PACK_ATTRIBUTE
#define PACK_ATTRIBUTE __attribute__((__packed__))
#else
#define PACK_ATTRIBUTE
#endif

// TODO define a compiler agnostic popcount?
#ifndef pop_count
    // GCC-14 bug. Release note says that with adding "-std=c23", __STDC_VERSION__
    // is set to 202311L. __STDC_VERSION__ is "202000L".
    //#if __STDC_VERSION__ >= 202311L
    #if __STDC_VERSION__ >= 202000L
        #define pop_count_uint16(x) stdc_count_ones_us(x)
    #else
        #define pop_count_uint16(x) pop_count_uint16_fallback(x)
    #endif
#endif




#endif // BINARY_UTILS_H
