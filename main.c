#include <stdint.h>
#include "bsp.h"     // for the blink functions
#include "TM4C123GH6PM.h" // map of named hardware addresses
#include "unicorn.h"
#include <intrinsics.h>
#include "usertasks.h" //for userTaskLoad()


int main()
{ 
  __asm("CPSID I"); // disable interrupts

  // un-gateclock GPIOF AHB, set digital/direction , set Systick, set SysTck/PendSV priorities
  BSP_init();
  
  // perform all scheduler startup, then load/start userTaskLoad (will enable interrupts and call sched()
  initializeScheduler(userTaskLoad, 1U);
  
  // we should probably never get to this point
  while(1); 
    
  //return 0;
}