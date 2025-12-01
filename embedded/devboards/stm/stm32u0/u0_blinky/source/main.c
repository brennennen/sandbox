/**
 * The structs/defines follow ARM CMSIS.
 */
#include <stdint.h>

#define     __O     volatile             /*!< Defines 'write only' permissions */
#define     __IO    volatile             /*!< Defines 'read / write' permissions */

/**
  * @brief Reset and Clock Control
  */
typedef struct
{
  __IO uint32_t CR;             /*!< RCC Clock Sources Control Register,                                     Address offset: 0x00 */
  __IO uint32_t ICSCR;          /*!< RCC Internal Clock Sources Calibration Register,                        Address offset: 0x04 */
  __IO uint32_t CFGR;           /*!< RCC Regulated Domain Clocks Configuration Register,                     Address offset: 0x08 */
  __IO uint32_t PLLCFGR;        /*!< RCC system PLL configuration register,                                  Address offset: 0x0C */
       uint32_t RESERVED0[2];   /*!< Reserved,                                                               Address offset: 0x10 -- 0x14 */
  __IO uint32_t CIER;           /*!< RCC Clock Interrupt Enable Register,                                    Address offset: 0x18 */
  __IO uint32_t CIFR;           /*!< RCC Clock Interrupt Flag Register,                                      Address offset: 0x1C */
  __IO uint32_t CICR;           /*!< RCC Clock Interrupt Clear Register,                                     Address offset: 0x20 */
       uint32_t RESERVED1;      /*!< Reserved,                                                               Address offset: 0x24 */
  __IO uint32_t AHBRSTR;        /*!< RCC AHB peripheral reset register,                                      Address offset: 0x28 */
  __IO uint32_t IOPRSTR;        /*!< RCC AHB2 peripheral reset register,                                     Address offset: 0x2C */
       uint32_t RESERVED2[2];   /*!< Reserved,                                                               Address offset: 0x30 -- 0x34 */
  __IO uint32_t APBRSTR1;       /*!< RCC APB1 peripheral reset register,                                     Address offset: 0x38 */
       uint32_t RESERVED3;      /*!< Reserved,                                                               Address offset: 0x3C */
  __IO uint32_t APBRSTR2;       /*!< RCC APB2 peripheral reset register,                                     Address offset: 0x40 */
       uint32_t RESERVED4;      /*!< Reserved,                                                               Address offset: 0x44 */
  __IO uint32_t AHBENR;         /*!< RCC AHB peripheral clocks enable register,                              Address offset: 0x48 */
  __IO uint32_t IOPENR;         /*!< RCC IO port enable register,                                            Address offset: 0x4C */
  __IO uint32_t DBGCFGR;        /*!< RCC DBGCFGR control register,                                           Address offset: 0x50 */
       uint32_t RESERVED5;      /*!< Reserved,                                                               Address offset: 0x54 */
  __IO uint32_t APBENR1;        /*!< RCC APB1 peripherals clock enable register,                             Address offset: 0x58 */
       uint32_t RESERVED6;      /*!< Reserved,                                                               Address offset: 0x5C */
  __IO uint32_t APBENR2;        /*!< RCC APB2 peripherals clock enable register,                             Address offset: 0x60 */
       uint32_t RESERVED7;      /*!< Reserved,                                                               Address offset: 0x64 */
  __IO uint32_t AHBSMENR;       /*!< RCC AHB peripheral clocks enable in sleep mode register,                Address offset: 0x68 */
  __IO uint32_t IOPSMENR;       /*!< RCC IO port peripheral clocks enable in sleep mode register,            Address offset: 0x6C */
       uint32_t RESERVED8[2];   /*!< Reserved,                                                               Address offset: 0x70 -- 0x74 */
  __IO uint32_t APBSMENR1;      /*!< RCC APB1 peripheral clocks enable in sleep mode register,               Address offset: 0x78 */
       uint32_t RESERVED9;      /*!< Reserved,                                                               Address offset: 0x7C */
  __IO uint32_t APBSMENR2;      /*!< RCC APB2 peripheral clocks enable in sleep mode register,               Address offset: 0x80 */
       uint32_t RESERVED10;     /*!< Reserved,                                                               Address offset: 0x84 */
  __IO uint32_t CCIPR;          /*!< RCC Peripherals Independent Clocks Configuration Register,              Address offset: 0x88 */
       uint32_t RESERVED11;     /*!< Reserved,                                                               Address offset: 0x8C */
  __IO uint32_t BDCR;           /*!< RCC backup domain control register,                                     Address offset: 0x90 */
  __IO uint32_t CSR;            /*!< RCC clock control & status register,                                    Address offset: 0x94 */
  __IO uint32_t CRRCR;          /*!< RCC clock recovery RC register,                                         Address offset: 0x98 */
} RCC_TypeDef;

#define PERIPH_BASE         (0x40000000UL)
#define AHBPERIPH_BASE      (PERIPH_BASE + 0x00020000UL)
#define RCC_BASE            (AHBPERIPH_BASE + 0x1000UL)
#define RCC                 ((RCC_TypeDef *) RCC_BASE)

/**
  * @brief General Purpose I/O
  */
typedef struct
{
  __IO uint32_t MODER;       /*!< GPIO port mode register,               Address offset: 0x00      */
  __IO uint32_t OTYPER;      /*!< GPIO port output type register,        Address offset: 0x04      */
  __IO uint32_t OSPEEDR;     /*!< GPIO port output speed register,       Address offset: 0x08      */
  __IO uint32_t PUPDR;       /*!< GPIO port pull-up/pull-down register,  Address offset: 0x0C      */
  __IO uint32_t IDR;         /*!< GPIO port input data register,         Address offset: 0x10      */
  __IO uint32_t ODR;         /*!< GPIO port output data register,        Address offset: 0x14      */
  __IO uint32_t BSRR;        /*!< GPIO port bit set/reset  register,     Address offset: 0x18      */
  __IO uint32_t LCKR;        /*!< GPIO port configuration lock register, Address offset: 0x1C      */
  __IO uint32_t AFR[2];      /*!< GPIO alternate function registers,     Address offset: 0x20-0x24 */
  __IO uint32_t BRR;         /*!< GPIO Bit Reset register,               Address offset: 0x28      */
} GPIO_TypeDef;

#define IOPORT_BASE         (0x50000000UL)              /*!< IOPORT base address */
#define GPIOA_BASE          (IOPORT_BASE + 0x0000UL)
#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)

// Bit definition for RCC_IOPENR register
#define RCC_IOPENR_GPIOAEN_Pos      (0UL)
#define RCC_IOPENR_GPIOAEN_Msk      (0x1UL << RCC_IOPENR_GPIOAEN_Pos)       /*!< 0x00000001 */
#define RCC_IOPENR_GPIOAEN          RCC_IOPENR_GPIOAEN_Msk

// Bits definition for GPIO_MODER register
#define GPIO_MODER_MODE5_Pos        (10UL)
#define GPIO_MODER_MODE5_Msk        (0x3UL << GPIO_MODER_MODE5_Pos)         /*!< 0x00000C00 */
#define GPIO_MODER_MODE5            GPIO_MODER_MODE5_Msk
#define GPIO_MODER_MODE5_0          (0x1UL << GPIO_MODER_MODE5_Pos)          /*!< 0x00000400 */
#define GPIO_MODER_MODE5_1          (0x2UL << GPIO_MODER_MODE5_Pos)          /*!< 0x00000800 */

// Bits definition for GPIO_ODR register
#define GPIO_ODR_OD5_Pos            (5UL)
#define GPIO_ODR_OD5_Msk            (0x1UL << GPIO_ODR_OD5_Pos)             /*!< 0x00000020 */
#define GPIO_ODR_OD5                GPIO_ODR_OD5_Msk

void delay(volatile uint32_t count) {
    while(count--) {
        __asm("nop");
    }
}

int main(void) {
    // Initialize the GPIO A controller
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

    // Initialize the LED (GPIO A - pin 5)
    GPIOA->MODER &= ~GPIO_MODER_MODE5;
    GPIOA->MODER |= GPIO_MODER_MODE5_0;

    // Turn on the LED
    GPIOA->ODR |= GPIO_ODR_OD5;

    // Toggle the LED every ~1(ish) seconds
    while (1)
    {
        GPIOA->ODR ^= GPIO_ODR_OD5;
        delay(500000);
    }
}
