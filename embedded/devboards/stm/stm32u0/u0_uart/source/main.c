/*
 * Simple STM32U0 UART test.
 * Prints a "Hello world" message over uart, then starts echoing back any sent
 * character data.
 * 
 * (read ./../u0_blinky/readme.md for tool setup)
 * 
 * Build:
 * arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -g -O0 -T linker_script.ld -nostdlib -o uart_test.elf ./source/main.c ./source/stm32u083xx.h ./source/startup.s
 * 
 * Flash:
 * pyocd flash --pack ./../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx ./uart_test.elf
 */

#include <stdint.h>
#include "stm32u083xx.h"

void delay(volatile uint32_t count) {
    while(count--) {
        __asm("nop");
    }
}

void system_clock_init_16mhz(void) {
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));
    RCC->CFGR &= ~RCC_CFGR_SW; 
    RCC->CFGR |= RCC_CFGR_SW_0; 
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_0);
}

void usart2_init_115200(void) {
    // Enable USART2 Clock (APB1)
    RCC->APBENR1 |= RCC_APBENR1_USART2EN;
    // Configure multi-purpose pins (PA2/PA3) to USART
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |= (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);
    GPIOA->AFR[0] |= ( (7UL << GPIO_AFRL_AFSEL2_Pos) | (7UL << GPIO_AFRL_AFSEL3_Pos) );
    // Clock = 16MHz, Baud = 115200
    // BRR = 16,000,000 / 115,200 = 138.88 -> 139
    USART2->BRR = 139;
    USART2->CR1 = (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE); // Transmit (TE), Receive (RE)
}

void uart_send_char(char c) {
    // wait until the transfer byte buffer has sent any previosu data
    while (!(USART2->ISR & USART_ISR_TXE_TXFNF)) { // TXE = Transmission data register is Empty
        // do nothing
    }
    USART2->TDR = c;
}

void uart_send_str(const char* str) {
    int i = 0;
    while (str[i] != '\0') {
        char current_char = str[i];
        uart_send_char(current_char);
        i++;
    }
}

int uart_read_char_nonblocking(char *c) {
    // Check Read Data Register Not Empty (RXNE)
    if (USART2->ISR & USART_ISR_RXNE_RXFNE) {
        *c = (char)(USART2->RDR);
        return 1;
    }
    return 0;
}

int main(void) {
    system_clock_init_16mhz();

    // Initialize the GPIO A controller
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // Initialize the LED (GPIO A - pin 5)
    GPIOA->MODER &= ~GPIO_MODER_MODE5;
    GPIOA->MODER |= GPIO_MODER_MODE5_0;

    // Turn on the LED
    GPIOA->ODR |= GPIO_ODR_OD5;

    usart2_init_115200();

    uart_send_str("\r\nSTM32U0 Minimal UART hello world!\r\n");

    char rx_byte;
    
    while (1) {
        //GPIOA->ODR ^= GPIO_ODR_OD5; // uncomment for blinking led

        // Simple echo test
        if (uart_read_char_nonblocking(&rx_byte)) {
            uart_send_char(rx_byte); 
            if (rx_byte == '\r') {
                uart_send_char('\n');
            }
        }
        
        delay(100000);
    }
}
