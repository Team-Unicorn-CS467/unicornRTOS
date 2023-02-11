#include "usertasks.h"


//***user tasks to be loaded at startup***
void userTaskLoad()
{

  ///////////////////////////////////////////////////////////////////
  // ADD STARTUP TASKS BELOW THIS LINE
  ///////////////////////////////////////////////////////////////////
  
  readyNewTask(&blinkRed);
  readyNewTask(&blinkBlue);
  readyNewTask(&blinkGreen);

  ///////////////////////////////////////////////////////////////////
  // NO CHANGES BELOW THIS LINE IN THIS FUNCTION
  ///////////////////////////////////////////////////////////////////

  while(1); //LATER THIS WILL BE CHANGED TO A TASK EXIT CALL
  
}

///////////////////////////////////////////////////////////////////
// ADD ANY USER TASK DEFINITIONS HERE
///////////////////////////////////////////////////////////////////

const int BLINK_TIME = 2000;

//each blink function below assumes necessary initial
//setup completed to support blinking

void blinkRed()
{
    int i;
    while(1)
    {
      acquireSpinLock();
      ledRedOn();
      for(i = 0; i < BLINK_TIME; ++i);
      releaseSpinLock();
      ledRedOff();
      for(i = 0; i < BLINK_TIME; ++i);
    }
}

void blinkBlue()
{
    int i;
    while(1)
    {
      acquireSpinLock();
      ledBlueOn();
      for(i = 0; i < BLINK_TIME; ++i);
      releaseSpinLock();
      ledBlueOff();
      for(i = 0; i < BLINK_TIME; ++i);
    }
}

void blinkGreen()
{
    int i;
    while(1)
    {
      acquireSpinLock();
      ledGreenOn();
      for(i = 0; i < BLINK_TIME; ++i);
      releaseSpinLock();
      ledGreenOff();
      for(i = 0; i < BLINK_TIME; ++i);
    }
}