# RISCV Privileged ISA Notes

Notes taken during a deep dive into the "RISC-V Instruction Set Manual Volume 2", subtitle "Privileged Architecture".
Compiled HTML Link:
* https://riscv.github.io/riscv-isa-manual/snapshot/privileged/

## Quick References
* `2.2 CSR Listing` - https://riscv.github.io/riscv-isa-manual/snapshot/privileged/#_csr_listing

## Terminology
* AEE - Application Execution Environment
* SEE - Supervisor Execution Environment
* HEE - Hypervisor Execution Environment
* ABI - Aplication Binary Interface
* SBI - Supervisor Binary Interface
* HBI - Hypervisor Binary Interface
*

## Notes
### 1 Intro
* 1.1 Terminology
  * You can run an application that talks through an ABI directly on the application execution environment (AEE = just the hardware?) or involve a OS to support running mutliple applications or involve a hypervisor to support running multiple isolated applications.
* 1.2 Privilege Levels
  * Privilege Levels:
    * 0 = user
    * 1 = supervisor
    * 2 = reserved
    * 3 = machine
  * most systems support both Machine and User modes. very simple embedded systems may support just the machine mode, and a full blown posix os would support all 3 (user, supervisor, and machine).
* 1.3 Debug Mode
  * (optional) off-chip debugging mode, extra mode with even more privilege than machine mode.
# 2 Control and Status Registers
CSRs are associated with a privilege level, they can be accessed by that level and higher (e.g. a superviser level csr can be accessed in both supervisor and machine mode).
* 2.1 CSR Address Mapping Conventions
  * access a csr without appropriate privilege level raises illegal instruction exception.
  * CSR high level addressing:
    * Machine-mode: 0x7A0 - 0x7BF
      * 0x7B0 - 0x7BF are accessible in debug mode only.
    *
* 2.2 CSR Listing
  * 0x000 - 0xFFF = 4k memory block
  * table 3. address ranges
  * tables 4 thru 8 - mapping of every csr (very important, will be referencing these a lot)
  * tables 9 thru 11 - TODO: what are these?
*
