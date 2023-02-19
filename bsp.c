/* Board Support Package (BSP) for the EK-TM4C123GXL board */

/* This file uses a custom "TM4C123GH6PM.h" file that should be included in the project. */
#include "bsp.h"


void BSP_init(void) 
{
	
	
#ifdef unicornRTOS
    __asm("CPSID I"); //disable interrupts
#endif	
	
    /* 
     Goal is to have available, extra GPIO to be used for logic analyzer testing.
     The GPIOs located on J4 Connector of TivaC are marked as digital GPIO and  all
     having interrupt capability. The selection was also used, as these pins do
     not share any common functionality such as serial communication or Analog ability
     This selection allows for the most flexibility in terms of future developement 
    */
	
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
		//systick stuff
    // register abstractions in core_cm4h.h (CMSIS directory)
    SysTick->LOAD = (uint32_t)1000000U;
    SysTick->VAL  = 0U;                          //clear on write (so clears the counter value)
    SysTick->CTRL = (uint32_t)0b00000111U;    //clock source, interrupt enable, counter enable in 'multi-shot' (repeating mode)
		
    //exception handler preemption priorty stuff
    NVIC_SetPriority(SysTick_IRQn, 0U); // set Systick to higherst priority
  
  
    //has the effect of setting pendsv to lowest priorty, 
    // RESERVED, and monitor interrupts to highest priority
    // *** couldn't find the TM4c123GH6PM.h equivalent of this lm4f120h5qr.h register abstraction, so I hardcoded it
      // NVIC_SYS_PRI3_R = 0x00FF0000U; 
    // sets PendSV interrupt priority to less than Systick, thus allowing Systic to exit and then enter PendSV
    (*((volatile unsigned long *)0xE000ED20)) = 0x00FF0000U;
#endif

#ifdef freeRTOS
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    __enable_irq();
#endif
}


/* Abstraction for setting the state of a GPIO on the TivaC Launchpad Board */
/* param GPIOx, the GPIO typedef ex. GPIOF_AHB                              */
/* param Pin, the GPIO being set ex. GPIO_PF1                               */
/* param PinState, HIGH or LOW                                              */
void BSP_setGPIO(GPIOA_Type* GPIOx, uint8_t Pin, GPIO_PinState PinState)
{
    if(PinState == HIGH)
    {  
      GPIOx->DATA_Bits[Pin] = Pin; /* Set high */
    }
    else
    { 
      GPIOx->DATA_Bits[Pin] = LOW; /*  all other conditions set low */
    }
}


/* Abstraction for setting the state of an LED on the TivaC Launchpad Board */
/* param led, the led being set (LED_x) */
/* param state, ON or Off  */
void BSP_setLED(uint8_t led, GPIO_LEDState state)
{
    /* casted state for readability */
    BSP_setGPIO(GPIOF_AHB, led, (GPIO_PinState)state);
}

