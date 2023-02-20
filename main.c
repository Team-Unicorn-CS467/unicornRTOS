#include <stdint.h>
#include "board_io.h"     // for the blink functions
//#include "ticks.h"        // for resetTicks()
#include "TM4C123GH6PM.h" // map of named hardware addresses
#include "unicorn.h"
#include <intrinsics.h>
#include "usertasks.h" //for userTaskLoad()


int main()
{ 
  __asm("CPSID I"); //disable interrupts

  // un-gateclock GPIOF AHB, set digital/direction , set Systick, set SysTck/PendSV priorities
  boardStartup();
  
  //OS stuff
  initializeScheduler();

  readyNewTask(userTaskLoad, 1U); //ready the task to run without scheduling
  
  __asm("CPSIE I"); //enable interrupts)
  
  //after the initial systick sched() call, we will never return here
  while(1); 
    
  //return 0;
}