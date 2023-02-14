#include "usertasks.h"

static int taskTimer = 10; //5000000;
static int i;

//***user tasks to be loaded at startup***
void userTaskLoad()
{

  ///////////////////////////////////////////////////////////////////
  // ADD STARTUP TASKS BELOW THIS LINE
  ///////////////////////////////////////////////////////////////////
  

  while(1)
  {
    readyNewTask(&blinkRed);
    for(i = 0; i < taskTimer; ++i);
    sleep(2);
    
    readyNewTask(&blinkBlue);
    for(i = 0; i < taskTimer; ++i);
    sleep(1);
    
    
    readyNewTask(&blinkGreen); 
    for(i = 0; i < taskTimer; ++i);
    sleep(1);
  }

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