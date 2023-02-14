#include "usertasks.h"


static int taskTimer = 10; //5000000;

//***user tasks to be loaded at startup***
void userTaskLoad()
{

  ///////////////////////////////////////////////////////////////////
  // ADD STARTUP TASKS BELOW THIS LINE
  ///////////////////////////////////////////////////////////////////

  ledRedOff();
  ledBlueOff();
  ledGreenOff();
  
  readyNewTask(&blinky1);
  readyNewTask(&blinky2);
  
  for (int j = 0; j < 1000000; ++j);
  readyNewTask(&anti_blinky);
  readyNewTask(&anti_blinky);

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
  int i;
  ledRedOn();
  for(i = 0; i < taskTimer / 10 ; ++i);
  ledRedOff();
  exitTask();
}

void blinkBlue()
{
  int i;
  ledBlueOn();
  for(i = 0; i < taskTimer / 10 ; ++i);
  ledBlueOff();
  exitTask();
}

void blinkGreen()
{
  int i;
  ledGreenOn();
  for(i = 0; i < taskTimer / 10 ; ++i);
  ledGreenOff();
  exitTask();
}

void blinky1() 
{
    while (1) 
    {
        ledBlueOn();
        sleep(3000);
        ledBlueOff();
        sleep(6000);;
        //ledBlueOff();
        //sleep(1);
    }
}

void blinky2() 
{
    while (1) 
    {
        ledRedOn();
        sleep(3000);
        ledRedOff();
        sleep(3000);;
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