#include <stdint.h>
#include "board_io.h"     // for the blink functions
//#include "ticks.h"        // for resetTicks()
#include "TM4C123GH6PM.h" // map of named hardware addresses
#include "unicorn.h"
#include <intrinsics.h>
#include "usertasks.h" //for userTaskLoad()


int main()
{ 
  // un-gateclock GPIOF AHB, set digital/direction , set Systick, set SysTck/PendSV priorities
  boardStartup();
  
  //OS stuff
  initializeScheduler();
  
  readyNewTask(userTaskLoad, 8U); //max priority
  
  //__asm("CPSID I");
  __asm("CPSID I");
  sched(); //schedule first task and set PendSV to trigger (as soon as interrupts are enabled)
  __asm("CPSIE I");
  
  //after the initial PendSV trigger following sched(),
  //we will never return here
  while(1); 
    
  //return 0;
}