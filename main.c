#include <stdint.h>

#include "board_io.h"     // for the blink functions
#include "ticks.h"        // for resetTicks()
#include "TM4C123GH6PM.h" // map of named hardware addresses
#include "locks.h"
#include "unicorn.h"

////////////////////////////////////////////////////////////

#include <intrinsics.h>

//***user adds tasks here***
void userTaskLoad()
{

  readyTaskStart(&blinkRed);
  readyTaskStart(&blinkBlue);
  readyTaskStart(&blinkGreen);
  
  /*
  readyTaskStart(&ledRedOn);
  readyTaskStart(&ledRedOff);
  
  readyTaskStart(&ledBlueOn);
  readyTaskStart(&ledBlueOff);
  
  readyTaskStart(&ledGreenOn);
  readyTaskStart(&ledGreenOff);
  */
  
  while(1); //LATER THIS WILL BE CHANGED TO A TASK EXIT CALL
  
}

int main()
{ 
  __asm("CPSID I"); //disable interrupts

  // un-gateclock GPIOF AHB, set digital/direction , set Systick, set SysTck/PendSV priorities
  boardStartup();
  
  //OS stuff
  initializeScheduler();

  //testing
  initSpinLock();
  acquireSpinLock();
  releaseSpinLock();
  //testing
  
  readyTaskStart(&userTaskLoad);
    
  resetTicks(); //set starting number of ticks to 0

  __asm("CPSIE I"); //enable interrupts
  
  //***from this point, the systick interrupt handler will begin executing and will cause scheduling of processes**
  
  while(1);

  //return 0;
}