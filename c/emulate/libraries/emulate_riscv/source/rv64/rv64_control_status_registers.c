
#include "rv64/rv64_control_status_registers.h"


// void write_csrs(emulator_rv64_t* emulator, uint8_t *data, int32_t data_size) {
//     data[0x001] = emulator.csrs.float_csrs.fflags;
//     // TODO
// }

char* rv64_get_csr_name(int32_t address) {
    switch(address) {
        // Unsupervised Floats
        case 0x001: return("fflags");
        case 0x002: return("frm");
        case 0x003: return("fcsr");

        // ...
        // Machine Trap Handling
        case 0x340: return("mscratch");
        // ...
        default: {
            // todo: log error
            return("???");
        }
    }
}

uint64_t rv64_get_csr_value(rv64_control_status_registers_t* csrs, int32_t address) {
    switch(address) {
        // Unsupervised Floats
        case 0x001: return(csrs->float_csrs.fflags);
        case 0x002: return(csrs->float_csrs.frm);
        case 0x003: return(csrs->float_csrs.fcsr);
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
