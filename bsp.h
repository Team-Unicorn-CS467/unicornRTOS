#ifndef __BSP_H__
#define __BSP_H__

/* Includes */
#include <stdint.h>  /* Standard integers. WG14/N843 C99 Standard */
#include "TM4C123GH6PM.h" /* the TM4C MCU Peripheral Access Layer (TI) */

/* uncomment the RTOS implementation that the file is being used in */
//#define freeRTOS
#define unicornRTOS

/* system clock tick [Hz] */
#define BSP_TICKS_PER_SEC 1000U

/* LED States */
typedef enum
{
  OFF = 0U,
  ON
}GPIO_LEDState;

/* GPIO Pin States */
typedef enum
{
  LOW = 0U,
  HIGH
}GPIO_PinState;


/* GPIO Rail Bit Fields */
#define GPIOF_BF  (1U << 5) /* GPIOF */
#define GPIOE_BF  (1u << 4) /* GPIOE */
#define GPIOD_BF  (1U << 3) /* GPIOD */
#define GPIOC_BF  (1U << 2) /* GPIOC */
#define GPIOB_BF  (1U << 1) /* GPIOB */
#define GPIOA_BF  (1U << 0) /* GPIOA */


/* GPIOs used in project */
#define GPIO_PF1  (1U << 1)
#define GPIO_PF2  (1U << 2)
#define GPIO_PF3  (1U << 3)

#define GPIO_PB3  (1u << 3)

#define GPIO_PC4  (1u << 4)
#define GPIO_PC5  (1u << 5)
#define GPIO_PC6  (1u << 6)
#define GPIO_PC7  (1u << 7)

#define GPIO_PD6  (1u << 6)
#define GPIO_PD7  (1u << 7)


/* on-board LEDs */
#define LED_RED   GPIO_PF1
#define LED_BLUE  GPIO_PF2
#define LED_GREEN GPIO_PF3

/* prototypes */
void BSP_init(void);
void BSP_setGPIO(GPIOA_Type* GPIOx, uint8_t Pin, GPIO_PinState PinState);
void BSP_setLED(uint8_t led, GPIO_LEDState state);

void UART5_SendString(char *str);
void UART5_SendByte(char data);
void Delay(unsigned long counter);
void init_UART(void);

#endif // __BSP_H__
