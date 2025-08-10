
#ifndef RV64_CONTROL_STATUS_REGISTERS_H
#define RV64_CONTROL_STATUS_REGISTERS_H

#include <stdint.h>

//#include "rv64/rv64_emulate.h"

typedef struct {
    uint64_t fflags; // Floating-Point Accrued Exceptions
    uint64_t frm;    // Floating-Point Dynamic Rounding Mode
    uint64_t fcsr;   // Floating-Point Control and Status Register (fflags + frm)
} rv64_float_csrs_t;

typedef struct {
    uint64_t vstart; // Vector start position
    uint64_t vxsat; // Fixed-point accrued saturation flag
    uint64_t vxrm; // Fixed-point rounding mode
    uint64_t vcsr; // Vector control and status register
    uint64_t vl; // Vector length
    uint64_t vtype; // Vector data type register
    uint64_t vlenb; // Vector register length in bytes
} rv64_vector_csrs_t;

/**
 * Zicfiss
 */
typedef struct {
    uint64_t ssp; // Shadow stack pointer
} rv64_shadow_stack_csrs_t;

typedef struct {
    uint64_t seed; // Seed for cryptographic random bit generators.
} rv64_entropy_source_csrs_t;

// typedef struct {
//     uint8_t
// } rv64_zcmt_csrs_t;

// typedef struct {

// } rv64_counters_csrs_t;

typedef struct {
    rv64_float_csrs_t float_csrs;
    rv64_vector_csrs_t vector_csrs;
    //rv64_shadow_stack_csrs_t shadow_stack_csrs;
    //rv64_entropy_source_csrs_t entropy_source_csrs;
} rv64_control_status_registers_t;

// void write_csrs(emulator_rv64_t* emulator, uint8_t *data, int32_t data_size); // 4k memory chunk

// TODO: map addresses to names
char* rv64_get_csr_name(int32_t address);

uint64_t rv64_get_csr_value(rv64_control_status_registers_t* csrs, int32_t address);

void rv64_set_csr_value(rv64_control_status_registers_t* csrs, int32_t address, uint64_t value);

#endif // RV64_CONTROL_STATUS_REGISTERS_H
