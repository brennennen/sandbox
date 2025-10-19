
#include <stdio.h>

#include "rv64/rv64_common.h"
#include "rv64/rv64_control_status_registers.h"






// void write_csrs(rv64_emulator_t* emulator, uint8_t *data, int32_t data_size) {
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
        case RV64_CSR_FFLAGS: return(csrs->fflags);
        case RV64_CSR_FRM: return(csrs->frm);
        case RV64_CSR_FCSR: return(csrs->fcsr);
        // Unprivileged Vector CSRs
        case RV64_CSR_VSTART: return(csrs->vstart);
        case RV64_CSR_VXSAT: return(csrs->vxsat);
        case RV64_CSR_VXRM: return(csrs->vxrm);
        case RV64_CSR_VCSR: return(csrs->vcsr);
        case RV64_CSR_VL: return(csrs->vl);
        case RV64_CSR_VTYPE: return(csrs->vtype);
        case RV64_CSR_VLENB: return(csrs->vlenb);
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
        case RV64_CSR_MVENDORID: return(csrs->mvendorid);
        case RV64_CSR_MARCHID: return(csrs->marchid);
        case RV64_CSR_MIMPID: return(csrs->mimpid);
        case RV64_CSR_MHARID: return(csrs->mhartid);
        case RV64_CSR_MCONFIGPTR: return(csrs->mconfigptr);
        // Machine Trap Setup
        case RV64_CSR_MSTATUS: return(csrs->mstatus);
        case 0x301: return(csrs->misa);
        case 0x302: return(csrs->medeleg);
        case 0x303: return(csrs->mideleg);
        case 0x304: return(csrs->mie);
        case 0x305: return(csrs->mtvec);
        case 0x306: return(csrs->mcounteren);
        // Machine Counter Configuration
        case 0x321: return(csrs->mcyclecfg);
        case 0x322: return(csrs->minstretcfg);
        // Machine Trap Handling
        case 0x340: return(csrs->mscratch);
        case 0x341: return(csrs->mepc);
        case 0x342: return(csrs->mcause);
        case 0x343: return(csrs->mtval);
        case 0x344: return(csrs->mip);
        case 0x34a: return(csrs->mtinst);
        case 0x34b: return(csrs->mtval2);
        // Machine Indirect
        case 0x350: return(csrs->miselect);
        case 0x351: return(csrs->mireg);
        case 0x352: return(csrs->mireg2);
        case 0x353: return(csrs->mireg3);
        case 0x355: return(csrs->mireg4);
        case 0x356: return(csrs->mireg5);
        case 0x357: return(csrs->mireg6);
        // Machine Configuration
        case 0x30a: return(csrs->menvcfg);
        case 0x747: return(csrs->mseccfg);
        // Machine Memory Protection
        case 0x3a0: return(csrs->pmpcfg0);
        case 0x3a2: return(csrs->pmpcfg2);
        // todo: pmpcfg 4 - 12
        case 0x3ae: return(csrs->pmpcfg14);
        case 0x3b1: return(csrs->pmpaddr0);
        case 0x3b2: return(csrs->pmpaddr1);
        // todo: pmpaddr 2 - 62
        case 0x3ef: return(csrs->pmpaddr63);
        // Machine State Enable Registers
        case 0x30c: return(csrs->mstateen0);
        case 0x30d: return(csrs->mstateen1);
        case 0x30e: return(csrs->mstateen2);
        case 0x30f: return(csrs->mstateen3);
        // Machine Non-Maskable Interrupt Handling
        case 0x740: return(csrs->mnscratch);
        case 0x741: return(csrs->mnepc);
        case 0x742: return(csrs->mncause);
        case 0x744: return(csrs->mnstatus);
        // Machine Counter/Timers
        case 0xb00: return(csrs->mcycle);
        case 0xb02: return(csrs->minstret);
        case 0xb03: return(csrs->mhpmcounter3);
        // todo: mhpmevent 4 - 30
        case 0xb1f: return(csrs->mhpmcounter31);
        // Machine Counter Setup
        case 0x320: return(csrs->mcountinhibit);
        case 0x323: return(csrs->mhpmevent3);
        // todo: mhpmevent 4 - 30
        case 0x33f: return(csrs->mhpmevent31);
        // Machine Control Transfer Records Configuration
        case 0x34e: return(csrs->mctrctl);
        // Debug/Trace Registers
        case 0x7a0: return(csrs->tselect);
        case 0x7a1: return(csrs->tdata1);
        case 0x7a2: return(csrs->tdata2);
        case 0x7a3: return(csrs->tdata3);
        case 0x7a8: return(csrs->mcontext);
        // Debug Mode Registers
        case 0x7b0: return(csrs->dcsr);
        case 0x7b1: return(csrs->dpc);
        case 0x7b2: return(csrs->dscratch0);
        case 0x7b3: return(csrs->dscratch1);
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
static void rv64_csr_set_misa(uint64_t* misa_csr, uint64_t misa) {
    misa = misa & 0x1fffffffffffffff; // top 2 bits are read only and top 3rd bit is always 0.
    // TODO: prefer "I" if both "I" and "E" are set (set I, clear E).
    // TODO: there are other requirements based on feature dependencies, see spec 3.1.1
    *misa_csr = misa;
}



static void rv64_csr_set_mstatus2(uint64_t* mstatus_csr, rv64_mstatus_t mstatus) {
    //*mstatus_csr = 0;
    // todo
}

/**
 *
 * @see https://riscv.github.io/riscv-isa-manual/snapshot/privileged/#_machine_status_mstatus_and_mstatush_registers
 */
static void rv64_csr_set_mstatus(uint64_t* mstatus_csr, uint64_t mstatus) {
    *mstatus_csr = mstatus;
}

static void rv64_csr_set_mtvec(uint64_t* mtvec_csr, uint64_t mtvec) {
    // 63 - 2: base
    // 0 - 1: mode
    // mode: 0 = direct, 1 = vectored
    *mtvec_csr = mtvec;
}

void rv64_set_csr_value(rv64_csrs_t* csrs, int32_t address, uint64_t value) {
    switch(address) {
        /*
         * MARK: Unsupervised
         */
        case RV64_CSR_FFLAGS: { csrs->fflags = value; break; }
        case RV64_CSR_FRM: { csrs->frm = value; break; }
        case RV64_CSR_FCSR: { csrs->fcsr = value; break; }
        case RV64_CSR_VSTART: { csrs->vstart = value; break; }
        case RV64_CSR_VXSAT: { csrs->vxsat = value; break; }
        case RV64_CSR_VXRM: { csrs->vxrm = value; break; }
        case RV64_CSR_VCSR: { csrs->vcsr = value; break; }
        case RV64_CSR_VL: { csrs->vl = value; break; }
        case RV64_CSR_VTYPE: { csrs->vtype = value; break; }
        case RV64_CSR_VLENB: { csrs->vlenb = value; break; }
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
        // Machine Infromation Registers
        // mvendorid, marchid, mimpid, mhartid, and mconfigptr are all read only.
        // Machine Trap Setup
        case RV64_CSR_MSTATUS: {
            rv64_csr_set_mstatus(&csrs->mstatus, value);
            break;
        }
        case 0x301: {
            rv64_csr_set_misa(&csrs->misa, value);
            break;
        }
        // todo: medeleg, mideleg, mie
        case 0x305: {
            rv64_csr_set_mtvec(&csrs->mtvec, value);
            break;
        }

        // ...
        default: {
            // todo: log error
        }
    }
}

void rv64_csr_decode_mstatus(uint64_t mstatus_raw, rv64_mstatus_t* mstatus) {
    mstatus->vs = (mstatus_raw >> 9) & 0b11;
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

void rv64_csr_set_initial_frm(rv64_csrs_t* csrs, rv64f_rounding_mode_t rm) {
    uint64_t frm_clear_mask = ~(0b111 << 5);
    csrs->fcsr = (csrs->fcsr & ~frm_clear_mask) | ((uint64_t)rm << 5);
    csrs->frm = rm;
}


uint8_t rv64_encode_vmul(rv64v_vlmul_t vlmul) {
    switch(vlmul) {
        case RV64_VLMUL_1: return 0b000;
        case RV64_VLMUL_2: return 0b001;
        case RV64_VLMUL_4: return 0b010;
        case RV64_VLMUL_8: return 0b011;
        case RV64_VLMUL_HALF: return 0b111;
        case RV64_VLMUL_QUARTER: return 0b110;
        case RV64_VLMUL_EIGHTH: return 0b101;
        // all enum cases handled, no default needed.
    }
}

rv64v_vlmul_t rv64_decode_vmul(uint8_t raw_vlmul) {
    switch(raw_vlmul) {
        case 0b000: return RV64_VLMUL_1;
        case 0b001: return RV64_VLMUL_2;
        case 0b010: return RV64_VLMUL_4;
        case 0b011: return RV64_VLMUL_8;
        case 0b111: return RV64_VLMUL_HALF;
        case 0b110: return RV64_VLMUL_QUARTER;
        case 0b101: return RV64_VLMUL_EIGHTH;
        default: {
            printf("%s:invalid vlmul: %d\n", __func__, raw_vlmul);
            return RV64_VLMUL_1; // need to return something? todo: should set error flag or something though.
        }
    }
}

void rv64_csr_decode_vtype(uint64_t vtype_raw, rv64v_vtype_t* vtype) {
    vtype->vma = (vtype_raw >> 7) & 0b1;
    vtype->vta = (vtype_raw >> 6) & 0b1;
    vtype->selected_element_width = (vtype_raw >> 3) & 0b111;
    uint8_t vlmul_raw = vtype_raw & 0b111;
    vtype->vlmul = rv64_decode_vmul(vlmul_raw);
}

uint64_t rv64_csr_encode_vtype(rv64v_vtype_t* vtype) {
    uint64_t vtype_raw = 0;
    vtype_raw |= (uint64_t) ((vtype->vma & 0b1) << 7);
    vtype_raw |= (uint64_t) ((vtype->vta & 0b1) << 6);
    vtype_raw |= (uint64_t) ((vtype->selected_element_width & 0b111) << 3);
    uint8_t vlmul_raw = rv64_encode_vmul(vtype->vlmul);
    vtype_raw |= (uint64_t) (vlmul_raw & 0b111);
    return vtype_raw;
}
