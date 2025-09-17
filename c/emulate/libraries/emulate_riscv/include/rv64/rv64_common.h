#ifndef RV64_COMMON_H
#define RV64_COMMON_H

typedef enum {
    RV64F_WIDTH_32 = 0, // single
    RV64F_WIDTH_64 = 1, // double
    RV64F_WIDTH_16 = 2, // half
    RV64F_WIDTH_128 = 3 // quad
} rv64f_width_t;


#endif // RV64_COMMON_H
