/*
 * ST provided ARM CMSIS file for the STM32U0 (apache license)
 * I've built this by copy pasting just the bits needed for a single
 * test from the ~11k loc file in an effort to understand the CMSIS
 * effort.
 */
#include <stdint.h>

#define     __O     volatile             /*!< Defines 'write only' permissions */
#define     __IO    volatile             /*!< Defines 'read / write' permissions */


// Interrupt Vector Table
typedef enum
{
/******  Cortex-M Processor Exceptions Numbers *****************************************************************/
  Reset_IRQn                  = -15,    /*!< -15 Reset Vector, invoked on Power up and warm reset              */
  NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                                          */
  HardFault_IRQn              = -13,    /*!< 3 Cortex-M Hard Fault Interrupt                                   */
  SVCall_IRQn                 = -5,     /*!< 11 Cortex-M SV Call Interrupt                                     */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M Pend SV Interrupt                                     */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M System Tick Interrupt                                 */
/******  STM32 specific Interrupt Numbers **********************************************************************/
  WWDG_IWDG_IRQn              = 0,      /*!< Window watchdog interrupt + Independent watchdog interrupt        */
  PVD_PVM_IRQn                = 1,      /*!< PVD through EXTI Line detection Interrupt(EXTI lines 16/19/20/21) */
  RTC_TAMP_IRQn               = 2,      /*!< RTC and TAMP interrupts (combined EXTI lines 20 & 21)             */
  FLASH_ECC_IRQn              = 3,      /*!< FLASH global Interrupt + FLASH ECC interrupt                      */
  RCC_CRS_IRQn                = 4,      /*!< RCC global Interrupt + CRS global interrupt                       */
  EXTI0_1_IRQn                = 5,      /*!< EXTI Line0 & Line1 Interrupt                                      */
  EXTI2_3_IRQn                = 6,      /*!< EXTI Line2 & Line3 Interrupt                                      */
  EXTI4_15_IRQn               = 7,      /*!< EXTI Line4 to Line15 Interrupt                                    */
  USB_DRD_FS_IRQn             = 8,      /*!< USB global interrupt (combined with EXTI 36)                      */
  DMA1_Channel1_IRQn          = 9,      /*!< DMA1 Channel 1 Interrupt                                          */
  DMA1_Channel2_3_IRQn        = 10,     /*!< DMA1 Channel 2 and Channel 3 Interrupts                           */
  DMA1_Ch4_7_DMA2_Ch1_5_DMAMUX_OVR_IRQn  = 11,     /*!< DMAMUX_OVR_IT + DMA1 channel 4 to 7 +  DMA2 channel 1 to 5        */
  ADC_COMP1_2_IRQn            = 12,     /*!< ADC and COMP1/COMP2 interrupts (ADC combined with EXTI 17 & 18)   */
  TIM1_BRK_UP_TRG_COM_IRQn    = 13,     /*!< TIM1 break, update, trigger, commutation, error, direction change and index interrupts        */
  TIM1_CC_IRQn                = 14,     /*!< TIM1 Capture Compare interrupt                                    */
  TIM2_IRQn                   = 15,     /*!< TIM2 global Interrupt                                             */
  TIM3_IRQn                   = 16,     /*!< TIM3 global Interrupt                                             */
  TIM6_DAC_LPTIM1_IRQn        = 17,     /*!< TIM6 + LPTIM1 + DAC global interrupt (combined with EXTI 31)      */
  TIM7_LPTIM2_IRQn            = 18,     /*!< TIM7 + LPTIM2 global interrupt (combined with EXTI 32)            */
  TIM15_LPTIM3_IRQn           = 19,     /*!< TIM15 + LPTIM3 global interrupt (combined with EXTI 33)           */
  TIM16_IRQn                  = 20,     /*!< TIM16 global interrupt                                            */
  TSC_IRQn                    = 21,     /*!< TSC global interrupt                                              */
  LCD_IRQn                    = 22,     /*!< LCD global interrupt                                              */
  I2C1_IRQn                   = 23,     /*!< I2C1 global interrupt (combined with EXTI 23)                     */
  I2C2_3_4_IRQn               = 24,     /*!< I2C2 + I2C3 global interrupt (combined with EXTI 22) + I2C4 global interrupt        */
  SPI1_IRQn                   = 25,     /*!< SPI1/I2S1 global interrupt                                        */
  SPI2_3_IRQn                 = 26,     /*!< SPI2 and SPI3 global interrupt                                    */
  USART1_IRQn                 = 27,     /*!< USART1 global interrupt (combined with EXTI 25)                   */
  USART2_LPUART2_IRQn         = 28,     /*!< USART2 global interrupt (combined with EXTI 26) + LPUART2 global interrupt (combined with EXTI lines 35)      */
  USART3_LPUART1_IRQn         = 29,     /*!< USART3  (combined with EXTI 24) + LPUART1 global interrupt (combined with EXTI lines 28)                      */
  USART4_LPUART3_IRQn         = 30,     /*!< USART4  global interrupt (combined with EXTI 20) + LPUART3 (combined with EXTI lines 34)                      */
  RNG_CRYP_IRQn               = 31,     /*!< RNG + CRYPTO global interrupt                                     */
} IRQn_Type;

/*
 * MARK: Device Specific Peripheral Address Map
 */
#define PERIPH_BASE         (0x40000000UL)
#define APBPERIPH_BASE        PERIPH_BASE
#define AHBPERIPH_BASE      (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE            (AHBPERIPH_BASE + 0x1000UL)
#define RCC                 ((RCC_TypeDef *) RCC_BASE)

#define USART2_BASE           (APBPERIPH_BASE + 0x4400UL)
#define USART2              ((USART_TypeDef *) USART2_BASE)

/*
 * MARK: Reset and Clock Control
 */

// Bit definition for RCC_CR register
#define RCC_CR_HSION_Pos                     (8UL)
#define RCC_CR_HSION_Msk                     (0x1UL << RCC_CR_HSION_Pos)       /*!< 0x00000100 */
#define RCC_CR_HSION                         RCC_CR_HSION_Msk                  /*!< Internal High Speed oscillator (HSI16) clock enable */
#define RCC_CR_HSIRDY_Pos                    (10UL)
#define RCC_CR_HSIRDY_Msk                    (0x1UL << RCC_CR_HSIRDY_Pos)      /*!< 0x00000400 */
#define RCC_CR_HSIRDY                        RCC_CR_HSIRDY_Msk                 /*!< Internal High Speed oscillator (HSI16) clock ready flag */

// Bit definition for RCC_CFGR register
// SW configuration
#define RCC_CFGR_SW_Pos                (0UL)
#define RCC_CFGR_SW_Msk                (0x7UL << RCC_CFGR_SW_Pos)              /*!< 0x00000007 */
#define RCC_CFGR_SW                    RCC_CFGR_SW_Msk                         /*!< SW[2:0] bits (System clock Switch) */
#define RCC_CFGR_SW_0                  (0x1UL << RCC_CFGR_SW_Pos)              /*!< 0x00000001 */
#define RCC_CFGR_SW_1                  (0x2UL << RCC_CFGR_SW_Pos)              /*!< 0x00000002 */
#define RCC_CFGR_SW_2                  (0x4UL << RCC_CFGR_SW_Pos)              /*!< 0x00000004 */
// SWS configuration
#define RCC_CFGR_SWS_Pos               (3UL)
#define RCC_CFGR_SWS_Msk               (0x7UL << RCC_CFGR_SWS_Pos)             /*!< 0x00000038 */
#define RCC_CFGR_SWS                   RCC_CFGR_SWS_Msk                        /*!< SWS[2:0] bits (System Clock Switch Status) */
#define RCC_CFGR_SWS_0                 (0x1UL << RCC_CFGR_SWS_Pos)             /*!< 0x00000008 */
#define RCC_CFGR_SWS_1                 (0x2UL << RCC_CFGR_SWS_Pos)             /*!< 0x00000010 */
#define RCC_CFGR_SWS_2                 (0x4UL << RCC_CFGR_SWS_Pos)             /*!< 0x00000020 */


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


/*
 * MARK: GPIO
 */

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
#define GPIO_MODER_MODE2_Pos           (4UL)
#define GPIO_MODER_MODE2_Msk           (0x3UL << GPIO_MODER_MODE2_Pos)         /*!< 0x00000030 */
#define GPIO_MODER_MODE2               GPIO_MODER_MODE2_Msk
#define GPIO_MODER_MODE2_0             (0x1UL << GPIO_MODER_MODE2_Pos)          /*!< 0x00000010 */
#define GPIO_MODER_MODE2_1             (0x2UL << GPIO_MODER_MODE2_Pos)          /*!< 0x00000020 */

#define GPIO_MODER_MODE3_Pos           (6UL)
#define GPIO_MODER_MODE3_Msk           (0x3UL << GPIO_MODER_MODE3_Pos)         /*!< 0x000000C0 */
#define GPIO_MODER_MODE3               GPIO_MODER_MODE3_Msk
#define GPIO_MODER_MODE3_0             (0x1UL << GPIO_MODER_MODE3_Pos)          /*!< 0x00000040 */
#define GPIO_MODER_MODE3_1             (0x2UL << GPIO_MODER_MODE3_Pos)          /*!< 0x00000080 */

#define GPIO_MODER_MODE5_Pos        (10UL)
#define GPIO_MODER_MODE5_Msk        (0x3UL << GPIO_MODER_MODE5_Pos)         /*!< 0x00000C00 */
#define GPIO_MODER_MODE5            GPIO_MODER_MODE5_Msk
#define GPIO_MODER_MODE5_0          (0x1UL << GPIO_MODER_MODE5_Pos)          /*!< 0x00000400 */
#define GPIO_MODER_MODE5_1          (0x2UL << GPIO_MODER_MODE5_Pos)          /*!< 0x00000800 */

// Bits definition for GPIO_ODR register
#define GPIO_ODR_OD5_Pos            (5UL)
#define GPIO_ODR_OD5_Msk            (0x1UL << GPIO_ODR_OD5_Pos)             /*!< 0x00000020 */
#define GPIO_ODR_OD5                GPIO_ODR_OD5_Msk

// Bit definition for GPIO_AFRL register
#define GPIO_AFRL_AFSEL3_Pos           (12UL)
#define GPIO_AFRL_AFSEL3_Msk           (0xFUL << GPIO_AFRL_AFSEL3_Pos)         /*!< 0x0000F000 */
#define GPIO_AFRL_AFSEL3               GPIO_AFRL_AFSEL3_Msk
#define GPIO_AFRL_AFSEL3_0             (0x1UL << GPIO_AFRL_AFSEL3_Pos)          /*!< 0x00001000 */
#define GPIO_AFRL_AFSEL3_1             (0x2UL << GPIO_AFRL_AFSEL3_Pos)          /*!< 0x00002000 */
#define GPIO_AFRL_AFSEL3_2             (0x4UL << GPIO_AFRL_AFSEL3_Pos)          /*!< 0x00004000 */
#define GPIO_AFRL_AFSEL3_3             (0x8UL << GPIO_AFRL_AFSEL3_Pos)          /*!< 0x00008000 */

/*
 * MARK: FLASH
 */
// FLASH Registers
typedef struct
{
  __IO uint32_t ACR;           /*!< FLASH Access Control register,                     Address offset: 0x00 */
       uint32_t RESERVED1;     /*!< Reserved1,                                         Address offset: 0x04 */
  __IO uint32_t KEYR;          /*!< FLASH Key register,                                Address offset: 0x08 */
  __IO uint32_t OPTKEYR;       /*!< FLASH Option Key register,                         Address offset: 0x0C */
  __IO uint32_t SR;            /*!< FLASH Status register,                             Address offset: 0x10 */
  __IO uint32_t CR;            /*!< FLASH Control register,                            Address offset: 0x14 */
  __IO uint32_t ECCR;          /*!< FLASH ECC register,                                Address offset: 0x18 */
       uint32_t RESERVED2;     /*!< Reserved2,                                         Address offset: 0x1C */
  __IO uint32_t OPTR;          /*!< FLASH Option register,                             Address offset: 0x20 */
       uint32_t RESERVED3[2];  /*!< Reserved3,                                                 0x24 -- 0x28 */
  __IO uint32_t WRP1AR;        /*!< FLASH Bank WRP area A address register,            Address offset: 0x2C */
  __IO uint32_t WRP1BR;        /*!< FLASH Bank WRP area B address register,            Address offset: 0x30 */
       uint32_t RESERVED4[19]; /*!< Reserved4,                                                0x34 -- 0x7C */
  __IO uint32_t SECR;          /*!< FLASH Security option register,                    Address offset: 0x80 */
       uint32_t RESERVED5;     /*!< Reserved5,                                         Address offset: 0x84 */
  __IO uint32_t OEM1KEYW0R;    /*!< FLASH OEM1 key register 1,                         Address offset: 0x88 */
  __IO uint32_t OEM1KEYW1R;    /*!< FLASH OEM1 key register 2,                         Address offset: 0x8C */
  __IO uint32_t OEM1KEYW2R;    /*!< FLASH OEM1 key register 3,                         Address offset: 0x90 */
  __IO uint32_t OEM1KEYW3R;    /*!< FLASH OEM1 key register 4,                         Address offset: 0x94 */
  __IO uint32_t OEM2KEYW0R;    /*!< FLASH OEM2 key register 5,                         Address offset: 0x98 */
  __IO uint32_t OEM2KEYW1R;    /*!< FLASH OEM2 key register 6,                         Address offset: 0x9C */
  __IO uint32_t OEM2KEYW2R;    /*!< FLASH OEM2 key register 7,                         Address offset: 0xA0 */
  __IO uint32_t OEM2KEYW3R;    /*!< FLASH OEM2 key register 8,                         Address offset: 0xA4 */
  __IO uint32_t OEMKEYSR;      /*!< FLASH OEM key status register,                     Address offset: 0xA8 */
  __IO uint32_t HDPCR;         /*!< FLASH HDP control register,                        Address offset: 0xAC */
  __IO uint32_t HDPEXTR;       /*!< FLASH HDP extension register,                      Address offset: 0xB0 */
} FLASH_TypeDef;

#define FLASH_R_BASE          (AHBPERIPH_BASE + 0x2000UL)
#define FLASH               ((FLASH_TypeDef *) FLASH_R_BASE)

// Bits definition for FLASH_SR register
#define FLASH_SR_PROGERR_Pos                (3UL)
#define FLASH_SR_PROGERR_Msk                (0x1UL << FLASH_SR_PROGERR_Pos)         /*!< 0x00000008 */
#define FLASH_SR_PROGERR                    FLASH_SR_PROGERR_Msk
#define FLASH_SR_BSY1_Pos                   (16UL)
#define FLASH_SR_BSY1_Msk                   (0x1UL << FLASH_SR_BSY1_Pos)            /*!< 0x00010000 */
#define FLASH_SR_BSY1                       FLASH_SR_BSY1_Msk

// Bits definition for FLASH_CR register
#define FLASH_CR_PG_Pos                     (0UL)
#define FLASH_CR_PG_Msk                     (0x1UL << FLASH_CR_PG_Pos)              /*!< 0x00000001 */
#define FLASH_CR_PG                         FLASH_CR_PG_Msk
#define FLASH_CR_LOCK_Pos                   (31UL)
#define FLASH_CR_LOCK_Msk                   (0x1UL << FLASH_CR_LOCK_Pos)            /*!< 0x80000000 */
#define FLASH_CR_LOCK                       FLASH_CR_LOCK_Msk


/*
 * MARK: USART
 */

/**
  * @brief Universal Synchronous Asynchronous Receiver Transmitter
  */
typedef struct
{
  __IO uint32_t CR1;    /*!< USART Control register 1,                 Address offset: 0x00 */
  __IO uint32_t CR2;    /*!< USART Control register 2,                 Address offset: 0x04 */
  __IO uint32_t CR3;    /*!< USART Control register 3,                 Address offset: 0x08 */
  __IO uint32_t BRR;    /*!< USART Baud rate register,                 Address offset: 0x0C */
  __IO uint32_t GTPR;   /*!< USART Guard time and prescaler register,  Address offset: 0x10 */
  __IO uint32_t RTOR;   /*!< USART Receiver Time Out register,         Address offset: 0x14 */
  __IO uint32_t RQR;    /*!< USART Request register,                   Address offset: 0x18 */
  __IO uint32_t ISR;    /*!< USART Interrupt and status register,      Address offset: 0x1C */
  __IO uint32_t ICR;    /*!< USART Interrupt flag Clear register,      Address offset: 0x20 */
  __IO uint32_t RDR;    /*!< USART Receive Data register,              Address offset: 0x24 */
  __IO uint32_t TDR;    /*!< USART Transmit Data register,             Address offset: 0x28 */
  __IO uint32_t PRESC;  /*!< USART clock Prescaler register,           Address offset: 0x2C */
} USART_TypeDef;

// Bits to configure multi-feature pins to be uart
#define GPIO_AFRL_AFSEL2_Pos           (8UL)
#define GPIO_AFRL_AFSEL2_Msk           (0xFUL << GPIO_AFRL_AFSEL2_Pos)         /*!< 0x00000F00 */
#define GPIO_AFRL_AFSEL2               GPIO_AFRL_AFSEL2_Msk

#define RCC_APBENR1_USART2EN_Pos       (17UL)
#define RCC_APBENR1_USART2EN_Msk       (0x1UL << RCC_APBENR1_USART2EN_Pos)     /*!< 0x00020000 */
#define RCC_APBENR1_USART2EN           RCC_APBENR1_USART2EN_Msk

// USART Control Bits
#define USART_CR1_UE            (1UL << 0)  // USART Enable
#define USART_CR1_RE            (1UL << 2)  // Receiver Enable
#define USART_CR1_TE            (1UL << 3)  // Transmitter Enable

// USART Status Bits (ISR)
#define USART_ISR_RXNE_RXFNE    (1UL << 5)  // Read data register not empty
#define USART_ISR_TXE_TXFNF     (1UL << 7)  // Transmit data register empty
