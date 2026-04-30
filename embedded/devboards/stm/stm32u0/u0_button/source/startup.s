/**
 * Minimal startup script for the STM32U0 (Cortex-M0+).
 */

.syntax unified     /* Modern mix of thumb and standard ARM instruction syntax. */
.cpu cortex-m0plus  /* STM32U0 is the ARM Cortex M0+. */
.fpu softvfp        /* m0+ has no hardware FPU. Need to use software FPU. */
.thumb              /* Generate Thumb-2 instructions (Thumb-2 includes 16-bit and 32-bit instructions, old A32 mode is now pretty much obsolete). */

.global interrupt_vector_table
.global reset_handler
.global default_handler

/**
 * Interrupt Vector Table
 */
.section .isr_vector, "a", %progbits
.type VectorTable, %object

interrupt_vector_table:
    g_pfnVectors:           // compatibility label for older tools
    .word _stack_end        // Address 0x00: Top of Stack
    .word reset_handler     // Address 0x04: Reset Handler
    .word default_handler   // Address 0x08: NMI (Non-Maskable Interrupt) Handler
    .word default_handler   // Address 0x0C: HardFault Handler

/**
 * Reset Handler
 */
.section .text.reset_handler
.weak reset_handler
.type reset_handler, %function
reset_handler:
    ldr   r0, = _stack_end
    mov   sp, r0

    /* Copy .data segment from flash to ram. */
    ldr r0, = _data_start
    ldr r1, = _data_end
    ldr r2, = _init_data_start
    movs r3, #0
    b initialize_data_loop

initialize_data:
    ldr r4, [r2, r3]
    str r4, [r0, r3]
    adds r3, r3, #4
initialize_data_loop:
    adds r4, r0, r3
    cmp r4, r1
    bcc initialize_data
    ldr r2, = _bss_start
    ldr r4, = _bss_end
    movs r3, #0
    b initialize_bss_loop

initialize_bss:
    str  r3, [r2]
    adds r2, r2, #4
initialize_bss_loop:
    cmp r2, r4
    bcc initialize_bss

    bl main     // Jump to application code
    b .         // main should never return, loop forever if it does.

/**
 * Default Handler
 */
.section .text.default_handler, "ax", %progbits
default_handler:
    b .         // hang/loop forever
