
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

uint64_t rv64_get_csr_value(rv64_csrs_t* csrs, int32_t address) {
    switch(address) {
        /*
         * MARK: Unsupervised
         */
        case 0x001: return(csrs->fflags);
        case 0x002: return(csrs->frm);
        case 0x003: return(csrs->fcsr);
        // Unprivileged Vector CSRs
        case 0x008: return(csrs->vstart);
        case 0x009: return(csrs->vxsat);
        case 0x00A: return(csrs->vxrm);
        case 0x00F: return(csrs->vcsr);
        case 0xc20: return(csrs->vl);
        case 0xc21: return(csrs->vtype);
        case 0xc22: return(csrs->vlenb);
        // Unprivileged Zicfiss
        case 0x011: return(csrs->ssp);
        // Unprivileged seed for cryptographic random bit generators
        case 0x015: return(csrs->seed);
        // ...
        /*
         * MARK: Supervisor
         */
        // ...
        /*
         * MARK: Hypervisor
         */
        // ...
        /*
         * MARK: Machine
         */
        // Machine Information Registers
        case 0xf11: return(csrs->mvendorid);
        case 0xf12: return(csrs->marchid);
        case 0xf13: return(csrs->mimpid);
        case 0xf14: return(csrs->mhartid);
        case 0xf15: return(csrs->mconfigptr);
        // Machine Trap Setup
        case 0x300: return(csrs->mstatus);
        case 0x301: return(csrs->misa);


        // ...
        default: {
            // todo: log error
            return(0);
        }
    }
}





/**
 * Set the "misa" (Machine ISA (Instruction Set Architecture)) csr.
 * WARL (Write any values, reads legal values)
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/privileged/#misa
 */
void rv64_csr_set_misa(uint64_t* misa_csr, uint64_t misa) {
    misa = misa & 0x1fffffffffffffff; // top 2 bits are read only and top 3rd bit is always 0.
    // TODO: prefer "I" if both "I" and "E" are set (set I, clear E).
    // TODO: there are other requirements based on feature dependencies, see spec 3.1.1
    *misa_csr = misa;
}

void rv64_set_csr_value(rv64_csrs_t* csrs, int32_t address, uint64_t value) {
    switch(address) {
        /*
         * MARK: Unsupervised
         */
        case 0x001: {
            csrs->fflags = value;
            break;
        }
        case 0x002: {
            csrs->frm = value;
            break;
        }
        case 0x003: {
            csrs->fcsr = value;
            break;
        }
        /*
         * MARK: Supervisor
         */
        // ...
        /*
         * MARK: Hypervisor
         */
        // ...
        /*
         * MARK: Machine
         */
        // Machine Infromation Registers
        // mvendorid, marchid, mimpid, mhartid, and mconfigptr are all read only.
        // Machine Trap Setup
        case 0x300: {
            csrs->mstatus = value;
            break;
        }
        case 0x301: {
            rv64_csr_set_misa(&csrs->misa, value);
            break;
        }

        // ...
        default: {
            // todo: log error
        }
    }
}


/*
 * Set initial (mostly read only) data
 */

void rv64_csr_set_initial_misa(rv64_csrs_t* csrs, rv_mxl_t mxl, uint32_t extensions) {
    uint64_t initial_misa = 0;
    initial_misa = (uint64_t)mxl << 62; // TODO: dynamically shift this or stick to rv64 only?
    initial_misa |= (uint64_t)extensions;
    // TODO: prefer "I" if both "I" and "E" are set (set I, clear E).
    // TODO: there are other requirements based on feature dependencies, see spec 3.1.1
    csrs->misa = initial_misa;
}

void rv64_csr_set_initial_mvendorid(rv64_csrs_t* csrs, uint32_t mvendorid) {
    csrs->mvendorid = mvendorid;
}

void rv64_csr_set_initial_marchid(rv64_csrs_t* csrs, uint64_t marchid) {
    csrs->marchid = marchid;
}

void rv64_csr_set_initial_mimpid(rv64_csrs_t* csrs, uint64_t mimpid) {
    csrs->mimpid = mimpid;
}

void rv64_csr_set_initial_mhartid(rv64_csrs_t* csrs, uint64_t mhartid) {
    csrs->mhartid = mhartid;
}

void rv64_csr_set_initial_mconfigptr(rv64_csrs_t* csrs, uint64_t mconfigptr) {
    csrs->mconfigptr = mconfigptr;
}
