#include "usertasks.h"


///////////////////////////////////////////////////////////////////
// ADD ANY USER TASK DEFINITIONS HERE
///////////////////////////////////////////////////////////////////


//each blink function below assumes necessary initial
//setup completed to support blinking

void singleBlinkRedExit()
{
  ledRedOff();
  ledBlueOff();
  ledGreenOff();
  
  ledRedOn();
  for (int i = 0; i < 400000; ++i);
  ledRedOff();
  exitTask();
}

void singleBlinkBlueExit()
{
  ledRedOff();
  ledBlueOff();
  ledGreenOff();
  
  ledBlueOn();
  for (int i = 0; i < 400000; ++i);
  ledBlueOff();
  exitTask();
}

void singleBlinkGreenExit()
{
  ledRedOff();
  ledBlueOff();
  ledGreenOff();
  
  ledGreenOn();
  for (int i = 0; i < 400000; ++i);
  ledGreenOff();
  exitTask();
}

void blinkRedFastExit() 
{
    for(int i = 0; i < 20; ++i) 
    {
      ledRedOff();
      ledBlueOff();
      ledGreenOff();
    
      ledRedOn();
      timeoutSleep(400);
      ledRedOff();
      timeoutSleep(400);
    }
    exitTask();
}

void blinkBlueMedExit() 
{
    for(int i = 0; i < 15; ++i)
    {
      ledRedOff();
      ledBlueOff();
      ledGreenOff();
    
      ledBlueOn();
      timeoutSleep(600);
      ledBlueOff();
      timeoutSleep(600);
    }
    exitTask();
}

void blinkGreenSlowExit() 
{
    for(int i = 0; i < 10; ++i)
    {
      ledRedOff();
      ledBlueOff();
      ledGreenOff();
    
      ledGreenOn();
      timeoutSleep(900);
      ledGreenOff();
      timeoutSleep(900);
    }
    exitTask();
}

void longPulseRedSemaphore() 
{
    while(1)
    {
      aquireUnicornSemaphore(0);
      
      ledRedOff();
      ledBlueOff();
      ledGreenOff();
    
      ledRedOn();
      for (int j = 0; j < 2000000; ++j);
      
      releaseUnicornSemaphore(0);
    }
}

void longPulseBlueSemaphore() 
{
    while(1)
    {
      aquireUnicornSemaphore(0);
      
      ledRedOff();
      ledBlueOff();
      ledGreenOff();
    
      ledBlueOn();
      for (int j = 0; j < 2000000; ++j);
      
      releaseUnicornSemaphore(0);
    }
}

void longPulseGreenSemaphore() 
{
    while(1)
    {
      aquireUnicornSemaphore(0);
      
      ledRedOff();
      ledBlueOff();
      ledGreenOff();
    
      ledGreenOn();
      for (int j = 0; j < 2000000; ++j);
      
      releaseUnicornSemaphore(0);
    }
}

//***user tasks to be loaded at startup***
void userTaskLoad()
{

  ///////////////////////////////////////////////////////////////////
  // ADD STARTUP TASKS BELOW THIS LINE
  ///////////////////////////////////////////////////////////////////

/*
  
  // all same priority with task exit
  int i;
  for(i = 0; i < 3; ++i)
  {
    int j;
    startNewTask(singleBlinkRedExit, 1);
    for (j = 0; j < 1000000; ++j);
    startNewTask(singleBlinkBlueExit, 1);
    for (j = 0; j < 1000000 / 3 ; ++j);
    startNewTask(singleBlinkGreenExit, 1);
    for (j = 0; j < 1000000; ++j);
  }
  
  
  // differing priorities with timeoutSleep
  startNewTask(blinkRedFastExit, 2);
  startNewTask(blinkBlueMedExit, 3);
  startNewTask(blinkGreenSlowExit, 4);

  timeoutSleep(30000); // sleep for 30 seconds

*/
  
  // semaphore testing
  startNewTask(longPulseRedSemaphore, 1);
  startNewTask(longPulseBlueSemaphore, 1);
  startNewTask(longPulseGreenSemaphore, 1);
  
  exitTask();
  
  ///////////////////////////////////////////////////////////////////
  // NO CHANGES BELOW THIS LINE IN THIS FUNCTION
  ///////////////////////////////////////////////////////////////////
  
}

