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
 */

#include <stdint.h>
#include "stm32u083xx.h"
#include "core_cm0plus.h"

#define FLASH_KEY1 0x45670123U
#define FLASH_KEY2 0xCDEF89ABU

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

void uart_print_hex(uint32_t val) {
    uart_send_str("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uint8_t nibble = (val >> i) & 0xF;
        if (nibble < 10) uart_send_char('0' + nibble);
        else uart_send_char('A' + (nibble - 10));
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

int str_starts_with(const char *str, const char *prefix) {
    while (*prefix) {
        if (*prefix++ != *str++) return 0;
    }
    return 1;
}

// Convert Hex String ("40001000") to Uint32
// TODO: support "0x40001000" and strip out "0x".
uint32_t parse_hex_str(char *str) {
    uint32_t result = 0;
    while (*str) {
        char c = *str++;
        result <<= 4; // Shift 4 bits for next hex digit
        if (c >= '0' && c <= '9') result |= (c - '0');
        else if (c >= 'a' && c <= 'f') result |= (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') result |= (c - 'A' + 10);
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
    while (FLASH->SR & FLASH_SR_BSY1);
    flash_unlock();
    FLASH->CR |= FLASH_CR_PG;

    // Write Data (Must be Double-Word / 2x 32-bit for STM32U0)
    // We assume 'addr' is the start of the double-word (8-byte aligned)
    *(volatile uint32_t*)addr = data;
    __asm("dmb"); // Data Memory Barrier to ensure ordering
    *(volatile uint32_t*)(addr + 4) = 0x00000000; 
    while (FLASH->SR & FLASH_SR_BSY1);

    if (FLASH->SR & FLASH_SR_PROGERR) {
        uart_send_str(" [Error: PROGERR] ");
        FLASH->SR |= FLASH_SR_PROGERR; 
    }
    FLASH->CR &= ~FLASH_CR_PG;
    flash_lock();
}

// Automated test: Peek -> Check Empty -> Poke -> Verify
void cmd_flash_test(char *args) {
    uint32_t addr;
    
    if (args[0] == '\0') {
        // End of 256KB flash (0x08040000) minus 8 bytes
        addr = 0x0803FFF8; 
        uart_send_str("Using default addr: ");
    } else {
        addr = parse_hex_str(args);
        uart_send_str("Target addr: ");
    }
    
    // Force 8-byte alignment (required for double-word programming)
    addr &= ~0x7; 
    
    uart_print_hex(addr);
    uart_send_str("\r\n");

    volatile uint32_t *ptr = (volatile uint32_t*)addr;

    // Peek (Pre-check)
    uart_send_str("1. Peek (Pre):  ");
    uint32_t val_low = ptr[0];
    uint32_t val_high = ptr[1];
    
    uart_print_hex(val_low);
    uart_send_str(" ");
    uart_print_hex(val_high);

    if (val_low != 0xFFFFFFFF || val_high != 0xFFFFFFFF) {
        uart_send_str("\r\n   [WARN] Target not empty (FFFFFFFF). Write might fail or mix bits.\r\n");
    } else {
        uart_send_str(" (Empty/OK)\r\n");
    }

    // Poke
    uint32_t pattern = 0xCAFEBABE;
    uart_send_str("2. Poking:      ");
    uart_print_hex(pattern);
    uart_send_str(" ... ");
    
    // This writes 'pattern' to addr, and 0x00000000 to addr+4
    flash_program(addr, pattern); 
    uart_send_str("Done.\r\n");

    // Peek (Verify)
    uart_send_str("3. Peek (Post): ");
    val_low = ptr[0];
    val_high = ptr[1];

    uart_print_hex(val_low);
    uart_send_str(" ");
    uart_print_hex(val_high);

    if (val_low == pattern && val_high == 0x00000000) {
        uart_send_str(" -> [PASS]\r\n");
    } else {
        uart_send_str(" -> [FAIL]\r\n");
    }
}

#define CMD_BUFFER_SIZE 64

void print_help() {
    uart_send_str("Available Commands:\r\n");
    uart_send_str("  help           - Show this list\r\n");
    uart_send_str("  map            - Show memory map\r\n");
    uart_send_str("  peek <addr>    - Read memory (hex addr)\r\n");
}

void print_memory_map() {
    uart_send_str("Memory map:\r\n");
    uart_send_str("0x2000 0000 - 0x2002 9FFF (40 Kbytes): SRAM\r\n");
    uart_send_str("0x1FFF 7000 - 0x1FFF 7FFF (4 Kbytes): Option bytes\r\n");
    uart_send_str("0x1FFF 6C00 - 0x1FFF 6FFF (1 Kbyte): Engineering bytes\r\n");
    uart_send_str("0x1FFF 6800 - 0x1FFF 6BFF (1 Kbyte): OTP\r\n");
    uart_send_str("0x1FFF 0000 - 0x1FFF 67FF (26 Kbytes): System Memory\r\n");
    uart_send_str("0x0800 0000 - 0x0803 FFFF (256 Kbytes): Main flash\r\n");
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
        if (*str == c) return str;
        str++;
    }
    return 0; // NULL
}

void process_command(char *cmd) {
    uart_send_str("process_command: ");
    uart_send_str(cmd);
    uart_send_str("\n");
    if (str_starts_with(cmd, "help")) {
        print_help();
    } if (str_starts_with(cmd, "map")) {
        print_memory_map();
        print_peripheral_memory_map();
    } else if (str_starts_with(cmd, "peek ")) {
        char *addr_str = cmd + 5; // Skip "peek "
        uint32_t addr = parse_hex_str(addr_str);
        // TODO: address sanity checks
        // WARNING: Reading invalid memory will cause a HardFault!
        uint32_t value = *(volatile uint32_t*)addr;
        uart_send_str("Read [");
        uart_print_hex(addr);
        uart_send_str("] = ");
        uart_print_hex(value);
        uart_send_str("\r\n");
    } else if (str_starts_with(cmd, "poke ")) {
        char *addr_str = cmd + 5;  // Skip "poke "
        char *space_ptr = str_find_char(addr_str, ' ');
        if (space_ptr) {
            uint32_t addr = parse_hex_str(addr_str);
            // Value starts 1 char after the space
            uint32_t val  = parse_hex_str(space_ptr + 1);

            uart_send_str("Writing [");
            uart_print_hex(addr);
            uart_send_str("] <= ");
            uart_print_hex(val);
            uart_send_str("... ");
            
            flash_program(addr, val);
            
            uart_send_str("Done.\r\n");
        } else {
            uart_send_str("Error: Format is 'poke <addr> <val>'\r\n");
        }
    } else if (str_starts_with(cmd, "flash_test")) {
        char *arg = cmd + 10;
        while(*arg == ' ') arg++;
        cmd_flash_test(arg);
    } else if (cmd[0] != 0) {
        uart_send_str("Unknown command.\r\n");
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
        // Blink the LED every 1 second.
        if (led_blink_tick >= 1000) {
            led4_toggle();
            led_blink_tick = 0;
        }

        // Check for uart data every 1ms
        if (uart_read_char_nonblocking(&rx_byte)) {
            if (rx_byte == '\r') {
                uart_send_str("\r\n");
                cmd_buffer[cmd_index] = 0;
                process_command(cmd_buffer);
                cmd_index = 0;
            } else if (rx_byte == 0x08 || rx_byte == 0x7F) { // backspace
                if (cmd_index > 0) {
                    cmd_index--;
                    uart_send_str("\b \b");
                }
            } else {
                if (cmd_index < CMD_BUFFER_SIZE - 1) {
                    cmd_buffer[cmd_index++] = rx_byte;
                    uart_send_char(rx_byte);
                }
            }
        }

        systick_delay_ms(1);
        frame_tick += 1;
        led_blink_tick += 1;
    }
}
