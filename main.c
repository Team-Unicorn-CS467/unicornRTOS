#include <stdint.h>

#include "board_io.h"     // for the blink functions
#include "ticks.h"        // for resetTicks()
#include "TM4C123GH6PM.h" // map of named hardware addresses
#include "unicorn.h"
#include "locks.h" //for initSpinLock()
#include "usertasks.h" //for userTaskLoad

////////////////////////////////////////////////////////////

#include <intrinsics.h>


int main()
{ 
  __asm("CPSID I"); //disable interrupts

  // un-gateclock GPIOF AHB, set digital/direction , set Systick, set SysTck/PendSV priorities
  boardStartup();
  
  //OS stuff
  initializeScheduler();

  //initialize the new spin lock
  initSpinLock();
  
  readyTaskStart(&userTaskLoad);
    
  resetTicks(); //set starting number of ticks to 0

  __asm("CPSIE I"); //enable interrupts
  
  //***from this point, the systick interrupt handler will begin executing and will cause scheduling of processes**
  
  while(1);

  //return 0;
}