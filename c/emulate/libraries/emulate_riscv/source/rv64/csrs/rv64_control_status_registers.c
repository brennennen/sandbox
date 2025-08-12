
#include "rv64/rv64_control_status_registers.h"


// void write_csrs(emulator_rv64_t* emulator, uint8_t *data, int32_t data_size) {
//     data[0x001] = emulator.csrs.float_csrs.fflags;
//     // TODO
// }


/**
 *
 * @see 3.1.1 Machine ISA (`misa`) Register https://riscv.github.io/riscv-isa-manual/snapshot/privileged/#misa
 */
uint64_t rv64_build_misa() {
    // TODO: need some data set on the emulator? cyclic dependency
    return 0;
}

uint64_t rv64_get_csr_value(rv64_control_status_registers_t* csrs, int32_t address) {
    switch(address) {
        // Unsupervised Floats
        case 0x001: return(csrs->float_csrs.fflags);
        case 0x002: return(csrs->float_csrs.frm);
        case 0x003: return(csrs->float_csrs.fcsr);
        // Unprivileged Vector CSRs
        case 0x008: return(csrs->vector_csrs.vstart);
        case 0x009: return(csrs->vector_csrs.vxsat);
        case 0x00A: return(csrs->vector_csrs.vxrm);
        case 0x00F: return(csrs->vector_csrs.vcsr);
        case 0xc20: return(csrs->vector_csrs.vl);
        case 0xc21: return(csrs->vector_csrs.vtype);
        case 0xc22: return(csrs->vector_csrs.vlenb);
        // Unprivileged Zicfiss
        case 0x011: return(csrs->shadow_stack_csrs.ssp);
        // Unprivileged seed for cryptographic random bit generators
        case 0x015: return(csrs->entropy_source_csrs.seed);
        // ...
        default: {
            // todo: log error
            return(0);
        }
    }
}

void rv64_set_csr_value(rv64_control_status_registers_t* csrs, int32_t address, uint64_t value) {
    switch(address) {
        // Unsupervised Floats
        case 0x001: {
            csrs->float_csrs.fflags = value;
            break;
        }
        case 0x002: {
            csrs->float_csrs.frm = value;
            break;
        }
        case 0x003: {
            csrs->float_csrs.fcsr = value;
            break;
        }
        default: {
            // todo: log error
        }
    }
}
