#include "usertasks.h"


static int taskTimer = 1000000;

//***user tasks to be loaded at startup***
void userTaskLoad()
{

  ///////////////////////////////////////////////////////////////////
  // ADD STARTUP TASKS BELOW THIS LINE
  ///////////////////////////////////////////////////////////////////

  while(1)
  {
    startNewTask(blinkRed, 1);
    
    //sleep(500);
    for (int i = 0; i < taskTimer / 3; ++i);
    
    startNewTask(blinkBlue, 1);
    
    //sleep(500);
    for (int i = 0; i < taskTimer / 3 ; ++i);
    
    startNewTask(blinkGreen, 1);
    
    //sleep(4000);
    for (int i = 0; i < taskTimer; ++i);
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
  ledRedOff();
  ledBlueOff();
  ledGreenOff();
  
  ledRedOn();
  
  //sleep(3000);
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
  
  //sleep(2000);
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
  
  //sleep(1000);
  for (int i = 0; i < taskTimer; ++i);
  
  ledGreenOff();
  
  exitTask();
}

void blinky1() 
{
    while (1) 
    {
        ledBlueOn();
        //sleep(500);
        ledBlueOff();
        //sleep(250);;
        //ledBlueOff();
        //sleep(1);
    }
}

void blinky2() 
{
    while (1) 
    {
        ledRedOn();
        //sleep(300);
        ledRedOff();
        //sleep(300);;
        //ledRedOff();
        //sleep(1);
    }
}

void anti_blinky() 
{
  ledRedOff();
  ledBlueOff();
  exitTask();
}