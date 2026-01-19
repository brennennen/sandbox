/**
 * @file main.c
 * @brief Simple STM32U0 UART Polling Test
 * Runs off the HSI (High Speed Internal) at 16MHz. Executes directly out of 
 * flash (.data and .bss are copied to RAM, but .text is not). Other demos 
 * will include a faster clock and executing out of RAM.
 *
 * This program configures the system clock to 16MHz and initializes USART2
 * in a simple polling mode (no interrupts, no DMA).
 *
 * It prints a greeting and then enters an infinite loop where it polls the 
 * RX register. If a character is received, it is immediately echoed back.
 *
 * Hardware Setup:
 * - MCU: STM32U083
 * - Board: Nucleo-U083RC
 * - Peripherals: 
 *   - LED4 (PA5)
 *   - USART2 (PA2=TX, PA3=RX) via ST-Link Virtual COM Port (or pins 0 and 1 of CN9, requires removing a solder bridge on bottom of board to disconnect the virtual comm tooling though).
 *
 * See ./../u0_blinky/readme.md for tool setup.
 * 
 * Build:
 * arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -g -O0 -T linker_script.ld -nostdlib -DDEBUG -o u0_uart_poll.elf ./source/main.c ./source/stm32u083xx.h ./source/startup.s
 * 
 * Flash:
 * pyocd flash --pack ./../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx ./u0_uart_poll.elf
 */

#include <stdint.h>
#include "stm32u083xx.h"

/** 
 * Number of cycles to wait when waiting for a register enable request
 * before giving up in failure.
 */
#define ENABLE_TIMEOUT_CYCLES ((uint16_t)(0xFFFF)) // 65,635 cycles

/** Standard return type */
typedef enum {
    SUCCESS,
    FAILURE
} result_t;

/**
 * Signals an unrecoverable software fault.
 */
void system_panic() {
#ifdef DEBUG
    __asm volatile ("bkpt #0"); // Force a breakpoint
    while(1) { 
        // Set breakpoint here
    }
#endif
    // TODO: Reset the microcontroller in release builds
    // NVIC_SystemReset(); // part of ARM CMSIS "core_cm0plus.h", not including to keep project small.
}

/**
 * Initializes the 16 MHz HSI (High Speed Internal) clock and configures
 * system clock to use it.
 * @return SUCCESS if the clock was configured correctly, FAILURE otherwise.
 */
result_t system_clock_init() {
    RCC->CR |= RCC_CR_HSION;            // Enable HS16 - 16 MHz clock
    uint16_t hsi16_enable_timeout = ENABLE_TIMEOUT_CYCLES;
    while (!(RCC->CR & RCC_CR_HSIRDY)) { // Wait for HSI16 to be ready
        if (hsi16_enable_timeout-- == 0) {
            return(FAILURE);
        }
    }
    RCC->CFGR &= ~RCC_CFGR_SW;          // Clear previous SYSCLOCK settings
    RCC->CFGR |= RCC_CFGR_SW_0;         // Set HSI16 as SYSCLOCK
    uint16_t sysclock_config_timeout = ENABLE_TIMEOUT_CYCLES;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_0) { // Wait for SYSCLOCK to be ready
        if (sysclock_config_timeout-- == 0) {
            return(FAILURE);
        }
    }
    return(SUCCESS);
}

/** Initialize the Nucleo board LED4. */
void led4_init() {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN; // Initialize the GPIO A controller
    GPIOA->MODER &= ~GPIO_MODER_MODE5; // Clear out previous GPIO port A pin 5 configuration
    GPIOA->MODER |= GPIO_MODER_MODE5_0; // Initialize GPIO port A pin 5 as output
    GPIOA->ODR |= GPIO_ODR_OD5; // Turn on the LED
}

/** Toggle LED4 on the nucleo board. */
void led4_toggle() {
    GPIOA->ODR ^= GPIO_ODR_OD5;
}

/**
 * Initializes USART2 for serial communication.
 * Settings: 115200,
 * - Baud Rate: 115200
 * - Word Length: 8 bits
 * - Stop Bits: 1
 * - Parity: None
 * - Flow Control: None
 * Pinout:
 * - PA2: USART2_TX
 * - PA3: USART2_RX
 */
void usart2_init() {
    // Configure RCC (Reset and Clock Control)
    RCC->APBENR1 |= RCC_APBENR1_USART2EN; // Enable USART2 (bus APB1)
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN; // Enable GPIO Port A

    // Configure GPIO Ports/Pins (PA2 and PA3)
    GPIOA->MODER &= ~GPIO_MODER_MODE2; // Clear pin PA2 mode
    GPIOA->MODER &= ~GPIO_MODER_MODE3; // Clear pin PA3 mode
    GPIOA->MODER |= GPIO_MODER_MODE2_1; // Set pin PA2 to Alternate Function mode
    GPIOA->MODER |= GPIO_MODER_MODE3_1; // Set pin PA3 to Alternate Function mode
    GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL2; // Clear PA2 Alternate Function settings
    GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL3; // Clear PA3 Alternate Functon settings
    GPIOA->AFR[0] |= (7UL << GPIO_AFRL_AFSEL2_Pos); // Configure PA2 to AF7 (Alternate Function 7, maps to USART_TX)
    GPIOA->AFR[0] |= (7UL << GPIO_AFRL_AFSEL3_Pos); // Configure PA3 to AF7 (Alternate Function 7, maps to USART2_RX)

    // Configure USART2 Peripheral
    USART2->BRR = 139; // Clock = 16MHz, Baud = 115200, BRR = 16,000,000 / 115,200 = 138.88 -> 139
    USART2->CR1 = USART_CR1_RE; // Receiver Enable
    USART2->CR1 |= USART_CR1_TE; // Transmitter Enable
    USART2->CR1 |= USART_CR1_UE; // USART Enable
}

/**
 * Transmits a single character over USART2.
 * @param c Character to transmit.
 */
void usart2_transmit_char(char c) {
    // wait until the transfer byte buffer has sent any previosu data
    while (!(USART2->ISR & USART_ISR_TXE_TXFNF)) { // TXE = Transmission data register is Empty
        // do nothing
    }
    USART2->TDR = c; // USART Transmit Data register
}

/**
 * Transmits a multi-character string over USART2.
 * @param str String to transmit.
 */
void usart2_transmit_str(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        usart2_transmit_char(str[i]);
        i++;
    }
}

/**
 * Receives a single character from USART2.
 * @param c Address to store the received character to.
 * @return 1 if a character was received, 0 otherwise.
 */
int usart2_receive_char(char *c) {
    if (USART2->ISR & USART_ISR_RXNE_RXFNE) { // Check Read Data Register Not Empty (RXNE)
        *c = (char)(USART2->RDR); // USART Receive Data register
        return 1;
    }
    return 0;
}

int main(void) {
    if (system_clock_init() != SUCCESS) {
        system_panic();
    }
    led4_init();
    usart2_init();

    usart2_transmit_str("\r\nSTM32U0 Minimal UART hello world!\r\n");
    usart2_transmit_str("(waiting for input to echo back...)\r\n");

    char rx_byte;
    
    while (1) {
        if (usart2_receive_char(&rx_byte)) {
            usart2_transmit_char(rx_byte); 
            if (rx_byte == '\r') {
                usart2_transmit_char('\n');
            }
        }
    }
}
