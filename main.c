#include <stdint.h>
#include "board_io.h"     // for the blink functions
#include "ticks.h"        // for resetTicks()
#include "TM4C123GH6PM.h" // map of named hardware addresses
#include "unicorn.h"
#include <intrinsics.h>


int main()
{ 
  // un-gateclock GPIOF AHB, set digital/direction , set Systick, set SysTck/PendSV priorities
  boardStartup();
  
  //OS stuff
  initializeScheduler();
  
  //***start tasks here***
  /*
  readyNewTask(&ledRedOn);
  readyNewTask(&ledRedOff);
  
  readyNewTask(&ledBlueOn);
  readyNewTask(&ledBlueOff);
  
  readyNewTask(&ledGreenOn);
  readyNewTask(&ledGreenOff);
  */
  
  readyNewTask(&blinkRed);
  readyNewTask(&blinkBlue);
  readyNewTask(&blinkGreen);
  
  resetTicks(); //set starting number of ticks to 0

  __asm("CPSIE I"); //enable interrupts
  
  //***from this point, the systick interrupt handler will begin executing and will cause scheduling of processes**
  
  while(1);

  //return 0;
}