#include "board_io.h"
//#include "lm4f120h5qr.h" //map of named hardware addresses
#include "TM4C123GH6PM.h"
//#include "core_cm4.h"

const int BLINKY_MAX = 2000;


void boardStartup(void)
{ 

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
  SysTick->LOAD = (uint32_t)1000000U;
  SysTick->VAL  = 0U;                          //clear on write (so clears the counter value)
  SysTick->CTRL = (uint32_t)0b00000111U;    //clock source, interrupt enable, counter enable in 'multi-shot' (repeating mode)
  
  //exception handler preemption priorty stuff
  //has the effect of setting pendsv to lowest priorty, 
  // RESERVED, and monitor interrupts to highest priority
  // *** couldn't find the TM4c123GH6PM.h equivalent of this lm4f120h5qr.h register abstraction, so I hardcoded it
    // NVIC_SYS_PRI3_R = 0x00FF0000U;    
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
    

void blinkRed()
{
    int direction = -1;
    int offset = BLINKY_MAX;
    while(1)
    {
      GPIOF_AHB->DATA_Bits[RED] = RED; //turn color on
      int i = -BLINKY_MAX;
      while(i < offset)
        ++i;        

      GPIOF_AHB->DATA_Bits[RED] = OFF; //turn color off
      i = offset;
      while(i < BLINKY_MAX)
        ++i;        

      offset += direction;
      if(offset == BLINKY_MAX || offset == -BLINKY_MAX)
        direction *= -1;
    }
}

//assumes necessary initial setup completed to support blinking
void blinkBlue()
{
    int direction = 1;
    int offset = 0;
    while(1)
    {
      GPIOF_AHB->DATA_Bits[BLUE] = BLUE; //turn color on
      int i = -BLINKY_MAX;
      while(i < offset)
        ++i;        

      GPIOF_AHB->DATA_Bits[BLUE] = OFF; //turn color off
      i = offset;
      while(i < BLINKY_MAX)
        ++i;  
      
      offset += direction;
      if (offset == BLINKY_MAX || offset == -BLINKY_MAX)
        direction *= -1;
    }
}

//assumes necessary initial setup completed to support blinking
void blinkGreen()
{
    int direction = 1;
    int offset = -BLINKY_MAX;
    while(1)
    {
      GPIOF_AHB->DATA_Bits[GREEN] = GREEN; //turn color on
      int i = -BLINKY_MAX;
      while(i < offset)
        ++i;        

      GPIOF_AHB->DATA_Bits[GREEN] = OFF; //turn color off
      i = offset;
      while(i < BLINKY_MAX)
        ++i;  
      
      offset += direction;
      if (offset == BLINKY_MAX || offset == -BLINKY_MAX)
        direction *= -1;
    }
}