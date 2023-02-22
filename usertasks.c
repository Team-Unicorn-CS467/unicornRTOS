#include "usertasks.h"
#include "bsp.h"

//static int taskTimer = 50000;

//***user tasks to be loaded at startup***
void userTaskLoad()
{

  ///////////////////////////////////////////////////////////////////
  // ADD STARTUP TASKS BELOW THIS LINE
  ///////////////////////////////////////////////////////////////////

  while(1)
  {
    readyNewTask(blinkRed, 1);
    sleep(500);
    readyNewTask(blinkBlue, 2);
    sleep(500);
    readyNewTask(blinkGreen, 3);
    
    sleep(4000);
  }
  
  //exitTask();
  
  ///////////////////////////////////////////////////////////////////
  // NO CHANGES BELOW THIS LINE IN THIS FUNCTION
  ///////////////////////////////////////////////////////////////////
  
}

///////////////////////////////////////////////////////////////////
// ADD ANY USER TASK DEFINITIONS HERE
///////////////////////////////////////////////////////////////////

//each blink function below assumes necessary initial
//setup completed to support blinking

void blinkRed()
{
  BSP_setLED(LED_RED, OFF);
  BSP_setLED(LED_BLUE, OFF);
  BSP_setLED(LED_GREEN, OFF);
  
  BSP_setLED(LED_RED, ON);
  sleep(3000);
  BSP_setLED(LED_RED, OFF);
  
  exitTask();
}

void blinkBlue()
{
  BSP_setLED(LED_RED, OFF);
  BSP_setLED(LED_BLUE, OFF);
  BSP_setLED(LED_GREEN, OFF);
  
  BSP_setLED(LED_BLUE, ON);
  sleep(2000);
  BSP_setLED(LED_BLUE, OFF);
  
  exitTask();
}

void blinkGreen()
{
  BSP_setLED(LED_RED, OFF);
  BSP_setLED(LED_BLUE, OFF);
  BSP_setLED(LED_GREEN, OFF);
  
  BSP_setLED(LED_GREEN, ON);
  sleep(1000);
  BSP_setLED(LED_GREEN, OFF);
  
  exitTask();
}

void blinky1() 
{
    while (1) 
    {
        BSP_setLED(LED_BLUE, ON);
        sleep(500);
        BSP_setLED(LED_BLUE, OFF);
        sleep(250);;
        //BSP_setLED(LED_BLUE, OFF);
        //sleep(1);
    }
}

void blinky2() 
{
    while (1) 
    {
        BSP_setLED(LED_RED, ON);
        sleep(300);
        BSP_setLED(LED_RED, OFF);
        sleep(300);;
        //BSP_setLED(LED_RED, OFF);
        //sleep(1);
    }
}

void anti_blinky() 
{
  BSP_setLED(LED_RED, OFF);
  BSP_setLED(LED_BLUE, OFF);
  exitTask();
}