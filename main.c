#include <stdint.h>
#include "bsp.h"     // for the blink functions
#include "TM4C123GH6PM.h" // map of named hardware addresses
#include "unicorn.h"
#include <intrinsics.h>
#include "usertasks.h" //for userTaskLoad()


int main()
{ 
  __asm("CPSID I");

  // un-gateclock GPIOF AHB, set digital/direction , set Systick, set SysTck/PendSV priorities
  //boardStartup();
  // using Kevin's SUPER bsp
  BSP_init();

  //testing
  UART5_SendString("this is a test");
  
  //OS stuff
  initializeScheduler();
  
  readyNewTask(userTaskLoad, MAX_TASKS); // MAX_TASKS is max priority
  
  sched(); //schedule first task and set PendSV to trigger (as soon as interrupts are enabled)
  __asm("CPSIE I");
  
  //after the initial PendSV trigger following sched(),
  //we will never return here
  while(1); 
    
  //return 0;
}