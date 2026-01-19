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
    .word _stack_end        // 0x00: Top of Stack
    .word reset_handler     // 0x04: Reset Handler
    .word default_handler   // 0x08: NMI (Non-Maskable Interrupt) Handler
    .word default_handler   // 0x0C: HardFault Handler
    .word 0                 // 0x10: Reserved
    .word 0                 // 0x14: Reserved
    .word 0                 // 0x18: Reserved
    .word 0                 // 0x1C: Reserved
    .word 0                 // 0x20: Reserved
    .word 0                 // 0x24: Reserved
    .word 0                 // 0x28: Reserved
    .word default_handler   // 0x2C: SVCall Handler 
    .word 0                 // 0x30: Reserved
    .word 0                 // 0x34: Reserved
    .word default_handler   // 0x38: PendSV Handler
    .word default_handler   // 0x3C: SysTick Handler

    /* External Interrupts (IRQ0 starts here) */
    .word default_handler   /* IRQ0:  WWDG */
    .word default_handler   /* IRQ1:  PVD */
    .word default_handler   /* IRQ2:  RTC */
    .word default_handler   /* IRQ3:  FLASH */
    .word default_handler   /* IRQ4:  RCC */
    .word default_handler   /* IRQ5:  EXTI0_1 */
    .word default_handler   /* IRQ6:  EXTI2_3 */
    .word default_handler   /* IRQ7:  EXTI4_15 */
    .word default_handler   /* IRQ8:  UCPD1/2 */
    .word default_handler   /* IRQ9:  DMA1_Channel1 */
    .word default_handler   /* IRQ10: DMA1_Channel2_3 */
    .word default_handler   /* IRQ11: DMA1_Channel4_7 */
    .word default_handler   /* IRQ12: ADC1 */
    .word default_handler   /* IRQ13: TIM1_BRK_UP_TRG_COM */
    .word default_handler   /* IRQ14: TIM1_CC */
    .word default_handler   /* IRQ15: TIM2 */
    .word default_handler   /* IRQ16: TIM3 */
    .word default_handler   /* IRQ17: Reserved */
    .word default_handler   /* IRQ18: LPTIM1 */
    .word default_handler   /* IRQ19: LPTIM2 */
    .word default_handler   /* IRQ20: TIM15 */
    .word default_handler   /* IRQ21: TIM16 */
    .word default_handler   /* IRQ22: TSC */
    .word default_handler   /* IRQ23: LCD */
    .word default_handler   /* IRQ24: I2C1 */
    .word default_handler   /* IRQ25: I2C2 */
    .word default_handler   /* IRQ26: SPI1 */
    .word default_handler   /* IRQ27: SPI2 */
    .word usart2_irq_handler /* IRQ28: USART2 */
    .word usart3_irq_handler /* IRQ29: USART3 */

.global usart2_irq_handler
.type usart2_irq_handler, %function

.global usart3_irq_handler
.type usart3_irq_handler, %function

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
