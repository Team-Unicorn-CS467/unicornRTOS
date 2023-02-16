#include <stdint.h>
#include "board_io.h"     // for the blink functions
//#include "ticks.h"        // for resetTicks()
#include "TM4C123GH6PM.h" // map of named hardware addresses
#include "unicorn.h"
#include <intrinsics.h>
#include "usertasks.h" //for userTaskLoad()


void blinkyA() 
{
    while (1) 
    {
        for (int i=0; i<25; i++) 
        {
            ledBlueOn();
            for (int j=0; j<1000000; j++) {};
            //sleep(1); 
            ledBlueOff(); 
            for (int j=0; j<1000000; j++) {};
            //sleep(1); // blink blue in .2s intervals for 10 seconds
        }
    
        ledBlueOff();
        for (int j=0; j<2000000; j++) {};
        //sleep(1);;
        //ledBlueOff();
        //sleep(1);
    }
}

// blue    1 1 1 1 1 1 1 1 1 1 0 0
// red     0 0 1 1 1 1 0 0 0 0 0 0 
// blueoff 0 0 1 1 1 1 0 0 0 0 0 0      
void blinkyB() 
{
    while (1) 
    {
        sleep(2000);
        ledBlueOff();
        for (int i=0; i<20; i++) 
        {
            ledRedOn();
            sleep(100); 
            ledRedOff(); 
            sleep(100);
        } // blink red in .1s intervals for 4 seconds
        
        sleep(6000);
        
    }
}

int main()
{ 
  // un-gateclock GPIOF AHB, set digital/direction , set Systick, set SysTck/PendSV priorities
  boardStartup();
  
  //OS stuff
  initializeScheduler();
  
  
  readyNewTask(&blinkyA, 1);
  readyNewTask(&blinkyB, 3);
  
  //__asm("CPSID I");
  __asm("CPSID I");
  sched(); //schedule first task and set PendSV to trigger (as soon as interrupts are enabled)
  __asm("CPSIE I");
  
  //after the initial PendSV trigger following sched(),
  //we will never return here
  while(1); 
    
  //return 0;
}