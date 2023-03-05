/* Board Support Package (BSP) for the EK-TM4C123GXL board */

/* This file uses a custom "TM4C123GH6PM.h" file that should be included in the project. */
#include "bsp.h"


void BSP_init(void)
{
  /* Goal is to have available, extra GPIO to be used for logic analyzer testing.
  The GPIOs located on J4 Connector of TivaC are marked as digital GPIO and  all
  having interrupt capability. The selection was also used, as these pins do
  not share any common functionality such as serial communication or Analog ability
  This selection allows for the most flexibility in terms of future developement */

  /* Enable Run mode for the following GPIO Rails */
  /* TM4C123GXL_datasheet - pg. 340 - Register 60 - General-Purpose Input/Output Run Mode Clock Gating Control (RCGCGPIO) */
  SYSCTL->RCGCGPIO  |= (GPIOF_BF | GPIOB_BF | GPIOC_BF | GPIOD_BF);

  /* Connect the GPIO rails to the high performance AHB bus */
  /* TM4C123GXL_datasheet - pg. 258 - Register 9 - GPIO High-Performance Bus Control (GPIOHBCTL) */
  SYSCTL->GPIOHBCTL |= (GPIOF_BF | GPIOB_BF | GPIOC_BF | GPIOD_BF);


  /* make sure the Run Mode and AHB-enable take effects
  *  before accessing the peripherals */
  __ISB(); /* Instruction Synchronization Barrier */
  __DSB(); /* Data Memory Barrier */

  GPIOF_AHB->DIR |= (GPIO_PF1 | GPIO_PF2 | GPIO_PF3);            /* Set GPIOs used in F rail as outputs */
  GPIOB_AHB->DIR |= (GPIO_PB3);                                  /* Set GPIOs used in B rail as outputs */
  GPIOC_AHB->DIR |= (GPIO_PC4 | GPIO_PC5 | GPIO_PC6 | GPIO_PC7); /* Set GPIOs used in C rail as outputs */
  GPIOD_AHB->DIR |= (GPIO_PD6 | GPIO_PD7);                       /* Set GPIOs used in D rail as outputs */


  GPIOF_AHB->DEN |= (GPIO_PF1 | GPIO_PF2 | GPIO_PF3);            /* Enable GPIOs used in F rail */
  GPIOB_AHB->DEN |= (GPIO_PB3);                                  /* Enable GPIOs used in B rail */
  GPIOC_AHB->DEN |= (GPIO_PC4 | GPIO_PC5 | GPIO_PC6 | GPIO_PC7); /* Enable GPIOs used in C rail */
  GPIOD_AHB->DEN |= (GPIO_PD6 | GPIO_PD7);                       /* Enable GPIOs used in D rail */

#ifdef unicornRTOS
  uint32_t clockFreq = 16000000U; // 16 mhz
  uint32_t systickLoadRegVal = (clockFreq / BSP_TICKS_PER_SEC) - 1U;

  //systick stuff
  // register abstractions in core_cm4h.h (CMSIS directory)
  SysTick->LOAD = systickLoadRegVal;          // sets SysTick interrupt frequency to systickLoadRegVal + 1 hz
  SysTick->VAL  = 0U;                         // clear on write (so clears the counter value)
  SysTick->CTRL = (uint32_t)0b00000111U;      // clock source, interrupt enable, counter enable in 'multi-shot' (repeating mode)
  
  // sets PendSV interrupt priority to less than Systick,
  // thus allowing Systic to exit and then enter PendSV
  NVIC_SetPriority(SysTick_IRQn,        0x00); // set Systick to highest priority
  NVIC_SetPriority(PendSV_IRQn,         0xFF); // set PendSV to the lowest priority
#endif

#ifdef freeRTOS

  //from keil file system_TM4C123.c
  //uint32_t SystemCoreClock = __CORE_CLK;  /*!< System Clock Frequency (Core Clock)*/
  // this seems to be setting the value of SystemCoreClock, used below, to the frequency of the system core clock
  SystemCoreClockUpdate();

  SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);
  __enable_irq();
#endif

  init_UART();
}

/* Function for sending entire strings */
void UART5_SendString(char *str)
{
  while(*str)
  {
    UART5_SendByte(*(str++));
  }
}

/* Function for sending one byte at a time */
void UART5_SendByte(char data)  
{
    while((UART5->FR & 0x20) != 0); 	 /* wait until Tx buffer not full */
    UART5->DR = data;                  /* before giving it another byte */
}


/*
TM4C123GH6PM has 7 total UART peripherals
Here we will use UART 2 connected to pins PE5(Tx) and PE7(Rx) 

The following UART configuration was prepared by following the tutorial listed below
https://microcontrollerslab.com/uart-communication-tm4c123-tiva-c-launchpad/
*/



/*
Baud Rate Setup - Baud rate is the rate at which the data will traverse the pin
- Both transmit and receive need to agree on the same baud

In the TM4C123GH6PM, there are two registers that dictate the baud rate:
        - UARTIBRD - Integer Baud Rate Divisor
        - UARTFBRD - Fractional Baud Rate Divisor

Steps to calculate baud rate register values
1. Obtain System clock speed
2. Use the following equation to calculate UARTIBRD integer value based on baud rate selection
     UART Baud Rate =  ( SystemCoreClock / 16 x UARTIBRD) 

     Example: for a baud rate of 921600 and SystemCoreClock of 16MHz

        921600 = (16MHz / 16 x UARTIBRD)
        or
        UARTIBRD = 1000000 / 921600
        then 
        UARTIBRD = 1.085069444444444

3. Take note of the integer portion and the decimal portion of the result
      integer portion - 1
      decimal portion - 0.085069444444444
 
4. The UARTIBRD should only be written with the integer portion
      Then: UARTIBRD = 1

5. The decimal portion should then be fed into the following calculation to determine the value for UARTFBRD
      Decimal Portion x 64 + 0.5 = UARTFBRD
      then
      0.085069444444444 x 64 + 0.5 = 5.944444444444444
      then the UARTFBRD will only use the integer portion of the result above. 
      UARTFBRD = 6
*/
void init_UART(void)
{
  SYSCTL->RCGCUART |= 0x20;  /* enable clock to UART5 */
  SYSCTL->RCGCGPIO |= 0x10;  /* enable clock to PORTE for PE4/Rx and RE5/Tx */
      
  /* UART5 initialization */
  UART5->CTL = 0;         /* UART5 module disbable */
  UART5->IBRD = 1;      /* for 921600 baud rate, integer = 1 */
  UART5->FBRD = 6;       /* for 921600 baud rate, fractional = 6*/
  UART5->CC = 0;          /* select system clock*/
  UART5->LCRH = 0x60;     /* data lenght 8-bit, not parity bit, no FIFO */
  UART5->CTL = 0x301;     /* Enable UART5 module, Rx and Tx */

  /* UART5 TX5 and RX5 use PE4 and PE5. Configure them digital and enable alternate function */
  GPIOE->DEN = 0x30;      /* set PE4 and PE5 as digital */
  GPIOE->AFSEL = 0x30;    /* Use PE4,PE5 alternate function */
  GPIOE->AMSEL = 0;    /* Turn off analg function*/
  GPIOE->PCTL = 0x00110000;     /* configure PE4 and PE5 for UART */
}

/* Abstraction for setting the state of a GPIO on the TivaC Launchpad Board */
/* param GPIOx, the GPIO typedef ex. GPIOF_AHB                              */
/* param Pin, the GPIO being set ex. GPIO_PF1                               */
/* param PinState, HIGH or LOW                                              */
void BSP_setGPIO(GPIOA_Type* GPIOx, uint8_t Pin, GPIO_PinState PinState)
{
  if(PinState == HIGH){ /* Set high */
    GPIOx->DATA_Bits[Pin] = Pin;	
  }
  else{ /*  all other conditions set low */
    GPIOx->DATA_Bits[Pin] = LOW;	
  }
}


/* Abstraction for setting the state of an LED on the TivaC Launchpad Board */
/* param led, the led being set (LED_x) */
/* param state, ON or Off  */
void BSP_setLED(uint8_t led, GPIO_LEDState state)
{
  BSP_setGPIO(GPIOF_AHB, led, (GPIO_PinState)(state));
}
