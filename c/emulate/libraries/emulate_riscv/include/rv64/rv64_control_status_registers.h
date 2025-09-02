
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
    // Supervisor Indirect
    uint64_t siselect; // Supervisor indirect register select
    uint64_t sireg; // Supervisor indirect register alias
    // ...
    uint64_t sireg6;
    // Supervisor Protection and Translation
    uint64_t satp;
    // Supervisor Timer Compare
    uint64_t stimecmp; // Supervisor timer compare
    // Debug/Trace Registers
    uint64_t scontext; // Supervisor-mode context register
    // Supervisor Resource Management Configuration
    uint64_t srmcfg; // Supervisor Resource Management Configuration
    // Supervisor State Enable Registers
    uint64_t sstateen0; // Supervisor State Enable 0 Register
    uint64_t sstateen1;
    uint64_t sstateen2;
    uint64_t sstateen3;
    // Supervisor Control Transfer Records Configuration
    uint64_t sctrctl; // Supervisor Control Transfer Records Control Register
    uint64_t sctrstatus; // Supervisor Control Transfer Records Status Register
    uint64_t sctrdepth; // Supervisor Control Transfer Records Depth Register

    /*
     * MARK: Hypervisor and VS
     */
    // Hypervisor Trap Setup
    uint64_t hstatus; // Hypervisor status register
    uint64_t hedeleg; // Hypervisor exception delegation register
    uint64_t hideleg; // Hypervisor interrupt delegation register
    uint64_t hie; // Hypervisor interrupt-enable register
    uint64_t hcountren; // Hypervisor counter enable
    uint64_t hgeie; // Hypervisor guest external interrupt-enable register
    // Hypervisor Trap Handling
    uint64_t htval; // Hypervisor trap value
    uint64_t hip; // Hypervisor interrupt pending
    uint64_t hvip; // Hypervisor virtual interrupt pending
    uint64_t htinst; // Hypervisor trap instruction (transformed)
    uint64_t hgeip; // Hypervisor guest external interrupt pending
    // Hypervisor Configuration
    uint64_t henvcfg; // Hypervisor environment configuration register
    // Hypervisor Protection and Translation
    uint64_t hgatp; // Hypervisor guest address translation and protection
    // Debug/Trace Registers
    uint64_t hcontext; // Hypervisor-mode context register
    // Hypervisor Counter/Timer Virtualization Registers
    uint64_t htimedelta; // Delta for VS/VU-mode timer
    // Hypervisor State Enable Registers
    uint64_t hstateen0; // Hypervisor State Enalbe 0 Register
    uint64_t hstateen1;
    uint64_t hstateen2;
    uint64_t hstateen3;
    // Virtual Supervisor Registers
    uint64_t vsstatus; // Virtual supervisor status register
    uint64_t vsie; // Virtual supervisor interrupt-enable register
    uint64_t vstvec; // Virtual supervisor trap handler base address
    uint64_t vsscratch; // Virtual supervisor scratch register
    uint64_t vsepc; // Virtual supervisor exception program counter
    uint64_t vscause; // Virtual supervisor trap cause
    uint64_t vstval; // Virtual supervisor trap value
    uint64_t vsip; // Virtual supervisor interrupt pending
    uint64_t vsatp; // Virtual supervisor address translation and protection
    // Virtual Supervisor Indirect
    uint64_t vsiselect; // Virtual supervisor indirect register select
    uint64_t vsireg; // Virtual supervisor indirect register alias
    uint64_t vsireg2;
    uint64_t vsireg3;
    uint64_t vsireg4;
    uint64_t vsireg5;
    uint64_t vsireg6;
    // Virtual Supervisor Timer Compare
    uint64_t vstimecmp; // Virtual supervisor timer compare
    // Virtual Supervisor Control Transfer Records Configuration
    uint64_t vsctrctl; // Virtual Supervisor Control Transfer Records Control Register

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
    uint64_t medeleg; // Machine exception delegation register
    uint64_t mideleg; // Machine interrupt delegation register
    uint64_t mie; // Machine interrupt-enable register
    uint64_t mtvec; // Machine trap-handler base address
    uint64_t mcounteren; // Machine counter enable
    // Machine Counter Configuration
    uint64_t mcyclecfg; // Machine cylce counter configuration register
    uint64_t minstretcfg; // Machine instret counter configuration register
    // Machine Trap Handling
    uint64_t mscratch; // Machine scratch register
    uint64_t mepc; // Machine exception program counter
    uint64_t mcause; // Machine trap cause
    uint64_t mtval; // Machine trap value
    uint64_t mip; // Machine interrupt pending
    uint64_t mtinst; // Machine trap instruction (transformed)
    uint64_t mtval2; // Machine second trap value
    // Machine Indirect
    uint64_t miselect; // Machine indirect register select
    uint64_t mireg; // Machine indirect register alias
    uint64_t mireg2;
    uint64_t mireg3;
    uint64_t mireg4;
    uint64_t mireg5;
    uint64_t mireg6;
    // Machine Configuration
    uint64_t menvcfg; // Machine environment configuration register
    uint64_t mseccfg; // Machine security configuration register
    // Machine Memory Protection
    uint64_t pmpcfg0; // Physical memory protection configuration
    uint64_t pmpcfg2;
    // todo: 4 - 12. odds are for rv32
    uint64_t pmpcfg14;
    uint64_t pmpaddr0; // Physical memory protection address register
    uint64_t pmpaddr1;
    // todo: 2 - 62
    uint64_t pmpaddr63;
    // Machine State Enable Registers
    uint64_t mstateen0;
    uint64_t mstateen1;
    uint64_t mstateen2;
    uint64_t mstateen3;
    // Machine Non-Maskable Interrupt Handling
    uint64_t mnscratch; // Resumable NMI scratch register
    uint64_t mnepc; // Resumable NMI program counter
    uint64_t mncause; // Resumable NMI cause
    uint64_t mnstatus; // Resumable NMI status
    // Machine Counter/Timers
    uint64_t mcycle; // Machine cycle counter
    uint64_t minstret; // Machine instructions-retired counter
    uint64_t mhpmcounter3; // Machine performance-monitoring counter
    // todo: 4 - 30
    uint64_t mhpmcounter31;
    // Machine Counter Setup
    uint64_t mcountinhibit; // Machine counter-inhibit register
    uint64_t mhpmevent3; // Machine performance-monitoring event selector
    // todo: 4 - 30
    uint64_t mhpmevent31;
    // Machine Control Transfer Records Configuration
    uint64_t mctrctl; // Machine Control Transfer Records Control Register
    // Debug/Trace Registers (shared with Debug Mode)
    uint64_t tselect; // Debug/Trace trigger register select
    uint64_t tdata1; // First Debug/Trace trigger data register
    uint64_t tdata2; // Second Debug/Trace trigger data register
    uint64_t tdata3; // Third Debug/Trace trigger data register
    uint64_t mcontext; // Machine-mode context register
    // Debug Mode Registers
    uint64_t dcsr; // Debug control and status register
    uint64_t dpc; // Debug program counter
    uint64_t dscratch0; // Debug scratch register 0
    uint64_t dscratch1; // Debug scratch register 1
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

typedef struct {
    uint8_t sd; // Dirty Floating-Point State
    uint8_t mdt;
    uint8_t mpelp;
    uint8_t mpv;
    uint8_t gva;
    uint8_t mbe;
    uint8_t sbe;
    uint8_t sxl;
    uint8_t uxl;
    uint8_t sdt;
    uint8_t spelp;
    uint8_t tsr;
    uint8_t tw;
    uint8_t tvm;
    uint8_t mxr;
    uint8_t sum;
    uint8_t mprv;
    uint8_t xs;
    uint8_t fs;
    uint8_t mpp;
    uint8_t vs;
    uint8_t spp;
    uint8_t mpie;
    uint8_t ube;
    uint8_t spie;
    uint8_t mie;
    uint8_t sie;
} rv64_mstatus_t;

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

// get functions?



// vector stuff

typedef enum {
    RV64_SEW_8,
    RV64_SEW_16,
    RV64_SEW_32,
    RV64_SEW_64
} rv64v_selected_element_width_t;



// TODO: decode vtypei?

// uint8_t vma = (vtypei >> 7) & 0b1;
// uint8_t vta = (vtypei >> 6) & 0b1;
// uint8_t vsew = (vtypei >> 3) & 0b11;
// uint8_t vlmul = vtypei & 0b11;
typedef struct {
    uint8_t vma;
    uint8_t vta;
    rv64v_selected_element_width_t selected_element_width;
    uint8_t vlmul;
} rv64v_vtype_t;
rv64v_vtype_t rv64_csr_decode_vtype(uint64_t vtype_raw);


#endif // RV64_CONTROL_STATUS_REGISTERS_H
