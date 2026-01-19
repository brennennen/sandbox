/**
 * @file main.c
 * @brief Simple STM32U0 UART Polling test.
 *
 * This program configures the system clock to 56MHz and initializes both
 * USART2 and USART3 using interrupts for rx/tx to free up the main
 * execution thread.
 *
 * Clock Notes:
 * - HSI (Reference): Generates a stable 16 MHz clock signal.
 * - PLL VCO (Multiplier): An internal analog oscillator that runs at 112 MHz.
 * It stays synchronized ("Locked") to the HSI by comparing its speed
 * to the HSI reference and adjusting itself to run exactly 7x faster.
 * - PLL R (Divider): The VCO's 112 MHz is too fast for the CPU logic (rated
 * for 56 MHz max). We divide the VCO output by 2 to get the final 56 MHz System
 * Clock.
 *
 * I also optimized the compiler flags for size with this build (got down to ~1.4KB
 * program size 'arm-none-eabi-size -t u0_uart_it.elf' ).
 *
 * Hardware Setup:
 * - MCU: STM32U083
 * - Board: Nucleo-U083RC
 * - Peripherals:
 *   - LED4 (PA5)
 *   - USART2 (PA2=TX, PA3=RX) via ST-Link Virtual COM Port (or pins 0 and 1 of CN9, requires removing a solder bridge on bottom of board to disconnect the virtual comm tooling though).
 *   - USART3 (PB8=TX, PB9=RX) via external usb to uart cable (top 2 CN5 pins, silk screen names: SCL/D15, SDA/D14).
 *
 * See ./../u0_blinky/readme.md for tool setup.
 *
 * Build:
 * build.py
 *
 * Flash:
 * pyocd flash --pack ./../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx ./u0_uart_it.elf
 *
 * Test:
 * * Connect to USART2 (Virtual COMM or PA2 and PA3 if sodler bridge removed) with a standard tty.
 * * Connect to USART3 (PB8, PB9).
 * * Run "test.py" to send messages over either comm port (ex: test.py "the quick brown fox jumped over the lazy dog")
 */

#include <stdint.h>
#include <stdbool.h>
#include <stm32u083xx.h>

/**
 * Number of cycles to wait when waiting for a register enable request
 * before giving up in failure.
 */
#define ENABLE_TIMEOUT_CYCLES ((uint16_t)(0xFFFF)) // 65,635 cycles

#define UART_BUFFER_SIZE      256

/** Standard return type */
typedef enum {
    SUCCESS,
    FAILURE
} result_t;

/** Buffer to store message data until the idle line indicates a finished message. */
typedef struct {
    uint8_t buffer[UART_BUFFER_SIZE];
    volatile uint16_t head; // Producer index
    volatile uint16_t tail; // Consumer index
} ring_buffer_t;

ring_buffer_t usart2_rx_buffer = {0};
ring_buffer_t usart2_tx_buffer = {0};
volatile bool usart2_msg_received = false;

ring_buffer_t usart3_rx_buffer = {0};
ring_buffer_t usart3_tx_buffer = {0};
volatile bool usart3_msg_received = false;

result_t ring_buffer_put(ring_buffer_t *rb, uint8_t data) {
    uint16_t next_head = (rb->head + 1) % UART_BUFFER_SIZE;
    if (next_head == rb->tail) return FAILURE; // Buffer Full
    rb->buffer[rb->head] = data;
    rb->head = next_head;
    return SUCCESS;
}

/**
 * Retrieves a single element from a ring buffer.
 * @param buffer Ring buffer to retrieve an element from.
 * @param data Where to store the retrieved element.
 */
result_t ring_buffer_get(ring_buffer_t *buffer, uint8_t *data) {
    if (buffer->head == buffer->tail) {
        return FAILURE; // Buffer Empty
    }
    *data = buffer->buffer[buffer->tail];
    buffer->tail = (buffer->tail + 1) % UART_BUFFER_SIZE;
    return SUCCESS;
}


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
 * Initializes the 56 MHz HSI (High Speed Internal) + PLL system and system
 * clock to use it.
 * @return SUCCESS if the clock was configured correctly, FAILURE otherwise.
 */
result_t system_clock_init() {
    // Reset and Clock Controller
    RCC->APBENR1 |= RCC_APBENR1_PWREN; // Enable Power Controller Clock

    // Power Control
    PWR->CR1 &= ~PWR_CR1_VOS; // Clear voltage range
    PWR->CR1 |= PWR_CR1_VOS_0; // Set High Performance voltage range (needed for 56 MHz clock)
    uint32_t timeout = ENABLE_TIMEOUT_CYCLES;
    while ((PWR->SR2 & PWR_SR2_VOSF) != 0) { // Wait until voltage range has updated.
        if (timeout-- == 0) {
            return FAILURE;
        }
    }

    // Flash Latency Configuration
    FLASH->ACR &= ~FLASH_ACR_LATENCY; // Clear previous flash wait states
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS; // 2 Wait States, needed for 56 MHz

    // Verify Latency was set
    if ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_2WS) {
        return FAILURE;
    }
    // Enable HSI (16MHz)
    RCC->CR |= RCC_CR_HSION;            // Enable HS16 - 16 MHz clock
    uint16_t hsi16_enable_timeout = ENABLE_TIMEOUT_CYCLES;
    while (!(RCC->CR & RCC_CR_HSIRDY)) { // Wait for HSI16 to be ready
        if (hsi16_enable_timeout-- == 0) {
            return(FAILURE);
        }
    }
    // Configure PLL (16 MHz -> 56 MHz)
    RCC->CR &= ~RCC_CR_PLLON;
    while((RCC->CR & RCC_CR_PLLRDY));
    RCC->PLLCFGR = 0; // Clear PLL Config
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_1; // PLL Source: HSI16 (Value 2)
    RCC->PLLCFGR |= (7 << RCC_PLLCFGR_PLLN_Pos); // PLLN (Multiplier)    = 7 -> 16 * 7 = 112 MHz
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN; // PLLRCLK clock output enable
    // PLLR (Output Div): 0b001 = divider of 2 -> 112 / 2 = 56 MHz
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLR_0;
    RCC->CR |= RCC_CR_PLLON; // System PLL clock enable
    timeout = ENABLE_TIMEOUT_CYCLES;
    while (!(RCC->CR & RCC_CR_PLLRDY)) { // Waint until the PLL is ready
        if (timeout-- == 0) return FAILURE;
    }

    // Configure SYSCLOCK (set to PLL)
    RCC->CFGR &= ~RCC_CFGR_SW;          // Clear previous SYSCLOCK settings
    RCC->CFGR |= (RCC_CFGR_SW_0 | RCC_CFGR_SW_1); // Set SYSCLOCK as PLL
    timeout = ENABLE_TIMEOUT_CYCLES;
    while ((RCC->CFGR & RCC_CFGR_SWS) != (RCC_CFGR_SWS_0 | RCC_CFGR_SWS_1)) {
        if (timeout-- == 0) return FAILURE;
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
 *
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
    GPIOA->AFR[0] |= (7UL << GPIO_AFRL_AFSEL2_Pos); // Configure PA2 to AF7 (Alternate Function 7, maps to USART2_TX)
    GPIOA->AFR[0] |= (7UL << GPIO_AFRL_AFSEL3_Pos); // Configure PA3 to AF7 (Alternate Function 7, maps to USART2_RX)

    // Enable the USART2 interrupt
    NVIC_EnableIRQ(USART2_LPUART2_IRQn); // USART2_IRQn
    NVIC_SetPriority(USART2_LPUART2_IRQn, 1); // (0 - 10, 0 = most important)

    // Configure USART2 Peripheral
    USART2->BRR = 486; // Clock = 56MHz, Baud = 115200, BRR = 56,000,000 / 115,200 = 486
    USART2->CR1 = USART_CR1_RE; // Receiver Enable
    USART2->CR1 |= USART_CR1_TE; // Transmitter Enable
    USART2->CR1 |= USART_CR1_UE; // USART Enable
    USART2->CR1 |= USART_CR1_RXNEIE_RXFNEIE; // RXNE and RX FIFO Not Empty Interrupt Enable
    USART2->CR1 |= USART_CR1_IDLEIE; // IDLE Interrupt Enable
}

/**
 * Initializes USART3 for serial communication.
 *
 * Settings: 115200,
 * - Baud Rate: 115200
 * - Word Length: 8 bits
 * - Stop Bits: 1
 * - Parity: None
 * - Flow Control: None
 * Pinout:
 * - PB8: USART3_TX
 * - PB9: USART3_RX
 */
void usart3_init() {
    // Configure RCC (Reset and Clock Control)
    RCC->APBENR1 |= RCC_APBENR1_USART3EN; // Enable USART2 (bus APB1)
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN; // Enable GPIO Port A

    // Configure GPIO Ports/Pins (PB8 and PB9)
    GPIOB->MODER &= ~GPIO_MODER_MODE8; // Clear pin PB8 mode
    GPIOB->MODER &= ~GPIO_MODER_MODE9; // Clear pin PB9 mode
    GPIOB->MODER |= GPIO_MODER_MODE8_1; // Set pin PB8 to Alternate Function mode
    GPIOB->MODER |= GPIO_MODER_MODE9_1; // Set pin PB9 to Alternate Function mode
    GPIOB->AFR[0] &= ~GPIO_AFRH_AFSEL8; // Clear PB8 Alternate Function settings
    GPIOB->AFR[0] &= ~GPIO_AFRH_AFSEL9; // Clear PB9 Alternate Functon settings
    GPIOB->AFR[1] |= (7UL << GPIO_AFRH_AFSEL8_Pos); // Configure PB8 to AF7 (Alternate Function 7, maps to USART3_TX)
    GPIOB->AFR[1] |= (7UL << GPIO_AFRH_AFSEL9_Pos); // Configure PB9 to AF7 (Alternate Function 7, maps to USART3_RX)

    // Enable the USART3 interrupt
    NVIC_EnableIRQ(USART3_LPUART1_IRQn); // USART3_IRQn
    NVIC_SetPriority(USART3_LPUART1_IRQn, 1); // (0 - 10, 0 = most important)

    // Configure USART3 Peripheral
    USART3->BRR = 486; // Clock = 56MHz, Baud = 115200, BRR = 56,000,000 / 115,200 = 486
    USART3->CR1 = USART_CR1_RE; // Receiver Enable
    USART3->CR1 |= USART_CR1_TE; // Transmitter Enable
    USART3->CR1 |= USART_CR1_UE; // USART Enable
    USART3->CR1 |= USART_CR1_RXNEIE_RXFNEIE; // RXNE and RX FIFO Not Empty Interrupt Enable
    USART3->CR1 |= USART_CR1_IDLEIE; // IDLE Interrupt Enable
}

void usart2_irq_handler() {
    uint32_t isr = USART2->ISR;

    // Receive ISR
    if (isr & USART_ISR_RXNE_RXFNE) { // Read data register not empty
        uint8_t data = (uint8_t)(USART2->RDR); // Auto clears the RX ISR
        ring_buffer_put(&usart2_rx_buffer, data);
    }

    // IDLE line detected (Received a full message and now the line is idle)
    if (isr & USART_ISR_IDLE) {
        USART2->ICR = USART_ICR_IDLECF;
        usart2_msg_received = true;
    }

    // Transmit ISR
    if (isr & USART_ISR_TXE_TXFNF) {
        if (USART2->CR1 & USART_CR1_TXEIE_TXFNFIE) {
            uint8_t data;
            if (ring_buffer_get(&usart2_tx_buffer, &data) == SUCCESS) {
                USART2->TDR = data;
            } else {
                USART2->CR1 &= ~USART_CR1_TXEIE_TXFNFIE;
            }
        }
    }
}

void usart3_irq_handler() {
    uint32_t isr = USART3->ISR;

    // Receive ISR
    if (isr & USART_ISR_RXNE_RXFNE) { // Read data register not empty
        uint8_t data = (uint8_t)(USART3->RDR); // Auto clears the RX ISR
        ring_buffer_put(&usart3_rx_buffer, data);
    }

    // IDLE line detected (Received a full message and now the line is idle)
    if (isr & USART_ISR_IDLE) {
        USART3->ICR = USART_ICR_IDLECF;
        usart3_msg_received = true;
    }

    // Transmit ISR
    if (isr & USART_ISR_TXE_TXFNF) {
        if (USART3->CR1 & USART_CR1_TXEIE_TXFNFIE) {
            uint8_t data;
            if (ring_buffer_get(&usart3_tx_buffer, &data) == SUCCESS) {
                USART3->TDR = data;
            } else {
                USART3->CR1 &= ~USART_CR1_TXEIE_TXFNFIE;
            }
        }
    }
}

/**
 * Transmits a single character over USART2.
 * @param c Character to transmit.
 */
void usart2_transmit_char(char c) {
    while(ring_buffer_put(&usart2_tx_buffer, c) != SUCCESS);
    USART2->CR1 |= USART_CR1_TXEIE_TXFNFIE; // Signal the USART2 peripheral to send the byte.
}

/**
 * Transmits a single character over USART3.
 * @param c Character to transmit.
 */
void usart3_transmit_char(char c) {
    while(ring_buffer_put(&usart3_tx_buffer, c) != SUCCESS);
    USART3->CR1 |= USART_CR1_TXEIE_TXFNFIE; // Signal the USART3 peripheral to send the byte.
}

int main(void) {
    if (system_clock_init() != SUCCESS) {
        system_panic();
    }
    led4_init();
    usart2_init();
    usart3_init();

    while (1) {
        if (usart2_msg_received) {
            usart2_msg_received = false;
            const char* prefix = "[Echo]: ";
            while(*prefix) usart2_transmit_char(*prefix++);
            uint8_t tx_byte;
            while (ring_buffer_get(&usart2_rx_buffer, &tx_byte) == SUCCESS) {
                usart2_transmit_char(tx_byte);
            }
        }

        if (usart3_msg_received) {
            usart3_msg_received = false;
            const char* prefix = "[Echo]: ";
            while(*prefix) usart3_transmit_char(*prefix++);
            uint8_t tx_byte;
            while (ring_buffer_get(&usart3_rx_buffer, &tx_byte) == SUCCESS) {
                usart3_transmit_char(tx_byte);
            }
        }
    }
}
