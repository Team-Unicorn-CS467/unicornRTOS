#include "usertasks.h"


///////////////////////////////////////////////////////////////////
// ADD ANY USER TASK DEFINITIONS HERE
///////////////////////////////////////////////////////////////////


//each blink function below assumes necessary initial
//setup completed to support blinking

void longPulseRedSemaphore() 
{
    while(1)
    {
      aquireUnicornSemaphore(0);

      //send task signal on 4th pin
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      
      //for (int j = 0; j < 100000; ++j);
      
      releaseUnicornSemaphore(0);
    }
}

void longPulseBlueSemaphore() 
{
    while(1)
    {
      aquireUnicornSemaphore(0);
      
      //send task signal on 4th pin
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      
      //for (int j = 0; j < 100000; ++j);
      
      releaseUnicornSemaphore(0);
    }
}

void longPulseGreenSemaphore() 
{
    while(1)
    {
      aquireUnicornSemaphore(0);
      
      //send task signal on 4th pin
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, HIGH);
      BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW);
      
      //for (int j = 0; j < 100000; ++j);
      
      releaseUnicornSemaphore(0);
    }
}

//***user tasks to be loaded at startup***
void userTaskLoad()
{

  ///////////////////////////////////////////////////////////////////
  // ADD STARTUP TASKS BELOW THIS LINE
  ///////////////////////////////////////////////////////////////////
  
  // semaphore testing
  startNewTask(longPulseRedSemaphore, 1);
  startNewTask(longPulseBlueSemaphore, 1);
  startNewTask(longPulseGreenSemaphore, 1);
  
  exitTask();
  
  ///////////////////////////////////////////////////////////////////
  // NO CHANGES BELOW THIS LINE IN THIS FUNCTION
  ///////////////////////////////////////////////////////////////////
  
}

