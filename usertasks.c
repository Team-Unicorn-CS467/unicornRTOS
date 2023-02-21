#include "usertasks.h"


static int taskTimer = 1000000;

//***user tasks to be loaded at startup***
void userTaskLoad()
{

  ///////////////////////////////////////////////////////////////////
  // ADD STARTUP TASKS BELOW THIS LINE
  ///////////////////////////////////////////////////////////////////

/*  
  
  // basic test w/ task exit
  int i;
  for(i = 0; i < 3; ++i)
  {
    int j;
    startNewTask(blinkRed, 1);
    for (j = 0; j < taskTimer / 3; ++j);
    
    startNewTask(blinkBlue, 1);
    for (j = 0; j < taskTimer / 3 ; ++j);
    
    startNewTask(blinkGreen, 1);
    for (j = 0; j < taskTimer; ++j);
  }
  
*/
  
  // timeoutSleep test
  startNewTask(blinky1, 2);
  startNewTask(blinky2, 3);
  
  exitTask();
  
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
  ledRedOff();
  ledBlueOff();
  ledGreenOff();
  
  ledRedOn();
  
  for (int i = 0; i < taskTimer; ++i);
  
  ledRedOff();
  
  exitTask();
}

void blinkBlue()
{
  ledRedOff();
  ledBlueOff();
  ledGreenOff();
  
  ledBlueOn();
  
  for (int i = 0; i < taskTimer; ++i);
  
  ledBlueOff();
  
  exitTask();
}

void blinkGreen()
{
  ledRedOff();
  ledBlueOff();
  ledGreenOff();
  
  ledGreenOn();
  
  for (int i = 0; i < taskTimer; ++i);
  
  ledGreenOff();
  
  exitTask();
}

void blinky1() 
{
    while (1) 
    {
        ledBlueOn();
        timeoutSleep(500);
        ledBlueOff();
        timeoutSleep(500);
    }
}

void blinky2() 
{
    while (1) 
    {
        ledRedOn();
        timeoutSleep(350);
        ledRedOff();
        timeoutSleep(350);
    }
}

void anti_blinky() 
{
  ledRedOff();
  ledBlueOff();
  exitTask();
}