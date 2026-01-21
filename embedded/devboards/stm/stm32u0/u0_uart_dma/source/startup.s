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

/**
 * Vector Table
 * @see RM0503 Section 11.3 Interrupt and exception vectors, Table 55. Vector table
 */
interrupt_vector_table:
    g_pfnVectors:           // compatibility label for older tools
    /* Handler                          Position    Address Priority    Acronym                 Description             */
    .word _stack_end                    // -        0x00                Top of Stack            Reserved
    .word reset_handler                 // -        0x04    -3          Reset                   Reset
    .word default_handler               // -        0x08    -2          NMI_Handler             Non maskable interrupt. The SRAM parity error, flash ECC double err., HSE CSS and LSE CSS are linked to the NMI vector.
    .word default_handler               // -        0x0C    -1          HardFault_Handler       All class of fault
    .word 0                             // -        0x10                Reserved
    .word 0                             // -        0x14                Reserved
    .word 0                             // -        0x18                Reserved
    .word 0                             // -        0x1C                Reserved
    .word 0                             // -        0x20                Reserved
    .word 0                             // -        0x24                Reserved
    .word 0                             // -        0x28                Reserved
    .word default_handler               // -        0x2C    3           SVC_Handler             System service call via SVC instruction (syscall, used for os level abstractions, not usually used in baremetal).
    .word 0                             // -        0x30                Reserved
    .word 0                             // -        0x34                Reserved
    .word default_handler               // -        0x38    5           PendSV_Handler          Pendable request for system service (usually used by RTOS/threaded systems for context switching).
    .word SysTick_Handler               // -        0x3C    6           SysTick_Handler         System tick timer (24 bit tick counter, conventionally set to ~1ms).

    /* External/Peripheral Interrupts (IRQ0 starts here) */
    .word default_handler               // 0        0x40    7           WWDG/IWDG               Window watchdog and independent watchdog interrupt
    .word default_handler               // 1        0x44    8           PVD/PVM                 PVD/PVM1/PVM2/PVM4 interrupt (combined with EXTI lines 16 & 19 & 20 & 21)
    .word default_handler               // 2        0x48    9           RTC/TAMP                RTC and TAMP interrupts (combined EXTI lines 19 & 21)
    .word default_handler               // 3        0x4C    10          FLASH                   Flash global interrupt
    .word default_handler               // 4        0x50    11          RCC/CRS
    .word default_handler               // 5        0x54    12          EXTI0_1
    .word default_handler               // 6        0x58    13          EXTI2_3
    .word default_handler               // 7        0x5C    14          EXTI4_15                EXTI lines 5 to 15 interrupt
    .word default_handler               // 8        0x60    15          USB                     USB global interrupt (combined with EXTI line 33)
    .word DMA1_Channel1_IRQHandler      // 9        0x64    16          DMA1_Channel1           DMA1 channel 1 interrupt
    .word default_handler               // 10       0x68    17          DMA1_Channel2_3         DMA channel 2 & 3 interrupts
    .word default_handler               // 11       0x6C    18          DMA_Channel4_5_6_7/DMAMUX/DMA2_Channel_2_3_4_5
    .word default_handler               // 12       0x70    19          ADC/COMP
    .word default_handler               // 13       0x74    20          TIM1_BRK_UP_TRG_COM
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
    .word default_handler   /* IRQ27: USART1 */
    .word USART2_LPUART2_IRQHandler /* IRQ28: USART2/LPUART2 */
    .word default_handler   /* IRQ29: USART3/LPUART1 */
    .word default_handler   /* IRQ30: USART4/LPUART3 */
    .word default_handler   /* IRQ31: AES/RNG */

.global DMA1_Channel1_IRQHandler
.type DMA1_Channel1_IRQHandler, %function

.global USART2_LPUART2_IRQHandler
.type USART2_LPUART2_IRQHandler, %function

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
