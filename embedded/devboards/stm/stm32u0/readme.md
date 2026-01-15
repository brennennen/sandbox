# STM32U0
Devboard: NUCLEO-U083RC
Chip: STM32U083RC (T6)
Vendor Firmeware Package: https://github.com/STMicroelectronics/STM32CubeU0
Specification:
Datasheet: https://www.st.com/resource/en/datasheet/stm32u083rc.pdf
Reference Manual: https://www.st.com/en/microcontrollers-microprocessors/stm32u0-series/documentation.html
CMSIS files: https://github.com/STMicroelectronics/STM32Cube_MCU_Overall_Offer

## NUCLEO-U083RC
Reference Manual: https://www.st.com/resource/en/reference_manual/rm0503-stm32u0-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf



## Lectures
* Digikey getting started: https://www.youtube.com/watch?v=hyZS2p1tW-g

## Specs
* Arm Cortex M0
* 56 MHz
* 256 Kbyte flash
* 40 Kbyte SRAM
* APB and AHB buses
* 32 bit multi-AHB bus matrix

## Peripherals
* 12 bit ADC
* 12 bit DAC
* Rail to rail analog comparators
* One operational amplifier
* low power RTC
* general purpose 32-bit timer
* 16-bit PWM timer for motor control
* 3 general purpose 16-bit timers
* 4 16-bit low power timers
* 21 capacitive sensing channels
* LCD controller
* 4 I2C interfaces
* 3 SPI interfaces
* 4 USART interfaces
* 3 low-poer uart interfaces
* crystal-less usb full speed device
* aes hardware accelerator

------------------------------------------------

## STM32U0 Option Bytes
STM32 "Option Bytes" are flats/data controlling core functionality of the chip that is processed before startup/reset handler code. See RM0503 Section 3.4 "FLASH option bytes". STM32U0 has 48 option bytes.

"Option Bytes" are write-protected on reset and special sequence is required to unlock them.
* 1. Unlock FLASH_CR
  * Write `KEY1 = 0x4567 01234` into `FLASH_KEYR`
  * Write `KEY2 = 0xCDEF89AB` into `FLASH_KEYR`
* 2. Write `OPTKEY1 = 0x0819 2A3B`
* 3. Write `OPTKEY2 = 0x4C5D 6E7F`

### FLASH_OPTR (bytes 0 - 3)
* Readout Protection (RDP): (bits 0 - 7)
  * Level 0 (0xAA): Unlocked. Flash and debug freely (default).
  * Level 1 (0xCC): Password read protected. Can only connect a debugger if you have a password. Can "regress" back to Level 0 if you both set a password previously and have that password (mass erasure will still occur) (128-bit PSWD).
  * Level 2 (Others): Can never connect a debugger again. This is not reversible.
* Brown-out Reset (BOR_EN): (bit 8)
  * 0: Brown-out reset disabled
  * 1: Brown-out reset enabled (if VDD below BOR_LEV, hold cpu in reset) (default)
* Brown-out Reset Level (BOR_LEV): (bits 9 - 10)
  * 0b00: level 0, 2.0 V
  * 0b01: level 1, 2.2 V
  * 0b10: level 2, 2.5 V
  * 0b11: level 3, 2.8 V (default)
* Reserved (bits 11 - 12)
* Negative reset on sleep mode. Catches calls to sleep and resets instead if set low.
  * Negative Reset on Stop sleep mode (NRST_STOP): (bit 13)
    * 0: Reset when entering "Stop" sleep mode.
    * 1: No reset (default)
  * Negative reset on Standby sleep mode (NRST_STDBY): (bit 14)
    * 0: Reset when entering "Standby" sleep mode.
    * 1: No reset (default)
  * Negative reset on Shutdown sleep mode (NRST_SHDW): (bit 15)
    * 0: Reset when entering "Shutdown" sleep mode.
    * 1: No reset (default)
* Independent watchdog (checks if too slow, ex: kick every 1 second or reset):
  * Independent watchdog selection (IWDG_SW): (bit 16)
    * 0: Hardware independent watchdog (on immediately after reset)
    * 1: Software independent watchdog (manually turned on in app software) (default)
  * Independent watchdog counter freeze on "Stop" sleep mode (IWDG_STOP): (bit 17)
    * 0: Watchdog freezes in "Stop" sleep mode.
    * 1: Stays running (default)
  * Independent watchdog counter freeze on "Standby" sleep mode (IWDG_STBY): (bit 18)
    * 0: Watchdog freezes in "Standby" sleep mode.
    * 1: Stays running (default)
* Window watchdog (checks if too slow OR too fast, ex: kick exactly every 40ms to 60ms or reset):
  * Window watchdog selection (WWDG_SW): (bit 19)
    * 0: Hardware window watchdog (on immediately after reset)
    * 1: Software window watchdog (manually turned on in app software) (default)
* Reserved (20)
* Backup domain reset (BDRST): (bit 21)
  * 0: Reset of backup domain (RTC registers and backup registers) forced on shutdown exit
  * 1: Backup domain not reset on shutdown sleep mode exit (default)
* RAM Error Checking (RAM_PARITY_CHECK): (bit 22)
  * 0: Enable RAM parity bit checking every read operation. On check fail, raise NMI and HardFault.
  * 1: Disable (default)
* Backup SRAM2 erase prevention (BKPSRAM_HW_ERASE_DISABLE): (bit 23)
  * 0: Erase backup SRAM2 on system restart.
  * 1: Keep backup SRAM2 on system restart (requires coin cell battery). (default)
* Boot Selector (NBOOT_SEL): (bit 24)
  * 0: Use BOOT0 pin (legacy mode)
  * 1: Use NBOOT0 option bit (default)
* Boot Configuration (NBOOT1): (bit 25)
  * 0: USB or UART special boot
  * 1: Boot to built-in ST factory code (default)
* Normal Boot 0 (NBOOT0): (bit 26)
  * 0: Special boot, check NBOOT1.
  * 1: Normal boot (default)
* Negative reset mode? (NRST_MODE): (bits 27 - 28)
  * 0b00: Reserved
  * 0b01: Reset input only (no output signaling when an internal reset occurs)
  * 0b10: Standard GPIO (hijack the reset pin for an extra gpio pin)
  * 0b11: Bidirectional reset: the PF2-NRST pin is configured in reset input/output (legacy) mode (default)
    * Input: setting the pin low resets the mcu (ex: a push button).
    * Output: when the board internally resets (ex: brown-out threshold met), the mcu pulls the pin low, to potentially signal to other devices to also reset.
* Internal reset holder enable (IRHEN): (bit 29)
  * 0: simple pulse reset pin low (might not be detected because of capacitance)
  * 1: pull reset pin low and monitor voltage, only raise after pulling low is detected. (default)
* Reserved (bits 30 - 31)

### FLASH_WRP1AR (bytes 4 - 7)

### FLASH_WRP1BR (bytes 8 - 11)

### FLASH_SECR (bytes 12 - 15)


### FLASH_OEM1KEYR# (bytes 16 - 31)
16 byte (128 bit) password

### FLASH_OEM2KEYR# (bytes 32 - 47)
16 byte (128 bit) password

------------------------------------------------

## FLASH registers
(useful references: 3.7 FLASH registers and 3.7.22 FLASH register map)
* FLASH_ACR
* FLASH_KEYR
* FLASH_OPTKEYR
* FLASH_SR
* FLASH_CR
* FLASH_ECCR
* FLASH_OPTR
* FLASH_WRP1AR
* FLASH_WRP1BR
* FLASH_SECR
* FLASH_OEM1KEYR1
* FLASH_OEM1KEYR2
* FLASH_OEM1KEYR3
* FLASH_OEM1KEYR4
* FLASH_OEM2KEYR1
* FLASH_OEM2KEYR2
* FLASH_OEM2KEYR3
* FLASH_OEM2KEYR4
* FLASH_OEMKEYSR
* FLASH_HDPCR
* FLASH_HDPEXTR

### FLASH_ACR
Address: 0x000
Reset value: 0x0004 0600
* Latency (LATENCY): (bits 0 - 2)
* Reserved (bits 3 - 7)
* CPU Prefetch enable (PRFTEN): (bit 8)
  * 1: enabled
* CPU Instruction cache enable (ICEN): (bit 9)
  * 1: enabled
* Reserved (bit 10)
* CPU Instruction cache reset (ICRST): (bit 11)
* Reserved (bits 12 - 15)
* Main flash memory area empty (EMPTY): (bit 16)
* Reserved (bit 17)
* Debug access software enable (DBG_SWEN): (bit 18)
  * 1: enabled
* Reserved (bits 19 - 31)

## Memory Map
* STM32U0 Programming Reference Manual (RM0503):
  * Section 2.2 Memory Map
```
Code:               0x00000000 - 0x1FFFFFFF
SRAM:               0x20000000 - 0x3FFFFFFF
Peripheral:         0x40000000 - 0x5FFFFFFF
External RAM:       0x60000000 - 0x9FFFFFFF
External device:    0xA0000000 - 0xDFFFFFFF
Private peripheral bus: 0xE0000000 - 0xE00FFFFF
Device:             0xE0100000 - 0xFFFFFFFF
```


