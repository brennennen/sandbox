/*
 * Simple STM32U0 flash test.
 * Small test that provides a shell over uart to peek and poke to flash.
 * Connect over the debugger virtual comm port, type "help" for a list of commands.
 * 
 * Init:
 * * Configures a 1ms "SysTick" using ARM's CMSIS.
 * 
 * Tasks:
 * * Toggles an LED on a ~1 second delay (visual monitor for hard fault/uncaught interrupts).
 * * Processes UART data on a ~1ms delay.
 * 
 * (read ./../u0_blinky/readme.md for tool setup)
 * 
 * Build:
 * arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -g -O0 -T linker_script.ld -nostdlib -o flash_test.elf -I./source ./source/main.c ./source/startup.s
 * 
 * Flash:
 * pyocd flash --pack ./../.temp/Keil.STM32U0xx_DFP.2.1.0.pack -t stm32u083rctx ./flash_test.elf
 * 
 * Connect:
 * * windows: plink -serial COM3 -sercfg 115200,8,n,1,N
 * * linux: picocom -b 115200 /dev/ttyACM0
 *
 * Example commands:
 * * peek 0x0803c000 - Reads the first 4 bytes of page 120
 * * poke 0x0803c000 0x00000012 - Writes 12 to the first 4 bytes of page 120
 * * erase_page 120 - Erases page 120
 * * peek_page 120 - Reads data around the start and end of page 120
 *
 */

#include <stdint.h>
#include "stm32u083xx.h"
#include "core_cm0plus.h"


#define FLASH_KEY1 0x45670123U
#define FLASH_KEY2 0xCDEF89ABU
#define PAGE_SIZE 2048U
#define CMD_BUFFER_SIZE 64


void lazy_delay(volatile uint32_t count) {
    while(count--) {
        __asm("nop");
    }
}

void systick_delay_ms(uint32_t ms) {
    SysTick->LOAD = 16000 - 1; // Program reload value (16000 cycles = 1ms at 16MHz)
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk; // Enable SysTick
    for (uint32_t i = 0; i < ms; i++) {
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    }
    SysTick->CTRL = 0; // Disable SysTick
}

void system_clock_init_16mhz(void) {
    RCC->CR |= RCC_CR_HSION;
    while (!(RCC->CR & RCC_CR_HSIRDY));
    RCC->CFGR &= ~RCC_CFGR_SW; 
    RCC->CFGR |= RCC_CFGR_SW_0; 
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_0);
}

void led4_init(void) {
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN; // Initialize the GPIO A controller
    // Initialize the LED (GPIO A - pin 5)
    GPIOA->MODER &= ~GPIO_MODER_MODE5;
    GPIOA->MODER |= GPIO_MODER_MODE5_0;
}

void led4_on(void) {
    GPIOA->ODR |= GPIO_ODR_OD5;
}

void led4_off(void) {
    GPIOA->ODR &= GPIO_ODR_OD5;
}

void led4_toggle(void) {
    GPIOA->ODR ^= GPIO_ODR_OD5;
}

void usart2_init_115200(void) {
    RCC->APBENR1 |= RCC_APBENR1_USART2EN; // Enable USART2 Clock (APB1)
    // Configure multi-purpose pins (PA2/PA3) to USART
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
    GPIOA->MODER |= (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);
    GPIOA->AFR[0] |= ( (7UL << GPIO_AFRL_AFSEL2_Pos) | (7UL << GPIO_AFRL_AFSEL3_Pos) );
    // Clock = 16MHz, Baud = 115200
    // BRR = 16,000,000 / 115,200 = 138.88 -> 139
    USART2->BRR = 139;
    USART2->CR1 = (USART_CR1_UE | USART_CR1_TE | USART_CR1_RE); // ransmit (TE), Receive (RE)
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

void uart_send_uint32(uint32_t val) {
    char buffer[12];
    int i = 0;
    if (val == 0) {
        uart_send_char('0');
        return;
    }
    while (val > 0) {
        buffer[i++] = (val % 10) + '0';
        val /= 10;
    }
    while (--i >= 0) {
        uart_send_char(buffer[i]);
    }
}

void uart_send_hex8(uint8_t val) {
    uint8_t nibble = (val >> 4) & 0xF;
    uart_send_char(nibble < 10 ? '0' + nibble : 'A' + (nibble - 10));
    nibble = val & 0xF;
    uart_send_char(nibble < 10 ? '0' + nibble : 'A' + (nibble - 10));
}

void uart_send_hex32(uint32_t val) {
    uart_send_str("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t nibble = (val >> i) & 0xF;
        if (nibble < 10) {
            uart_send_char('0' + nibble);
        } else {
            uart_send_char('A' + (nibble - 10));
        }
    }
}

int uart_read_char_nonblocking(char *c) {
    if (USART2->ISR & USART_ISR_RXNE_RXFNE) { // Check Read Data Register Not Empty (RXNE)
        *c = (char)(USART2->RDR);
        return 1;
    }
    return 0;
}

int str_starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*prefix++ != *str++) return 0;
    }
    return 1;
}

uint32_t parse_dec_str(char *str) {
    uint32_t result = 0;
    while (*str) {
        char c = *str++;
        if (c >= '0' && c <= '9') {
            result = result * 10 + (c - '0');
        } else {
            break; 
        }
    }
    return result;
}

uint32_t parse_hex_str(char *str) {
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2;
    }

    uint32_t result = 0;
    while (*str) {
        char c = *str++;
        result = result << 4; // Shift 4 bits for next hex digit
        if (c >= '0' && c <= '9') {
            result |= (c - '0');
        }
        else if (c >= 'a' && c <= 'f') {
            result |= (c - 'a' + 10);
        }
        else if (c >= 'A' && c <= 'F') {
            result |= (c - 'A' + 10);
        }
        else {
            // Stop parsing on non-hex char (like space or null)
            result >>= 4; // Revert the shift
            break; 
        }
    }
    return result;
}

void flash_unlock(void) {
    // If locked, feed the keys
    if (FLASH->CR & FLASH_CR_LOCK) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

void flash_lock(void) {
    FLASH->CR |= FLASH_CR_LOCK;
}

// Programs a Double-Word (64-bit).
// We set the target 'addr' to 'data', and the neighbor (addr+4) to 0x00000000
void flash_program(uint32_t addr, uint32_t data) {
    while (FLASH->SR & FLASH_SR_BSY1) {
        // do nothing
    }
    flash_unlock();
    FLASH->CR |= FLASH_CR_PG;

    // Write Data (Must be Double-Word / 2x 32-bit for STM32U0)
    // We assume 'addr' is the start of the double-word (8-byte aligned)
    *(volatile uint32_t*)addr = data;
    __asm("dmb"); // Data Memory Barrier to ensure ordering
    *(volatile uint32_t*)(addr + 4) = 0x00000000; 
    while (FLASH->SR & FLASH_SR_BSY1) {
        // do nothing
    }

    if (FLASH->SR & FLASH_SR_PROGERR) {
        uart_send_str(" [Error: PROGERR] ");
        FLASH->SR |= FLASH_SR_PROGERR; 
    }
    FLASH->CR &= ~FLASH_CR_PG;
    flash_lock();
}

void flash_erase_page(uint32_t page_num) {
    while (FLASH->SR & FLASH_SR_BSY1);
    flash_unlock();
    FLASH->CR |= FLASH_CR_PER;
    FLASH->CR &= ~FLASH_CR_PNB_Msk; 
    FLASH->CR |= (page_num << FLASH_CR_PNB_Pos);
    FLASH->CR |= FLASH_CR_STRT; // Start Erase (STRT)
    while (FLASH->SR & FLASH_SR_BSY1);

    if (FLASH->SR & (FLASH_SR_PROGERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR)) {
        uart_send_str(" [Error: Erase Failed] ");
        FLASH->SR |= (FLASH_SR_PROGERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR);
    }

    FLASH->CR &= ~FLASH_CR_PER;
    flash_lock();
}

void flash_erase_page_address(uint32_t address) {
    uint32_t page_num = (address - FLASH_BASE) / PAGE_SIZE;
    flash_erase_page(page_num);
}

void print_help() {
    uart_send_str("Available Commands:\r\n");
    uart_send_str("  help           - Show this list\r\n");
    uart_send_str("  map            - Show memory map\r\n");
    uart_send_str("  peek <addr>    - Read memory (hex address)\r\n");
    uart_send_str("  peek_page <page_num> - Reads memory at the start and end of a page\r\n");
    uart_send_str("  poke <addr> <val> - Read memory (hex address)\r\n");
    uart_send_str("  erase <page_num> - Erases a page (0 - 127)\r\n");
}

void print_memory_map() {
    uart_send_str("Memory map:\r\n");
    uart_send_str("0x2000 0000 - 0x2002 9FFF (40 Kbytes): SRAM\r\n");
    uart_send_str("0x1FFF 7000 - 0x1FFF 7FFF (4 Kbytes): Option bytes\r\n");
    uart_send_str("0x1FFF 6C00 - 0x1FFF 6FFF (1 Kbyte): Engineering bytes\r\n");
    uart_send_str("0x1FFF 6800 - 0x1FFF 6BFF (1 Kbyte): OTP\r\n");
    uart_send_str("0x1FFF 0000 - 0x1FFF 67FF (26 Kbytes): System Memory\r\n");
    uart_send_str("0x0800 0000 - 0x0803 FFFF (256 Kbytes): Main flash (2Kb pages)\r\n");
    uart_send_str("0x0000 0000 - 0x0003 FFFF (256 Kbytes): Aliased boot memory\r\n");
}

void print_peripheral_memory_map() {
    uart_send_str("Peripheral memory map:\r\n");
    uart_send_str("0xE000 0000 - 0xE00F FFFF (1 Mbyte): Internal\r\n");
    uart_send_str("0x5000 1400 - 0x5000 17FF (1 Kbyte): GPIOF\r\n");
    uart_send_str("0x5000 1000 - 0x5000 13FF (1 Kbyte): GPIOE\r\n");
    // ...
    uart_send_str("0x5000 0000 - 0x5000 03FF (1 Kbyte): GPIOA\r\n");
    // ...
    uart_send_str("0x4002 0000 - 0x4002 23FF (1 Kbyte): FLASH\r\n");
    // ...
    // TODO: RM0503 - Table 4 (Section 2.2.2)
}

char *str_find_char(char *str, char c) {
    while (*str) {
        if (*str == c) {
            return str;
        }
        str += 1;
    }
    return 0; // NULL
}

void peek_command(char* command) {
    char *arg = command + 5; // Skip "peek "
    uint32_t address = parse_hex_str(arg);
    uint32_t value = *(volatile uint32_t*)address;
    uart_send_str("peek [");
    uart_send_hex32(address);
    uart_send_str("] = ");
    uart_send_hex32(value);
    uart_send_str("\r\n");
}

void peek_page_command(char* command) {
    char *arg = command + 10; // Skip "peek_page "
    uint32_t page_num = parse_dec_str(arg);
    if (page_num > 127) {
        uart_send_str("error: page_num must be 0 - 127\r\n");
        return;
    }
    uint32_t start_address = 0x08000000 + (page_num * 2048);
    uint32_t end_address = start_address + 2048 - 16;
    uart_send_str("peek_page: ");
    uart_send_uint32(page_num);
    uart_send_str(" (address: ");
    uart_send_hex32(start_address);
    uart_send_str(")\r\n");
    
    volatile uint8_t *ptr = (volatile uint8_t*)start_address;
    for(int i=0; i < 16; i++) {
        uart_send_hex8(ptr[i]);
        uart_send_char(' ');
    }
    uart_send_str("\r\n(...)\r\n");
    ptr = (volatile uint8_t*)end_address;
    for(int i=0; i < 16; i++) {
        uart_send_hex8(ptr[i]);
        uart_send_char(' ');
    }
    uart_send_str("\r\n");
}

void poke_command(char* command) {
    char *addr_str = command + 5;  // Skip "poke "
    char *space_ptr = str_find_char(addr_str, ' ');
    if (space_ptr) {
        uint32_t address = parse_hex_str(addr_str);
        uint32_t value  = parse_hex_str(space_ptr + 1); // Value starts 1 char after the space
        uart_send_str("poke: ");
        uart_send_hex32(address);
        uart_send_str(", ");
        uart_send_hex32(value);
        uart_send_str("...");
        flash_program(address, value);
        uart_send_str("Done.\r\n");
    } else {
        uart_send_str("Error: Format is 'poke <addr> <val>'\r\n");
    }
}

void erase_command(char* command) {
    char *arg = command + 6; // Skip "erase "
    uint32_t page_num = parse_dec_str(arg);
    if (page_num > 127) {
        uart_send_str("error: page_num must be 0 - 127\r\n");
        return;
    }
    uint32_t address = FLASH_BASE + (page_num * 2048);
    uart_send_str("erase: ");
    uart_send_uint32(page_num);
    uart_send_str(" (address: ");
    uart_send_hex32(address);
    uart_send_str(")\r\n");
    flash_erase_page(page_num);
    uart_send_str("Done.\r\n");
}

void process_command(char *command) {
    if (str_starts_with(command, "help")) {
        print_help();
    } else if (str_starts_with(command, "map")) {
        print_memory_map();
        print_peripheral_memory_map();
    } else if (str_starts_with(command, "peek ")) {
        peek_command(command);
    } else if (str_starts_with(command, "peek_page ")) {
        peek_page_command(command);
    } else if (str_starts_with(command, "poke ")) {
        poke_command(command);
    } else if (str_starts_with(command, "erase ")) {
        erase_command(command);
    } else {
        uart_send_str("Unknown command.\r\n");
    }
}

void process_uart_byte(
    char rx_byte, 
    char* cmd_buffer, 
    int cmd_buffer_size, 
    int* cmd_index
) {
    if (rx_byte == '\r') { // newline
        uart_send_str("\r\n");
        cmd_buffer[*cmd_index] = 0;
        process_command(cmd_buffer);
        *cmd_index = 0;
    } else if (rx_byte == 0x08 || rx_byte == 0x7F) { // backspace
        if (*cmd_index > 0) {
            *cmd_index -= 1;
            uart_send_str("\b \b");
        }
    } else {
        if (*cmd_index < CMD_BUFFER_SIZE - 1) { // normal characters
            cmd_buffer[(*cmd_index)++] = rx_byte;
            uart_send_char(rx_byte);
        }
    }
}

int main(void) {
    system_clock_init_16mhz();
    led4_init();
    led4_on();
    usart2_init_115200();

    uart_send_str("\r\n");
    uart_send_str("STM32U0 Minimal flash test start...\r\n");
    uart_send_str("(type 'help' for command list)\r\n");
    print_help();

    char rx_byte = '\0';
    static char cmd_buffer[CMD_BUFFER_SIZE];
    int cmd_index = 0;

    uint32_t frame_tick = 0;
    uint32_t led_blink_tick = 0;
    while (1) {
        if (led_blink_tick >= 1000) {
            led4_toggle();
            led_blink_tick = 0;
        }

        if (uart_read_char_nonblocking(&rx_byte)) {
            process_uart_byte(rx_byte, cmd_buffer, sizeof(cmd_buffer), &cmd_index);
        }

        systick_delay_ms(1);
        frame_tick += 1;
        led_blink_tick += 1;
    }
}
