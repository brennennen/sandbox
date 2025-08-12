
#ifndef RV64_CONTROL_STATUS_REGISTERS_H
#define RV64_CONTROL_STATUS_REGISTERS_H

#include <stdint.h>

//#include "rv64/rv64_emulate.h"

/*
 * MARK: Unprivileged
 */

// float
typedef struct {
    uint64_t fflags; // Floating-Point Accrued Exceptions
    uint64_t frm;    // Floating-Point Dynamic Rounding Mode
    uint64_t fcsr;   // Floating-Point Control and Status Register (fflags + frm)
} rv64_float_csrs_t;

// vector
typedef struct {
    uint64_t vstart; // Vector start position
    uint64_t vxsat; // Fixed-point accrued saturation flag
    uint64_t vxrm; // Fixed-point rounding mode
    uint64_t vcsr; // Vector control and status register
    uint64_t vl; // Vector length
    uint64_t vtype; // Vector data type register
    uint64_t vlenb; // Vector register length in bytes
} rv64_vector_csrs_t;

// Zicfiss
typedef struct {
    uint64_t ssp; // Shadow stack pointer
} rv64_shadow_stack_csrs_t;

// Unprivileged Entropy Source Extension
typedef struct {
    uint64_t seed; // Seed for cryptographic random bit generators
} rv64_entropy_source_csrs_t;

// Zcmt
typedef struct {
    uint64_t jvt; // Table jump base vector and control register
} rv64_zcmt_csrs_t;

// Unprivileged Counter/Timers
typedef struct {
    uint64_t cycle;
    uint64_t time;
    uint64_t instret;
    uint64_t hpmcounter3;
    uint64_t hpmcounter4;
    // ... todo: 5 - 30
    uint64_t hpmcounter31;
    // the rest are rv32 only and skipped for rv64
} rv64_counters_csrs_t;

/*
 * MARK: Supervisor
 */

// Supervisor Trap Setup
typedef struct {
    uint64_t sstatus; // Supervisor status register
    uint64_t sie; // Supervisor interrupt-enable register
    uint64_t stvec; // Supervisor trap handler base address
    uint64_t scounteren; // Supervisor counter enable
} rv64_supervisor_trap_csrs_t;

// Supervisor Configuration
typedef struct {
    uint64_t senvcfg; // Supervisor environment configuration register
} rv64_supervisor_configuration_csrs_t;

// Supervisor Counter Setup
typedef struct {
    uint64_t scountinhibit; // Supervisor counter inhibit register
} rv64_supervisor_counter_setup_csrs_t;

// Supervisor Trap Handling
typedef struct {
    uint64_t sscratch; // Supervisor scratch register
    uint64_t sepc; // Supervisor exception program counter
    uint64_t scause; // Supervisor trap cause
    uint64_t stval; // Supervisor trap value
    uint64_t sip; // Supervisor interrupt pending
    uint64_t scountovf; // Supervisor count overflow
} rv64_supervisor_trap_handling_csrs_t;



/*
 * MARK: Main CSR
 */

typedef struct {
    // unprivileged
    rv64_float_csrs_t float_csrs;
    rv64_vector_csrs_t vector_csrs;
    rv64_shadow_stack_csrs_t shadow_stack_csrs;
    rv64_entropy_source_csrs_t entropy_source_csrs;
    rv64_zcmt_csrs_t zcmt_csrs;
    rv64_counters_csrs_t counters_csrs;
    // supervisor
    rv64_supervisor_trap_csrs_t supervisor_trap_csrs;
    rv64_supervisor_configuration_csrs_t supervisor_configuration_csrs;
    rv64_supervisor_counter_setup_csrs_t supervisor_counter_setup_csrs;
    rv64_supervisor_trap_handling_csrs_t supervisor_trap_handling_csrs;
    // ...
    // hypervisor and vs
    // ...
    // machine-level
    // ...


} rv64_control_status_registers_t;

// void write_csrs(emulator_rv64_t* emulator, uint8_t *data, int32_t data_size); // 4k memory chunk

// TODO: map addresses to names
char* rv64_csr_name_from_address(int32_t address);

uint64_t rv64_get_csr_value(rv64_control_status_registers_t* csrs, int32_t address);

void rv64_set_csr_value(rv64_control_status_registers_t* csrs, int32_t address, uint64_t value);

#endif // RV64_CONTROL_STATUS_REGISTERS_H
