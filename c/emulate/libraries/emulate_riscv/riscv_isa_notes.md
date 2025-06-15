# RISCV ISA Notes
Notes taken during a deep dive into the "RISC-V Instruction Set Manual Volume 1", subtitle "Unprivileged Architecture".

Source: https://github.com/riscv/riscv-isa-manual/
Compiled HTML Link:
* https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/
* https://riscv.github.io/riscv-isa-manual/snapshot/privileged/
Compiled PDF Link: https://drive.google.com/file/d/1uviu1nH-tScFfgrovvFCrj7Omv8tFtkp/view
 * If link is dead, start here and click around: https://riscv.org/specifications/ratified/

TODO: pickup on page 31, section: 2.6 load and store instructions

## Terminology
* RISC-V Hardware Platform - A single core microcontroller to a many-thousand-node cluster of shared-memory manycore server nodes.
* Component -
* Core - A component that has an independent instruction fetch unit.
* hart - RISCV compatible hardware (or virtual?) thread. A "thing" capable of autonomously fetching and executing RISC-V instructions.
* Coprocessor - A unit attached to a RISC-V core.
* Accelerator - Either a non-programmable fixed-function unit or a core that can operate autonomously but is specialized for certain tasks.
  * ex: I/O Accelerators - Offload I/O processing tasks from main application cores
* SBI - RISC-V Supervisor Binary Interface -
* EEI - Execution Environment Interface - initial state of program, hardware threads, interrupt handling, environment calls, etc.
  * Example EEIs: Bare metal, OSes, Hypervisors, Emulators
* Base ISA - All the core instructions needed to function as a conventional cpu (load, store, simple arithmetic add/sub, simple logical and/or, etc.). One of these is needed per RISC-V core? Current Base ISAs: rv32i, rv32e, rv64i, rv64e (i is all included, e is a reduced set for micro-controllers).
* Extension ISAs - ISAs like SIMD (single instruction-multiple data).
* XLEN - Width of the integer register in bits (32 vs 64 bit currently)
* RVWMO - Risc-V Weak Memory Ordering - Memory consistency model. Execution ordering is only garunteed per "hart"/core. Multi-threaded code requires explicit synchronization to guarantee ordering.
* IALIGN - instruction width alignment, 16 or 32 bits (16 for compressed, 32 for everything else).
* ILEN - Max instruction length (32 for base ISA, may be larger for extension ISAs).
* Parcel - 16 bit chunk of memory
* Exception - Unusual condition occurring at run time associated with an instruction.
* Interrupt - External asynchronous event causing a transfer of control
* Trap - Transfer of control to a trap handler caused by either an exception or interrupt.
* Contained Trap - Trap contained in a single hart.
* Requested Trap - Synchronous exception to execution environment from inside execution environment. System call.
* Invisible Trap - Trap handled and execution resumes normally. Emulating missing instructions, handling page faults, etc.
* Fatal Trap - Trap that causes the execution environment to terminate execution.
* Core Instruction Formats:
  * I - register-immediate
  * R - register-register
  * S - ???
  * U - ???
  * B - branch
  * J - jump

## Notes
* Free for academia and industry, no expensive upfront or per-chip license fees like with ARM.
* "RISC-V" Etymology: 5th major RISC (Reduced Instruction Set Computer) from UC Berkeley: RISC-I, RISC-II, SOAR, SPUR.
* RISC-V emulators: SPIKE, QEMU, rv8
* "HART" Etymology: introduced to represent an abstract execution resource
* RISC-V is not 1 ISA, it is a family of ISAs with 4 "base" ISAs.
* Base integer instruction sets use a two's complement representation for signed integer values.
* A RVI128I is in development.
* RV32I and RV64I are not compatible, this breaks from the convention that Intel started where you can run x86 code on an x64 device. For RISCV land, you'd need to emulate the RV32I on a RV64I built device (or just recompile for RV64I). This simplifies many design issues.
* RISC-V "privileged" archtecuture provides fields in "misa"
* Extensions: "M" for multiply/divide, "A" for atomic, "F" for floating-point, "D" for double, "C" compressed (16 bit variations of base ISA).
* size names: word = 4 bytes (32 bits), halfword = 2 bytes (16 bits), double word = 8 bytes (64 bits), quad word = 16 bytes (128 bits).
* risc-v hart has a single byte-addressable address space of 2^{xlen} bytes, where xlen is 32 or 64 bits depending on the base isa. the address space is circular, so 2^{xlen} - 1 is adjacent to 0. Memory address computations done by the hardware ignore overflow and wrap around modulo 2^{xlen}.
* execution environment determines the memory map. memory can be vacant, contain "main memory", or contain an I/O device. Reads/Writes to I/O device memory may have visible side effects, main memory cannot.
* caching requires specific fence or cache-control instructions.
* RISC-V Weak Memory Ordering
* The standard RISC-V ISAs are all 32 bit wide, but extensions can be any multiple of 16 bits.
* ISA instruction length encoding convention: all 32-bit extensions have the lowest 2 bits set to "11". The compressed 16-bit extension ISA have their lowest 2 bits set to 00, 01, or 10.
* RISC-V base ISAs have either little-endian or big-endian memory systems.
* Instructions are stored in memory as a sequence of 16-bit little-endian parcels, regardless of memory system endianness.
* The length of an instruction can always be determined by the first 16 bits in an instruction (usually just the first 8 bits), which simplifies the decode process (opposed to intel's ISAs, which are (understandably given when they were developed) a chaotic organic mess).
* RV32i consists of 40 instructions
* For RV32I, the 32 x registers are each 32 bits wide (xlen = 32), register x0 is always 0. one additional unprivileged register: pc, holds the address of the current instruction
* 4 core instruction formats: R, I, S, and U
* sign bit for all immediates inside of instruction byte code is always in the 31st bit. goal being to optimize for hardware complexity.
* immediates are sign extended
* There are 2 additional formats: B and J. They are the same as S and U (respectively) but shifted.
* Section 2.4.1 (page 26) has the definition for "ADDI" and is the start of instruction definitions.
* register-register and register-immediate use `rd` as the detination register.
* No special instructions for overflow checking (no overflow flag like intel). You instead do checks in software by adding branch instructions and checking.
* 
