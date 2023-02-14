#include "board_io.h"
//#include "lm4f120h5qr.h" //map of named hardware addresses
#include "TM4C123GH6PM.h"
//#include "core_cm4.h"


void boardStartup(void)
{
   __asm("CPSID I"); //disable interrupts

  /////////////////////////////////////////////////////////////////
  // switched to using the AHB bus (faster)
  /////////////////////////////////////////////////////////////////
  SYSCTL->RCGCGPIO  |= (1U << 5); // enable Run mode for GPIOF */
  SYSCTL->GPIOHBCTL |= (1U << 5); // enable AHB for GPIOF */
  
  //make sure the Run Mode and AHB-enable take effects
  //before accessing the peripherals
  __ISB(); //Instruction Synchronization Barrier 
  __DSB(); //Data Memory Barrier 
  
  
  GPIOF_AHB->DIR |= (RED | BLUE | GREEN);   // enable GPIOF red, blue and green as inputs
  GPIOF_AHB->DEN |= (RED | BLUE | GREEN);   // enable GPIOF red, blue and green as digital
  
  //systick stuff
  // register abstractions in core_cm4h.h (CMSIS directory)
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // 16 MHz clock with STRELOAD set to 15,999 should result in 1ms ticks
  SysTick->LOAD = (uint32_t)15999U;  //1000000 // STRELOAD datasheet p140
  SysTick->VAL  = 0U;                          //clear on write (so clears the counter value)
  SysTick->CTRL = (uint32_t)0b00000111U;    //clock source, interrupt enable, counter enable in 'multi-shot' (repeating mode)
  
  // just couldn't get this to work !
  //SystemCoreClockUpdate();
  //SysTick_Config(SystemCoreClock / SYSTICKS_PER_SEC);

  
  //exception handler preemption priorty stuff
  NVIC_SetPriority(SysTick_IRQn, 0U); // set Systick to higherst priority
  
  
  //has the effect of setting pendsv to lowest priorty, 
  // RESERVED, and monitor interrupts to highest priority
  // *** couldn't find the TM4c123GH6PM.h equivalent of this lm4f120h5qr.h register abstraction, so I hardcoded it
    // NVIC_SYS_PRI3_R = 0x00FF0000U; 
  // sets PendSV interrupt priority to less than Systick, thus allowing Systic to exit and then enter PendSV
  (*((volatile unsigned long *)0xE000ED20)) = 0x00FF0000U;
  
}
    
///////////////////////////////////////////////////////////////////
// Gotta get the Systick handler to manage the ticks on these
////////////////////////////////////////////////////////////////////
//assumes necessary initial setup completed to support blinking
void ledRedOn()    { GPIOF_AHB->DATA_Bits[RED] = RED; }
void ledRedOff()   { GPIOF_AHB->DATA_Bits[RED] = OFF; }

void ledGreenOn()  { GPIOF_AHB->DATA_Bits[GREEN] = GREEN; }
void ledGreenOff() { GPIOF_AHB->DATA_Bits[GREEN] = OFF; }
    
void ledBlueOn()   { GPIOF_AHB->DATA_Bits[BLUE] = BLUE; }
void ledBlueOff()  { GPIOF_AHB->DATA_Bits[BLUE] = OFF; }