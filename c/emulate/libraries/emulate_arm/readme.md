# ARM Emulation

## Resources
* A64/ARM64/AArch64
  * https://en.wikipedia.org/wiki/AArch64
  * https://developer.arm.com/documentation/ddi0602/2025-03/Base-Instructions?lang=en
*

## A64 Notes
* Main opcode is in bits 25-28
  * 0xx0000 - reserved
  * 1xx0000 - SME
  * xxx0001 - Unallocated
  * xxx0010 - SVE
  * xxx0011 - Unallocated
  * xxx1000 - Data processing
  *

## ABIs
Different per OS.
* Linux:
  * ARM ABI - Old and being deprecated. Seen in ARMv4 and earlier (~1994).
  * EABI - Embedded Application Binary Interface. Toolchains: "arm-none-eabi"
* Android:
  * "arm-linux-androideabi"



## Building Arm assembly
### 32 bit
* `sudo apt install binutils-arm-none-eabi`
add.s
```asm
    .text
    .global _start
_start:
    add r0, r0, #5
    .end
```
* `arm-none-eabi-as -o add.o ./add.s`

### 64 bit
add.s
```asm
    .text
    .global _start
_start:
    add x0, x0, #5
    .end
```
* `aarch64-none-linux-gnu-as -o add.o add.s`
  * `aarch64-none-linux-gnu-as -o add.o add.s -mbig-endian`
  * `aarch64-none-linux-gnu-as -o add.o add.s -mlittle-endian`
* `aarch64-none-linux-gnu-objdump -d add.o`
* `add x0, x0, #5` == `91001400`
