/**
 * @file main.c
 * @brief Simple STM32U0 UART DMA test.
 *
 * This program configures the system clock to 56MHz and initializes
 * USART2 using DMA for rx/tx to free up the main execution thread.
 * USART2 is configured with fifo buffers and a circular rx dma buffer.
 *
 * The USART2 phy/hal layer holds raw byte data in the circular dma buffer.
 * The data link layer scans over the circular dma buffer on certain events
 * to packetize/frame the raw bytes into messages and places them on a received
 * message queue for the application/business logic to process.
 * The application/business logic for this test is to echo back the data.
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
 * build.py
 *
 * Flash:
 * pyocd flash --pack ./../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx ./u0_uart_dma.elf
 *
 * Test:
 * * Connect to USART2 (Virtual COMM or PA2 and PA3 if sodler bridge removed).
 * * Run "test.py" to send messages over either comm port (ex: test.py "the quick brown fox jumped over the lazy dog")
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <stm32u083xx.h>


/** RM0503 - Section 10.3.2 DMAMUX Mapping */
#define DMAMUX_USART2_RX 71
#define DMAMUX_USART2_TX 72

/**
 * Number of cycles to wait when waiting for a register enable request
 * before giving up in failure.
 */
#define ENABLE_TIMEOUT_CYCLES ((uint16_t)(0xFFFF)) // 65,635 cycles

#define UART_BUFFER_SIZE        256 // Must be power of 2 for masking math (avoids modulos/division)
#define MAX_PAYLOAD_SIZE        64
#define PREAMBLE_BYTE           0x55
#define PREAMBLE2_BYTE          0xAA
#define MAX_RX_QUEUE            8
#define MAX_TX_QUEUE            8
/** Maximum number of uart stream bytes to process per tick. */
#define MAX_UART_STREAM_BYTES_PER_TICK  64

/** Standard return type */
typedef enum {
    SUCCESS,
    FAILURE
} result_t;

typedef struct {
    uint8_t preamble0;
    uint8_t preamble1;
    uint8_t id;
    uint8_t length;
    uint8_t payload[MAX_PAYLOAD_SIZE];
    uint8_t crc;
} message_t;

typedef struct uart_s {
    USART_TypeDef       *uart;
    DMA_Channel_TypeDef *dma_rx;
    DMA_Channel_TypeDef *dma_tx;
    uint32_t            tx_dma_flag_clear_mask;
    volatile uint8_t rx_dma_buffer[UART_BUFFER_SIZE];
    volatile uint8_t tx_dma_buffer[UART_BUFFER_SIZE];
    uint16_t rx_tail_ptr;       // Where CPU is reading in RX buffer
    volatile bool rx_data_ready;
    // Metrics
    volatile uint32_t overrun_errors;
    volatile uint32_t noise_errors;
    volatile uint32_t frame_errors;
} uart_t;

typedef struct framer_s {
    uart_t* uart;
    uint16_t rx_index;
    message_t current_msg;
    message_t rx_queue[MAX_RX_QUEUE];
    uint8_t rx_queue_head;
    uint8_t rx_queue_tail;

    message_t tx_queue[MAX_TX_QUEUE];
    uint8_t tx_queue_head;
    uint8_t tx_queue_tail;

    uint32_t rx_dropped_messages;
    uint32_t rx_crc_mismatches;
    uint32_t tx_dropped_messages;
} data_link_t;


volatile uint32_t ms_ticks = 0;

uart_t usart2 = {
    .uart = USART2,
    .dma_rx = DMA1_Channel1,
    .dma_tx = DMA1_Channel2,
    .tx_dma_flag_clear_mask = DMA_IFCR_CGIF2
};
data_link_t usart2_link = {
    .uart = &usart2
};

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
    // TODO: write a reboot counter to flash somewhere?
    NVIC_SystemReset();
}

// MARK: INIT

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
            return(FAILURE);
        }
    }

    // Flash Latency Configuration
    FLASH->ACR &= ~FLASH_ACR_LATENCY; // Clear previous flash wait states
    FLASH->ACR |= FLASH_ACR_LATENCY_2WS; // 2 Wait States, needed for 56 MHz

    // Verify Latency was set
    if ((FLASH->ACR & FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_2WS) {
        return(FAILURE);
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
    uint16_t pll_ready_timeout = ENABLE_TIMEOUT_CYCLES;
    while (!(RCC->CR & RCC_CR_PLLRDY)) { // Waint until the PLL is ready
        if (pll_ready_timeout-- == 0) {
            return(FAILURE);
        }
    }

    // Configure SYSCLOCK (set to PLL)
    RCC->CFGR &= ~RCC_CFGR_SW;          // Clear previous SYSCLOCK settings
    RCC->CFGR |= (RCC_CFGR_SW_0 | RCC_CFGR_SW_1); // Set SYSCLOCK as PLL
    uint16_t sys_clock_config_timeout = ENABLE_TIMEOUT_CYCLES;
    while ((RCC->CFGR & RCC_CFGR_SWS) != (RCC_CFGR_SWS_0 | RCC_CFGR_SWS_1)) {
        if (sys_clock_config_timeout-- == 0) {
            return(FAILURE);
        }
    }
    return(SUCCESS);
}

/**
 * Configures the SYSTICK to interrupt every 1ms.
 */
void systick_init(void) {
    SysTick_Config(56000000 / 1000); // Reload Value = (56,000,000 / 1000) - 1 = 55,999
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

void dma1_init() {
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    (void)RCC->AHBENR;
    RCC->AHBENR |= (1 << 2);
    (void)RCC->AHBENR; // Read back delay
    // Individual channels are configured in the peripherals they are a part of.
}

uint32_t get_tick(void) {
    return ms_ticks;
}

// MARK: NET PHYSICAL

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

    // Configure USART2 Peripheral
    USART2->CR1 &= ~USART_CR1_UE; // Disable USART2

    USART2->BRR = 486; // Clock = 56MHz, Baud = 115200, BRR = 56,000,000 / 115,200 = 486
    USART2->CR1 |= USART_CR1_FIFOEN; // 8 byte FIFO buffer between uart and dma to minimize lost data
    USART2->CR1 |= USART_CR1_RE; // Receiver Enable
    USART2->CR1 |= USART_CR1_TE; // Transmitter Enable
    USART2->CR1 |= USART_CR1_IDLEIE; // IDLE Interrupt Enable
    USART2->CR3 |= USART_CR3_EIE; // Error Interrupt Enable  (Overrun, Noise, Framing)
    USART2->CR3 |= USART_CR3_DMAR; // DMA Enable Receiver
    USART2->CR3 |= USART_CR3_DMAT; // DMA Enable Transmitter

    // DMA Channel 1 RX
    DMAMUX1_Channel0->CCR = (DMAMUX_USART2_RX << DMAMUX_CxCR_DMAREQ_ID_Pos);  //
    //DMAMUX1_Channel0->CCR |= DMAMUX_CxCR_EGE;   // Event generation interrupt enable
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;      // Disable RX channel
    DMA1_Channel1->CPAR = (uint32_t)&(USART2->RDR);
    DMA1_Channel1->CMAR = (uint32_t)usart2.rx_dma_buffer;
    DMA1_Channel1->CNDTR = UART_BUFFER_SIZE;
    DMA1_Channel1->CCR = DMA_CCR_CIRC;      // Circular mode
    DMA1_Channel1->CCR |= DMA_CCR_MINC;     // Memory increment mode
    DMA1_Channel1->CCR |= DMA_CCR_PL_1;
    DMA1_Channel1->CCR |= DMA_CCR_TCIE;     // Transfer complete interrupt enable
    DMA1_Channel1->CCR |= DMA_CCR_HTIE;     // Half Transfer interrupt enable
    DMA1_Channel1->CCR |= DMA_CCR_EN;       // Channel enable

    // DMA Channel 2 TX
    DMAMUX1_Channel1->CCR = (DMAMUX_USART2_TX << DMAMUX_CxCR_DMAREQ_ID_Pos);
    //DMAMUX1_Channel1->CCR |= DMAMUX_CxCR_EGE; // Event Gen Enable
    DMA1_Channel2->CCR &= ~DMA_CCR_EN; // Ensure disabled before config
    DMA1_Channel2->CPAR = (uint32_t)&(USART2->TDR);
    DMA1_Channel2->CMAR = (uint32_t)usart2.tx_dma_buffer;
    DMA1_Channel2->CCR = DMA_CCR_DIR; // Data transfer direction
    DMA1_Channel2->CCR |= DMA_CCR_MINC; // Memory increment mode
    //DMA1_Channel2->CCR |= DMA_CCR_TCIE;
    //DMA1_Channel2->CCR |= DMA_CCR_EN; - Using linear mode, only enable for short bursts when transmitting.
    USART2->CR1 |= USART_CR1_UE; // USART Enable

    // Enable interrupts
    NVIC_EnableIRQ(USART2_LPUART2_IRQn); // USART2_IRQn
    NVIC_SetPriority(USART2_LPUART2_IRQn, 1); // (0 - 10, 0 = most important)
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
    NVIC_SetPriority(DMA1_Channel1_IRQn, 1);
}

// MARK: IRQ HANDLERS

void SysTick_Handler() {
    ms_ticks++;
}

void DMA1_Channel1_IRQHandler() {
    // Check for Half Transfer
    if (DMA1->ISR & DMA_ISR_HTIF1) {
        usart2.rx_data_ready = true;
        DMA1->IFCR = DMA_IFCR_CHTIF1;
    }

    // Check for Transfer Complete
    if (DMA1->ISR & DMA_ISR_TCIF1) {
        usart2.rx_data_ready = true;
        DMA1->IFCR = DMA_IFCR_CTCIF1;
    }
}

void USART2_LPUART2_IRQHandler() {
    // IDLE line detected (Received a full message and now the line is idle)
    if (USART2->ISR & USART_ISR_IDLE) {
        usart2.rx_data_ready = true;
        USART2->ICR = USART_ICR_IDLECF;
    }

    // Overrun Error
    if (USART2->ISR & USART_ISR_ORE) {
        usart2.overrun_errors += 1;
        USART2->ICR = USART_ICR_ORECF;
    }

    // Noise Error
    if (USART2->ISR & USART_ISR_NE) {
        usart2.noise_errors += 1;
        USART2->ICR = USART_ICR_NECF;
    }

    // Frame Error
    if (USART2->ISR & USART_ISR_FE) {
        usart2.frame_errors += 1;
        USART2->ICR = USART_ICR_FECF;
    }
}

// MARK: DATA LINK LAYER

/** Polynomial: 0x07 (SMBus/ATM) */
uint8_t crc8_byte(uint8_t crc, uint8_t data) {
    crc ^= data;
    for (int i = 0; i < 8; i++) {
        if (crc & 0x80) {
            crc = (crc << 1) ^ 0x07;
        } else {
            crc <<= 1;
        }
    }
    return crc;
}

/** Calculates CRC-8 (Polynomial: 0x07) over a buffer. */
uint8_t crc8(uint8_t *data, size_t len) {
    uint8_t crc = 0;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

bool crc_match(message_t* message) {
    uint8_t *start_ptr = &message->id;
    size_t check_len = 2 + message->length;
    uint8_t calculated = crc8(start_ptr, check_len);
    return (calculated == message->crc);
}

void link_process_rx_stream(data_link_t* link) {
    if (!link->uart->rx_data_ready) {
        return;
    }
    link->uart->rx_data_ready = false;
    uint16_t uart_buffer_mask = UART_BUFFER_SIZE - 1;
    uint16_t bytes_processed = 0; // Sentinel Counter
    while (bytes_processed < MAX_UART_STREAM_BYTES_PER_TICK) {
        uint16_t head = UART_BUFFER_SIZE - link->uart->dma_rx->CNDTR;
        uint16_t tail = link->uart->rx_tail_ptr;
        uint16_t available = (head - tail) & uart_buffer_mask;
        if (available < 4) {
            break; // Not enough to possibly have a message, wait for more data.
        }
        uint8_t p0 = link->uart->rx_dma_buffer[(tail + 0) & uart_buffer_mask]; // Preamble 1
        uint8_t p1 = link->uart->rx_dma_buffer[(tail + 1) & uart_buffer_mask]; // Preamble 2
        uint8_t id = link->uart->rx_dma_buffer[(tail + 2) & uart_buffer_mask];
        uint8_t payload_length = link->uart->rx_dma_buffer[(tail + 3) & uart_buffer_mask]; // Length Byte
        if (p0 != PREAMBLE_BYTE || p1 != PREAMBLE2_BYTE) {
            link->uart->rx_tail_ptr = (tail + 1) & uart_buffer_mask; // if the first bytes are not the preamble, we have garbage on the line, walk the tail forward.
            bytes_processed++;
            continue;
        }
        if (payload_length > MAX_PAYLOAD_SIZE) {
            // Impossible length. Treat as garbage.
            link->uart->rx_tail_ptr = (tail + 1) & uart_buffer_mask;
            bytes_processed++;
            continue;
        }
        uint16_t total_msg_size = 4 + payload_length + 1;
        if (available < total_msg_size) {
            bytes_processed++;
            return; // Not enough to possibly have a message, wait for more data.
        }
        uint8_t next_head = (link->rx_queue_head + 1) & (MAX_RX_QUEUE - 1);
        if (next_head == link->rx_queue_tail) {
            // Message queue full! Dropping the message!
            link->uart->rx_tail_ptr = (tail + total_msg_size) & uart_buffer_mask;
            bytes_processed += total_msg_size;
            link->rx_dropped_messages += 1;
            continue;
        }
        message_t* message = &link->rx_queue[link->rx_queue_head];
        uint8_t calculated_crc = 0;
        message->preamble0 = p0;
        message->preamble1 = p1;
        message->id = id;
        calculated_crc = crc8_byte(calculated_crc, id);
        message->length = payload_length;
        calculated_crc = crc8_byte(calculated_crc, payload_length);
        for (int i = 0; i < payload_length; i++) {
            uint8_t byte = link->uart->rx_dma_buffer[(tail + 4 + i) & uart_buffer_mask];
            message->payload[i] = byte;
            calculated_crc = crc8_byte(calculated_crc, byte);
        }
        uint8_t received_crc = link->uart->rx_dma_buffer[(tail + 4 + payload_length) & uart_buffer_mask];
        if (calculated_crc == received_crc) {
            link->rx_queue_head = next_head;
        } else {
            link->rx_crc_mismatches += 1;
        }
        link->uart->rx_tail_ptr = (tail + total_msg_size) & uart_buffer_mask;
    }
}

/**
 * Tries to work through the transmit queue, shoveling messages into the
 * uart's DMA.
 */
void link_handle_tx_queue(data_link_t* link) {
    if (link->tx_queue_head == link->tx_queue_tail) {
        return; // Nothing to send
    }
    // Using linear DMA, check if it's worked through the last transmit request.
    if (link->uart->dma_tx->CCR & DMA_CCR_EN) { // Channel enable
        if (!(DMA1->ISR & DMA_ISR_TCIF2)) {
            return;
        }
        link->uart->dma_tx->CCR &= ~DMA_CCR_EN;
    }

    message_t* msg = &link->tx_queue[link->tx_queue_tail];
    link->uart->dma_tx->CCR &= ~DMA_CCR_EN;

    uint16_t idx = 0;
    link->uart->tx_dma_buffer[idx++] = msg->preamble0;
    link->uart->tx_dma_buffer[idx++] = msg->preamble1;
    link->uart->tx_dma_buffer[idx++] = msg->id;
    link->uart->tx_dma_buffer[idx++] = msg->length;
    for(int i = 0; i < msg->length; i++) {
        link->uart->tx_dma_buffer[idx++] = msg->payload[i];
    }
    link->uart->tx_dma_buffer[idx++] = msg->crc;
    uint16_t total_dma_size = idx;

    link->uart->uart->ICR = USART_ICR_TCCF; // Transmission Complete Clear Flag
    DMA1->IFCR = link->uart->tx_dma_flag_clear_mask;

    link->uart->dma_tx->CMAR = (uint32_t)link->uart->tx_dma_buffer;
    link->uart->dma_tx->CPAR = (uint32_t)&(link->uart->uart->TDR);
    link->uart->dma_tx->CNDTR = total_dma_size;
    link->uart->dma_tx->CCR |= DMA_CCR_EN; // Enable Channel -> Starts Transmission!
    link->tx_queue_tail = (link->tx_queue_tail + 1) & (MAX_TX_QUEUE - 1);
}

/**
 * Checks the link's rx queue for any inbound messages to process.
 */
bool link_try_receive(data_link_t* link, message_t** out_message) {
    if (link->rx_queue_head == link->rx_queue_tail) {
        return false;
    }
    *out_message = &link->rx_queue[link->rx_queue_tail];
    uint8_t mask = MAX_RX_QUEUE - 1;
    link->rx_queue_tail = (link->rx_queue_tail + 1) & mask;
    return true;
}

/**
 * Attempts to find an empty slot in the transmit queue. If an empty
 * slot is found, a pointer is provided to the open slot. Once the
 * message to be transmitted is written, a corresponding "*_enqueue_commit"
 * call is required to finalize writing the message to the queue.
 */
bool link_tx_try_enqueue_reserve(data_link_t* link, message_t** out_msg_ptr) {
    uint8_t next_head = (link->tx_queue_head + 1) & (MAX_TX_QUEUE - 1);
    if (next_head == link->tx_queue_tail) {
        link->tx_dropped_messages++;
        return false;
    }
    *out_msg_ptr = &link->tx_queue[link->tx_queue_head];
    return true;
}

/**
 * Finalizes a "*_enqueue_reserve" call and write to the tx queue. Called
 * as part of a 2 step process, "*_enqueue_reserve" must be called first
 * and a complete message must be written the provided buffer before
 * this function is called.
 */
void link_tx_enqueue_commit(data_link_t* link) {
    uint8_t next_head = (link->tx_queue_head + 1) & (MAX_TX_QUEUE - 1);
    link->tx_queue_head = next_head;
}

// MARK: BUSINESS LOGIC

typedef struct {
    data_link_t* usart2_link;
} app_t;
app_t app = {0};
void app_init(app_t* app) {
    app->usart2_link = &usart2_link;
}

void app_link_process_messages(data_link_t *link) {
    link_process_rx_stream(&usart2_link);
    link_handle_tx_queue(&usart2_link);
    message_t* msg = NULL;
    if (!link_try_receive(link, &msg)) {
        return;
    }
    message_t* tx_msg = NULL;
    if (link_tx_try_enqueue_reserve(link,  &tx_msg)) {
        led4_toggle();
        //*tx_msg = *msg; // Echo back the message.
        tx_msg->preamble0 = PREAMBLE_BYTE;
        tx_msg->preamble1 = PREAMBLE2_BYTE;
        tx_msg->id = msg->id;
        tx_msg->length = msg->length;
        for(int i = 0; i < msg->length; i++) {
            tx_msg->payload[i] = msg->payload[i];
        }
        uint8_t *start_ptr = &tx_msg->id;
        size_t check_len = 2 + tx_msg->length;
        tx_msg->crc = crc8(start_ptr, check_len);
        link_tx_enqueue_commit(link);
    }
}

void app_process_messages(app_t* app) {
    app_link_process_messages(app->usart2_link);
}

// MARK: MAIN

int main(void) {
    if (system_clock_init() != SUCCESS) {
        system_panic();
    }

    systick_init();
    led4_init();
    dma1_init();
    usart2_init();
    app_init(&app);

    uint32_t last_led_toggle_tick = 0;
    const uint32_t led_interval = 500; // 500ms

    while (1) {
        app_process_messages(&app);

        uint32_t current_tick = get_tick();
        if ((current_tick - last_led_toggle_tick) >= led_interval) {
            led4_toggle();
            last_led_toggle_tick = current_tick;
        }
    }
}
