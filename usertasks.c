#include "usertasks.h"
#include "bsp.h"


///////////////////////////////////////////////////////////////////
// ADD ANY USER TASK DEFINITIONS HERE
///////////////////////////////////////////////////////////////////

//each function below assumes necessary initial
//setup completed to support I/O


//BSP_setGPIO(GPIOF_AHB, GPIO_PF2, LOW); // first pin
//BSP_setGPIO(GPIOF_AHB, GPIO_PF3, LOW); // second pin
//BSP_setGPIO(GPIOB_AHB, GPIO_PB3, LOW); // third pin
//BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW); // fourth pin
//BSP_setGPIO(GPIOC_AHB, GPIO_PC5, LOW); // fifth pin
//BSP_setGPIO(GPIOC_AHB, GPIO_PC6, LOW); // sixth pin
//BSP_setGPIO(GPIOC_AHB, GPIO_PC7, LOW); // seventh pin
//BSP_setGPIO(GPIOD_AHB, GPIO_PD6, LOW); // eighth pin

GPIOA_Type* taskSignalTypes[10][2] =
{
  {
    GPIOC_AHB,  // pin 4 base
    GPIOC_AHB   // pin 5 base    
  },
  {
    GPIOC_AHB,  // pin 4 base
    GPIOC_AHB   // pin 6 base    
  },
  {
    GPIOC_AHB,  // pin 4 base
    GPIOC_AHB   // pin 7 base    
  },
  {
    GPIOC_AHB,  // pin 4 base
    GPIOD_AHB   // pin 8 base    
  },
  {
    GPIOC_AHB,  // pin 5 base
    GPIOC_AHB   // pin 6 base    
  },
  {
    GPIOC_AHB,  // pin 5 base
    GPIOC_AHB   // pin 7 base    
  },
  {
    GPIOC_AHB,  // pin 5 base
    GPIOD_AHB   // pin 8 base    
  },
  {
    GPIOC_AHB,  // pin 6 base
    GPIOC_AHB   // pin 7 base    
  },
  {
    GPIOC_AHB,  // pin 6 base
    GPIOD_AHB   // pin 8 base    
  },
  {
    GPIOC_AHB,  // pin 7 base
    GPIOD_AHB   // pin 8 base
  }
};

const uint8_t taskSignalPins[10][2] =
{
  {
    GPIO_PC4,   // pin 4 base-relative offset
    GPIO_PC5    // pin 5 base-relative offset
  },
  {
    GPIO_PC4,   // pin 4 base-relative offset
    GPIO_PC6    // pin 6 base-relative offset
  },
  {
    GPIO_PC4,   // pin 4 base-relative offset
    GPIO_PC7    // pin 7 base-relative offset
  },
  {
    GPIO_PC4,   // pin 4 base-relative offset
    GPIO_PD6    // pin 8 base-relative offset
  },
  {
    GPIO_PC5,   // pin 5 base-relative offset
    GPIO_PC6    // pin 6 base-relative offset
  },
  {
    GPIO_PC5,   // pin 5 base-relative offset
    GPIO_PC7    // pin 7 base-relative offset
  },
  {
    GPIO_PC5,   // pin 5 base-relative offset
    GPIO_PD6    // pin 8 base-relative offset
  },
  {
    GPIO_PC6,   // pin 6 base-relative offset
    GPIO_PC7    // pin 7 base-relative offset
  },
  {
    GPIO_PC6,   // pin 6 base-relative offset
    GPIO_PD6    // pin 8 base-relative offset
  },
  {
    GPIO_PC7,   // pin 7 base-relative offset
    GPIO_PD6    // pin 8 base-relative offset
  }  
};

// helper function

void taskSignal(uint8_t taskIndex, GPIO_PinState pState)
{
    BSP_setGPIO(taskSignalTypes[taskIndex][0], taskSignalPins[taskIndex][0], pState);
    BSP_setGPIO(taskSignalTypes[taskIndex][1], taskSignalPins[taskIndex][1], pState);
}

// task functions

void task_sleepBlink1()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(0U, HIGH);
    for(int  i = 0; i < 25; ++i);
    taskSignal(0U, LOW);
    sleep(4);    
  }
  exitTask();
}

void task_sleepBlink2()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(1U, HIGH);
    for(int  i = 0; i < 25; ++i);
    taskSignal(1U, LOW);
    sleep(4);    
  }
  exitTask();
}

void task_sleepBlink3()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(2U, HIGH);
    for(int  i = 0; i < 25; ++i);
    taskSignal(2U, LOW);
    sleep(4);   
  }
  exitTask();
}

void task_sleepBlink4()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(3U, HIGH);
    for(int  i = 0; i < 25; ++i);
    taskSignal(3U, LOW);
    sleep(4);     
  }
  exitTask();
}

void task_sleepBlink5()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(4U, HIGH);
    for(int  i = 0; i < 25; ++i);
    taskSignal(4U, LOW);
    sleep(4);   
  }
  exitTask();
}

void task_sleepBlink6()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(5U, HIGH);
    for(int  i = 0; i < 25; ++i);
    taskSignal(5U, LOW);
    sleep(4);    
  }
  exitTask();
}

void task_sleepBlink7()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(6U, HIGH);
    for(int  i = 0; i < 25; ++i);
    taskSignal(6U, LOW);
    sleep(4);   
  }
  exitTask();
}

void task_sleepBlink8()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(7U, HIGH);
    for(int  i = 0; i < 25; ++i);
    taskSignal(7U, LOW);
    sleep(4);     
  }
  exitTask();
}

void task_sleepBlink9()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(8U, HIGH);
    for(int  i = 0; i < 25; ++i);
    taskSignal(8U, LOW);
    sleep(4);     
  }
  exitTask();
}

void task_sleepBlink10()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(9U, HIGH);
    for(int  i = 0; i < 25; ++i);
    taskSignal(9U, LOW);
    sleep(4);    
  }
  exitTask();
}

void (*tasks[])() = 
{
  task_sleepBlink1,
  task_sleepBlink2,
  task_sleepBlink3,
  task_sleepBlink4,
  task_sleepBlink5,
  task_sleepBlink6,
  task_sleepBlink7,
  task_sleepBlink8,
  task_sleepBlink9,
  task_sleepBlink10,
};

const int taskCounts[5] = {1, 2, 3, 6, 10};

//***user tasks to be loaded at startup***
void userTaskLoad()
{

  ///////////////////////////////////////////////////////////////////
  // ADD STARTUP TASKS BELOW THIS LINE
  ///////////////////////////////////////////////////////////////////
  
  // set all the pins low to start
  BSP_setGPIO(GPIOF_AHB, GPIO_PF2, LOW); // first pin
  BSP_setGPIO(GPIOF_AHB, GPIO_PF3, LOW); // second pin
  BSP_setGPIO(GPIOB_AHB, GPIO_PB3, LOW); // third pin
  BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW); // fourth pin
  BSP_setGPIO(GPIOC_AHB, GPIO_PC5, LOW); // fifth pin
  BSP_setGPIO(GPIOC_AHB, GPIO_PC6, LOW); // sixth pin
  BSP_setGPIO(GPIOC_AHB, GPIO_PC7, LOW); // seventh pin
  BSP_setGPIO(GPIOD_AHB, GPIO_PD6, LOW); // eighth pin

  for(int i = 0; i < 5; ++i)
  {
    for(int j = 0; j < taskCounts[i]; ++j)
      readyNewTask(tasks[j], 10 - j);
    
    sleep(10);
  }

  exitTask();

  ///////////////////////////////////////////////////////////////////
  // NO CHANGES BELOW THIS LINE IN THIS FUNCTION
  ///////////////////////////////////////////////////////////////////
}
