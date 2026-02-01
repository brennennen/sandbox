
#include <stdint.h>

#include "rv64/rv64_control_status_registers.h"

/**
 * Look up the CSR name from the address.
 * @see 2.2 CSR Listing - https://riscv.github.io/riscv-isa-manual/snapshot/privileged/#_csr_listing
 */
char* rv64_csr_name_from_address(int32_t address) {
    switch (address) {
        /*
         * MARK: Unsupervised Floats
         */
        case 0x001:
            return ("fflags");
        case 0x002:
            return ("frm");
        case 0x003:
            return ("fcsr");
        // Unprivileged Vector CSRs
        case 0x008:
            return ("vstart");
        case 0x009:
            return ("vxsat");
        case 0x00A:
            return ("vxrm");
        case 0x00F:
            return ("vcsr");
        case 0xc20:
            return ("vl");
        case 0xc21:
            return ("vtype");
        case 0xc22:
            return ("vlenb");
        // Unprivileged Zicfiss
        case 0x011:
            return ("ssp");
        // Unprivileged seed for cryptographic random bit generators
        case 0x015:
            return ("seed");
        // Unprivileged Zcmt Extension CSR
        case 0x017:
            return ("jvt");
        // Unprivileged Counter/Timers
        case 0xc00:
            return ("cycle");
        case 0xc01:
            return ("time");
        case 0xc02:
            return ("instret");
        case 0xc03:
            return ("hpmcounter3");
        case 0xc04:
            return ("hpmcounter4");
        case 0xc05:
            return ("hpmcounter5");
        case 0xc06:
            return ("hpmcounter6");
        case 0xc07:
            return ("hpmcounter7");
        case 0xc08:
            return ("hpmcounter8");
        case 0xc09:
            return ("hpmcounter9");
        case 0xc0a:
            return ("hpmcounter10");
        case 0xc0b:
            return ("hpmcounter11");
        case 0xc0c:
            return ("hpmcounter12");
        case 0xc0d:
            return ("hpmcounter13");
        case 0xc0e:
            return ("hpmcounter14");
        case 0xc0f:
            return ("hpmcounter15");
        case 0xc10:
            return ("hpmcounter16");
        case 0xc11:
            return ("hpmcounter17");
        case 0xc12:
            return ("hpmcounter18");
        case 0xc13:
            return ("hpmcounter19");
        case 0xc14:
            return ("hpmcounter20");
        case 0xc15:
            return ("hpmcounter21");
        case 0xc16:
            return ("hpmcounter22");
        case 0xc17:
            return ("hpmcounter23");
        case 0xc18:
            return ("hpmcounter24");
        case 0xc19:
            return ("hpmcounter25");
        case 0xc1a:
            return ("hpmcounter26");
        case 0xc1b:
            return ("hpmcounter27");
        case 0xc1c:
            return ("hpmcounter28");
        case 0xc1d:
            return ("hpmcounter29");
        case 0xc1e:
            return ("hpmcounter30");
        case 0xc1f:
            return ("hpmcounter31");
        case 0xc80:
            return ("cycleh");
        case 0xc81:
            return ("timeh");
        case 0xc82:
            return ("instreth");
        case 0xc83:
            return ("hpmcounter3h");
        case 0xc84:
            return ("hpmcounter4h");
        case 0xc85:
            return ("hpmcounter5h");
        case 0xc86:
            return ("hpmcounter6h");
        case 0xc87:
            return ("hpmcounter7h");
        case 0xc88:
            return ("hpmcounter8h");
        case 0xc89:
            return ("hpmcounter9h");
        case 0xc8a:
            return ("hpmcounter10h");
        case 0xc8b:
            return ("hpmcounter11h");
        case 0xc8c:
            return ("hpmcounter12h");
        case 0xc8d:
            return ("hpmcounter13h");
        case 0xc8e:
            return ("hpmcounter14h");
        case 0xc8f:
            return ("hpmcounter15h");
        case 0xc90:
            return ("hpmcounter16h");
        case 0xc91:
            return ("hpmcounter17h");
        case 0xc92:
            return ("hpmcounter18h");
        case 0xc93:
            return ("hpmcounter19h");
        case 0xc94:
            return ("hpmcounter20h");
        case 0xc95:
            return ("hpmcounter21h");
        case 0xc96:
            return ("hpmcounter22h");
        case 0xc97:
            return ("hpmcounter23h");
        case 0xc98:
            return ("hpmcounter24h");
        case 0xc99:
            return ("hpmcounter25h");
        case 0xc9a:
            return ("hpmcounter26h");
        case 0xc9b:
            return ("hpmcounter27h");
        case 0xc9c:
            return ("hpmcounter28h");
        case 0xc9d:
            return ("hpmcounter29h");
        case 0xc9e:
            return ("hpmcounter30h");
        case 0xc9f:
            return ("hpmcounter31h");

        /*
         * MARK: Supervisor
         */
        // Supervisor Trap Setup
        case 0x100:
            return ("sstatus");
        case 0x104:
            return ("sie");
        case 0x105:
            return ("stvec");
        case 0x106:
            return ("scounteren");
        // Supervisor Configuration
        case 0x10a:
            return ("senvcfg");
        // Supervisor Counter Setup
        case 0x120:
            return ("scountinhibit");
        // Supervisor Trap Handling
        case 0x140:
            return ("sscratch");
        case 0x141:
            return ("sepc");
        case 0x142:
            return ("scause");
        case 0x143:
            return ("stval");
        case 0x144:
            return ("sip");
        case 0xda0:
            return ("scountovf");
        // Supervisor Indirect
        case 0x150:
            return ("siselect");
        case 0x151:
            return ("sireg");
        case 0x152:
            return ("sireg2");
        case 0x153:
            return ("sireg3");
        case 0x154:
            return ("sireg4");
        case 0x155:
            return ("sireg5");
        case 0x156:
            return ("sireg6");
        // Supervisor Protection and Translation
        case 0x180:
            return ("satp");
        // Supervisor Timer Compare
        case 0x14d:
            return ("stimecmp");
        case 0x15d:
            return ("stimecmph");
        // Debug/Trace Registers
        case 0x5a8:
            return ("scontext");
        // Supervisor Resource Management Configuration
        case 0x181:
            return ("srmcfg");
        // Supervisor State Enable Registers
        case 0x10c:
            return ("sstateen0");
        case 0x10d:
            return ("sstateen1");
        case 0x10e:
            return ("sstateen2");
        case 0x10f:
            return ("sstateen3");
        // Supervisor Control Transfer Records Configuration
        case 0x14e:
            return ("sctrctl");
        case 0x14f:
            return ("sctrstatus");
        case 0x15e:
            return ("sctrdepth");

        /*
         * MARK: Hypervisor
         */
        // Hypervisor Trap Setup
        case 0x600:
            return ("hstatus");
        case 0x602:
            return ("hedeleg");
        case 0x603:
            return ("hideleg");
        case 0x604:
            return ("hie");
        case 0x606:
            return ("hcounteren");
        case 0x607:
            return ("hgeie");
        case 0x612:
            return ("hedelegh");
        // Hypervisor Trap Handling
        case 0x643:
            return ("htval");
        case 0x644:
            return ("hip");
        case 0x645:
            return ("hvip");
        case 0x64a:
            return ("htinst");
        case 0xe12:
            return ("hgeip");
        // Hypervisor Configuration
        case 0x60a:
            return ("henvcfg");
        case 0x61a:
            return ("henvcfgh");
        // Hypervisor Protection and Translation
        case 0x680:
            return ("hgatp");
        // Debug/Trace Registers
        case 0x6a8:
            return ("hcontext");
        case 0x605:
            return ("htimedelta");
        case 0x615:
            return ("htimedeltah");
        // Hypervisor State Enable Registers
        case 0x60c:
            return ("hstateen0");
        case 0x60d:
            return ("hstateen1");
        case 0x60e:
            return ("hstateen2");
        case 0x60f:
            return ("hstateen3");
        case 0x61c:
            return ("hstateen0h");
        case 0x61d:
            return ("hstateen1h");
        case 0x61e:
            return ("hstateen2h");
        case 0x61f:
            return ("hstateen3h");
        // Virtual Supervisor Registers
        case 0x200:
            return ("vsstatus");
        case 0x204:
            return ("vsie");
        case 0x205:
            return ("vstvec");
        case 0x240:
            return ("vsscratch");
        case 0x241:
            return ("vsepc");
        case 0x242:
            return ("vscause");
        case 0x243:
            return ("vstval");
        case 0x244:
            return ("vsip");
        case 0x280:
            return ("vsatp");
        // Virtual Supervisor Indirect
        case 0x250:
            return ("vsiselect");
        case 0x251:
            return ("vsireg");
        case 0x252:
            return ("vsireg2");
        case 0x253:
            return ("vsireg3");
        case 0x255:
            return ("vsireg4");
        case 0x256:
            return ("vsireg5");
        case 0x257:
            return ("vsireg6");
        // Virtual Supervisor Timer Compare
        case 0x24d:
            return ("vstimecmp");
        case 0x25d:
            return ("vstimecmph");
        // Virtual Supervisor Control Transfer Records Configuration
        case 0x24e:
            return ("vsctrctl");

        /*
         * MARK: Machine
         */
        // Machine Information Registers
        case 0xf11:
            return ("mvendorid");
        case 0xf12:
            return ("marchid");
        case 0xf13:
            return ("mimpid");
        case 0xf14:
            return ("mhartid");
        case 0xf15:
            return ("mconfigptr");
        // Machine Trap Setup
        case 0x300:
            return ("mstatus");
        case 0x301:
            return ("misa");
        case 0x302:
            return ("medeleg");
        case 0x303:
            return ("mideleg");
        case 0x304:
            return ("mie");
        case 0x305:
            return ("mtvec");
        case 0x306:
            return ("mcounteren");
        case 0x310:
            return ("mstatush");
        case 0x312:
            return ("medelegh");
        // Machine Counter Configuration
        case 0x321:
            return ("mcyclecfg");
        case 0x322:
            return ("minstretcfg");
        case 0x721:
            return ("mcyclecfgh");
        case 0x722:
            return ("minstretcfgh");
        // Machine Trap Handling
        case 0x340:
            return ("mscratch");
        case 0x341:
            return ("mepc");
        case 0x342:
            return ("mcause");
        case 0x343:
            return ("mtval");
        case 0x344:
            return ("mip");
        case 0x34a:
            return ("mtinst");
        case 0x34b:
            return ("mtval2");
        // Machine Indirect
        case 0x350:
            return ("miselect");
        case 0x351:
            return ("mireg");
        case 0x352:
            return ("mireg2");
        case 0x353:
            return ("mireg3");
        case 0x355:
            return ("mireg4");  // not 0x354... literally the devil...
        case 0x356:
            return ("mireg5");
        case 0x357:
            return ("mireg6");
        // Machine Configuration
        case 0x30a:
            return ("menvcfg");
        case 0x31a:
            return ("menvcfgh");
        case 0x747:
            return ("mseccfg");
        case 0x757:
            return ("mseccfgh");
        // Machine Memory Protection
        case 0x3a0:
            return ("pmpcfg0");
        case 0x3a1:
            return ("pmpcfg1");
        case 0x3a2:
            return ("pmpcfg2");
        case 0x3a3:
            return ("pmpcfg3");
        // todo: 4 - 13
        case 0x3ae:
            return ("pmpcfg14");
        case 0x3af:
            return ("pmpcfg15");
        case 0x3b0:
            return ("pmpaddr0");
        case 0x3b1:
            return ("pmpaddr1");
        // todo: 2 - 62
        case 0x3ef:
            return ("pmpaddr63");
        // Machine State Enable Registers
        case 0x30c:
            return ("mstateen0");
        case 0x30d:
            return ("mstateen1");
        case 0x30e:
            return ("mstateen2");
        case 0x30f:
            return ("mstateen3");
        case 0x31c:
            return ("mstateen0h");
        case 0x31d:
            return ("mstateen1h");
        case 0x31e:
            return ("mstateen2h");
        case 0x31f:
            return ("mstateen3h");
        // Machine Non-Maskable Interrupt Handling
        case 0x740:
            return ("mnscratch");
        case 0x741:
            return ("mnepc");
        case 0x742:
            return ("mncause");
        case 0x744:
            return ("mnstatus");
        // Machine Counter/Timers
        case 0xb00:
            return ("mcycle");
        case 0xb02:
            return ("minstret");
        case 0xb03:
            return ("mhpmcounter3");
        case 0xb04:
            return ("mhpmcounter4");
        // todo: 5 - 30
        case 0xb1f:
            return ("mhpmcounter31");
        case 0xb80:
            return ("mcycleh");
        case 0xb82:
            return ("minstreth");
        case 0xb83:
            return ("mhpmcounter3h");
        case 0xb84:
            return ("mhpmcounter4h");
        // todo: 5 - 30
        case 0xb9f:
            return ("mhpmcounter31h");
        // Machine Counter Setup
        case 0x320:
            return ("mcountinhibit");
        case 0x323:
            return ("mhpmevent3");
        case 0x324:
            return ("mhpmevent4");
        // todo: 5 - 30
        case 0x33f:
            return ("mhpmevent31");
        case 0x723:
            return ("mhpmevent3h");
        case 0x724:
            return ("mhpmevent4h");
        // todo: 5 - 30
        case 0x73f:
            return ("mhpmevent31h");
        // Machine Control Transfer Records Configuration
        case 0x34e:
            return ("mctrctl");
        // Debug/Trace Registers (shared with Debug Mode)
        case 0x7a0:
            return ("tselect");
        case 0x7a1:
            return ("tdata1");
        case 0x7a2:
            return ("tdata2");
        case 0x7a3:
            return ("tdata3");
        case 0x7a8:
            return ("mcontext");
        // Debug Mode Registers
        case 0x7b0:
            return ("dcsr");
        case 0x7b1:
            return ("dpc");
        case 0x7b2:
            return ("dscratch0");
        case 0x7b3:
            return ("dscratch1");
        default: {
            // todo: log error
            return ("unknowncsr");
        }
    }
}
