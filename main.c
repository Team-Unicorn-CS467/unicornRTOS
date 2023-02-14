#include <stdint.h>
#include "board_io.h"     // for the blink functions
//#include "ticks.h"        // for resetTicks()
#include "TM4C123GH6PM.h" // map of named hardware addresses
#include "unicorn.h"
#include <intrinsics.h>
#include "usertasks.h" //for userTaskLoad()


void blinky1() 
{
    while (1) 
    {
        ledBlueOn();
        sleep(1000);
        ledBlueOff();
        sleep(2000);;
        //ledBlueOff();
        //sleep(1);
    }
}



void blinky2() 
{
    while (1) 
    {
        ledRedOn();
        sleep(1000);
        ledRedOff();
        sleep(1000);;
        //ledRedOff();
        //sleep(1);
    }
}
int main()
{ 
  // un-gateclock GPIOF AHB, set digital/direction , set Systick, set SysTck/PendSV priorities
  boardStartup();
  
  //OS stuff
  initializeScheduler();
  
  //readyNewTask(&userTaskLoad);
  readyNewTask(&blinky1);
  readyNewTask(&blinky2);
  
  
  //__asm("CPSID I");
  __asm("CPSID I");
  sched();
  __asm("CPSIE I");
  
  //***from this point, the systick interrupt handler will begin executing and will cause scheduling of processes**
 
  while(1);
  
  //return 0;
}