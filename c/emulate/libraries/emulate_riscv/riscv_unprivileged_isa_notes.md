# RISCV ISA Notes
Notes taken during a deep dive into the "RISC-V Instruction Set Manual Volume 1", subtitle "Unprivileged Architecture".

Source: https://github.com/riscv/riscv-isa-manual/
Compiled HTML Link:
* https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/
* https://riscv.github.io/riscv-isa-manual/snapshot/privileged/
Compiled PDF Link: https://drive.google.com/file/d/1uviu1nH-tScFfgrovvFCrj7Omv8tFtkp/view
 * If link is dead, start here and click around: https://riscv.org/specifications/ratified/

## Quick References
* `2.4. Integer Computational Instructions` - https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#_integer_computational_instructions
* `35. RV32/64G Instruction Set Listings` - https://riscv.github.io/riscv-isa-manual/snapshot/unprivileged/#rv32-64g

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
  * I - register-immediate (used for Load operations)
  * R - register-register
  * S - store
  * U - ???
  * B - branch
  * J - jump

## Notes
### 1. Intro
* Free for academia and industry, no expensive upfront or per-chip license fees like with ARM.
* "RISC-V" Etymology: 5th major RISC (Reduced Instruction Set Computer) from UC Berkeley: RISC-I, RISC-II, SOAR, SPUR.
* RISC-V emulators: SPIKE, QEMU, rv8
* "HART" Etymology: introduced to represent an abstract execution resource
* RISC-V is not 1 ISA, it is a family of ISAs with 4 "base" ISAs.
### 2. RV32I
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
* Loads for both signed and unsigned bit lengths from memory (signed loads sign extend, unsigned loads zero extend).
* TODO: 2.6 Load and Store instructions - reasearch EEI. Loads and Stores with unaligned addresses may raise an address-misaligned exception or access-fault exception.
* 2.7 Memory Ordering Instructions - "FENCE" instructions.
  * https://en.wikipedia.org/wiki/Memory_barrier
  * Used to order device I/O and memory accesses by other cores/devices.
  * 4 types:
    * I - Input
    * O - Output
    * R - Read
    * W - Write
* 2.8 environment Call and Breakpoints
  * ECALL - used to make a service request to the execution environment. EEI defines how parameters for the service request are passed. Used to implement Linux syscalls for example.
  * EBREAK - used to return control to a debugging environment. Used to implement debuggers.
* 2.9 HINT instructions
  * primarily used to communicate performance hints to the microarchitecture
  * there are no dedicated instructions for hints, they are embedded in normal instructions with values you'd never see, like "ADDI" with a x0 return address (x0 is hard coded to always be 0 and not writeable). Full table of these encodings are in table
### 3. RV32E
* RiscV 32-bit Embedded ISA. For Micro-controllers. Only change between RV32I and RV32E is reducing the number of integer registers from 32 to 16.
### 4. RV64I
* RV32I but with XLEN raised from 32 bits to 64 bits (all registers are 64 bits wide and address space increased to 64 bits) and a handful of extra instructions.
  * New "W" instructions (W for 32-bit word, all non-specified width instructions implicitly become 64-bit double word) which ignore the upper 32 bits of their input and always produce 32-bit signed values, sign-extending them to 64 bits.
* 4.4 HINT instructions - RV64I has extra hint encodings hidden throughout it's instruction set. See table 6. "RV64I HINT instructions" for a full list.
### I Extensions (5 - 11)
* Zifencei Instruction-Fetch Fence
  * "Zifencei" extension, includes `FENCE.I` instruction. Provides synchronizing within the local core.
  * "Z" modules are extensions. The second letter is the "most closely related to non-extension module", so if something extends atomics, the prefix would be "Za". "I" being so common as the second letter can maybe be seen as a catch all default or that most things do really work with integers. Mentally remove the first 2 letters to read the extension sanely (Zifencei -> fencei).
* "Zicsr" Control and Status Register (CSR) Instructions
  * csr - Control and Status Register(s)
  * CSRRW - Atomic Control and Status Register Read/Write
    * Reads the old value of the CSR, zero-extended, and writes it to rd.
    * if rd = x0, then does not read.
    * rs1 is written to the CSR (even if x0).
  * CSRRS - Atomic Control and Status Register Read and Set Bits
    * Reads the old value of the CSR, zero-extended, and writes it to rd.
    * rs1 is treated as a bit mask that specifies bit positions to be set in the CSR.
    * if rs1 = x0, then does not write.
  * CSRRC - Atomic Control and Status Register Read and Clear Bits
    * same as CSRRS but for clearing.
  * CSRRWI, CSRRSI, CSRRCI - same as above but with an immediate instead of a register.
  * There are potential "side effects" and maybe logic that can be triggered when a read/write occurs and some specific argument combinations should or shouldn't cause these "on read/write" side effect triggers to occur. See table 7. "Conditions determining whether a CSR instruction reads or writes the specified CSR." for details.
* "Zicntr" and "Zihpm" Counters
  * 32 64-bit performance counters and timers accessible through read-only CSR registers 0xC00 - 0xC1F for RV64I (0xC80 - 0xC9F for RV32I).
  * "Zi cntr" - (counter) 3 counters (CYCLE - cycle count, TIME - real-time clock, INSTRET - instructions retired)
  * "Zi hpm" - (high performance monitor) 29 additional counters/timers (hpmcounter3 - hpmcounter31)
* "Zihintntl" Non-Temporal Locality Hints
  * hint ntl - hints for non-temporal locality
  * TODO: double back on later, probably won't get to this until more required modules are implemented.
* "Zihintpause" pause hint
  * hint pause - pause instruction hints
  * reduce energy consumption while spin-waiting
  * encoded as a fence instruction
* "Zimop" May-Be-Operations
  * mop - may be operations
  * mops are initially defined to write 0 to x[rd], but are designed to be redefined.
  * encoding space for 40 mops
  * 32 MOP instructions: MOP.R.0 - MOP.R.31
  * 8 MOP instructions: MOP.RR.0 - MOP.R.7
* "Zicond" Integer Condition Operations
  * support conditional arithmetic and select/move operations
  * `czero.eqz rd, rs1, rs2`
  * `czero.nez rd, rs1, rs2`
### 12. "M" - Integer Multiplication and Division
* TODO: pickup here
