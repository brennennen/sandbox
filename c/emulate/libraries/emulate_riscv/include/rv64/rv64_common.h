#ifndef RV64_COMMON_H
#define RV64_COMMON_H

typedef enum {
    RV64F_WIDTH_16 = 1, // half
    RV64F_WIDTH_32 = 2, // word
    RV64F_WIDTH_64 = 3, // double
    RV64F_WIDTH_128 = 4 // quad
} rv64f_width_t;

#endif // RV64_COMMON_H
