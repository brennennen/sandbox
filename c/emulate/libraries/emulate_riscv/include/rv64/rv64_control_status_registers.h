
#ifndef RV64_CONTROL_STATUS_REGISTERS_H
#define RV64_CONTROL_STATUS_REGISTERS_H

#include <stdint.h>

//#include "rv64/rv64_emulate.h"

typedef struct emulator_rv64_s emulator_rv64_t; // forward declare

typedef struct {
    /*
     * MARK: Unprivileged
     */
    // float
    uint64_t fflags; // Floating-Point Accrued Exceptions
    uint64_t frm;    // Floating-Point Dynamic Rounding Mode
    uint64_t fcsr;   // Floating-Point Control and Status Register (fflags + frm)
    // vector
    uint64_t vstart; // Vector start position
    uint64_t vxsat; // Fixed-point accrued saturation flag
    uint64_t vxrm; // Fixed-point rounding mode
    uint64_t vcsr; // Vector control and status register
    uint64_t vl; // Vector length
    uint64_t vtype; // Vector data type register
    uint64_t vlenb; // Vector register length in bytes
    // Zicfiss - shadow stack
    uint64_t ssp; // Shadow stack pointer
    // Unprivileged Entropy Source Extension
    uint64_t seed; // Seed for cryptographic random bit generators
    // Zcmt
    uint64_t jvt; // Table jump base vector and control register
    // Unprivileged Counter/Timers
    uint64_t cycle;
    uint64_t time;
    uint64_t instret;
    uint64_t hpmcounter3;
    uint64_t hpmcounter4;
    uint64_t hpmcounter5;
    uint64_t hpmcounter6;
    uint64_t hpmcounter7;
    uint64_t hpmcounter8;
    uint64_t hpmcounter9;
    uint64_t hpmcounter10;
    uint64_t hpmcounter11;
    uint64_t hpmcounter12;
    uint64_t hpmcounter13;
    uint64_t hpmcounter14;
    uint64_t hpmcounter15;
    uint64_t hpmcounter16;
    uint64_t hpmcounter17;
    uint64_t hpmcounter18;
    uint64_t hpmcounter19;
    uint64_t hpmcounter20;
    uint64_t hpmcounter21;
    uint64_t hpmcounter22;
    uint64_t hpmcounter23;
    uint64_t hpmcounter24;
    uint64_t hpmcounter25;
    uint64_t hpmcounter26;
    uint64_t hpmcounter27;
    uint64_t hpmcounter28;
    uint64_t hpmcounter29;
    uint64_t hpmcounter30;
    uint64_t hpmcounter31;

    /*
     * MARK: Supervisor
     */
    // Supervisor Trap Setup
    uint64_t sstatus; // Supervisor status register
    uint64_t sie; // Supervisor interrupt-enable register
    uint64_t stvec; // Supervisor trap handler base address
    uint64_t scounteren; // Supervisor counter enable
    // Supervisor Configuration
    uint64_t senvcfg; // Supervisor environment configuration register
    // Supervisor Counter Setup
    uint64_t scountinhibit; // Supervisor counter inhibit register
    // Supervisor Trap Handling
    uint64_t sscratch; // Supervisor scratch register
    uint64_t sepc; // Supervisor exception program counter
    uint64_t scause; // Supervisor trap cause
    uint64_t stval; // Supervisor trap value
    uint64_t sip; // Supervisor interrupt pending
    uint64_t scountovf; // Supervisor count overflow
    // ...
    // hypervisor and vs
    // ...
    /*
     * MARK: Machine
     */
    // Machine Information Registers
    uint32_t mvendorid; // Vendor ID (32 bits, not xlen) (1 less than JEDEC vendor id)
    uint64_t marchid; // Architecture ID
    uint64_t mimpid; // Implementation ID
    uint64_t mhartid; // Hardware Thread ID
    uint64_t mconfigptr; // Pointer to configuration data structure (@see https://riscv.github.io/riscv-isa-manual/snapshot/privileged/#_machine_configuration_pointer_mconfigptr_register)
    // Machine Trap Setup
    uint64_t mstatus; // Machine status register
    uint64_t misa; // ISA and extensions
    uint64_t medeleg; // Machine exception delegation register.
    uint64_t mideleg; // Machine interrupt delegation register.
    uint64_t mie; // Machine interrupt-enable register.
    uint64_t mtvec; // Machine trap-handler base address.
    uint64_t mcounteren; // Machine counter enable.
    // Machine Counter Configuration
    uint64_t mcyclecfg; // Machine cylce counter configuration register.
    uint64_t minstretcfg; // Machine instret counter configuration register.
    // Machine Trap Handling
    uint64_t mscratch; // Machine scratch register.
    uint64_t mepc; // Machine exception program counter.
    uint64_t mcause; // Machine trap cause.
    uint64_t mtval; // Machine trap value.
    uint64_t mip; // Machine interrupt pending.
    uint64_t mtinst; // Machine trap instruction (transformed).
    uint64_t mtval2; // Machine second trap value.
    // Machine Configuration
    uint64_t menvcfg; // Machine environment configuration register.
    uint64_t mseccfg; // Machine security configuration register.
    // Machine Memory Protection
    // ...
} rv64_csrs_t;

char* rv64_csr_name_from_address(int32_t address);

uint64_t rv64_get_csr_value(rv64_csrs_t* csrs, int32_t address);

void rv64_set_csr_value(rv64_csrs_t* csrs, int32_t address, uint64_t value);


typedef enum {
    RV_MXL_32 = 0,
    RV_MXL_64,
    RV_MXL_RESERVED1
} rv_mxl_t;

typedef enum {
    RV_EXTENSION_A_ATOMIC = 0,
    RV_EXTENSION_B,
    RV_EXTENSION_C,
    RV_EXTENSION_D,
    RV_EXTENSION_E,
    RV_EXTENSION_F,
    RV_EXTENSION_G,
    RV_EXTENSION_H,
    RV_EXTENSION_I,
    RV_EXTENSION_J,
    RV_EXTENSION_K,
    RV_EXTENSION_L,
    RV_EXTENSION_M,
    RV_EXTENSION_N,
    RV_EXTENSION_O,
    RV_EXTENSION_P,
    RV_EXTENSION_Q,
    RV_EXTENSION_R,
    RV_EXTENSION_S,
    RV_EXTENSION_T,
    RV_EXTENSION_U,
    RV_EXTENSION_V,
    RV_EXTENSION_W,
    RV_EXTENSION_X,
    RV_EXTENSION_Y,
    RV_EXTENSION_Z
} rv_extensions_t;

/*
 * Set initial data functions (data that is set at init and read-only after)
 */
void rv64_csr_set_initial_misa(rv64_csrs_t* csrs, rv_mxl_t mxl, uint32_t extensions);
void rv64_csr_set_initial_mvendorid(rv64_csrs_t* csrs, uint32_t vendor_id);
void rv64_csr_set_initial_marchid(rv64_csrs_t* csrs, uint64_t arch_id);
void rv64_csr_set_initial_mimpid(rv64_csrs_t* csrs, uint64_t mimpid);
void rv64_csr_set_initial_mhartid(rv64_csrs_t* csrs, uint64_t mhartid);
void rv64_csr_set_initial_mconfigptr(rv64_csrs_t* csrs, uint64_t mconfigptr);

/*
 * Set functions (these can probably be static? not even in the header file?)
 */
void rv64_csr_set_misa(uint64_t* misa_csr, uint64_t misa);

// get functions?


#endif // RV64_CONTROL_STATUS_REGISTERS_H
