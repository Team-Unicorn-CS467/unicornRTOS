#include "usertasks.h"
#include "bsp.h"


///////////////////////////////////////////////////////////////////
// ADD ANY USER TASK DEFINITIONS HERE
///////////////////////////////////////////////////////////////////

//each function below assumes necessary initial
//setup completed to support I/O

//{GPIOF_AHB, GPIO_PF1} - RED LED
//{GPIOF_AHB, GPIO_PF2} - BLUE LED
//{GPIOF_AHB, GPIO_PF3} - GREEN LED
//{GPIOB_AHB, GPIO_PB3} - PendSV Signal to LA
//{GPIOC_AHB, GPIO_PC4} - SysTick Signal to LA
//{GPIOC_AHB, GPIO_PC5} - Sched Signal to LA
//{GPIOC_AHB, GPIO_PC6} - External Interrupt
//{GPIOC_AHB, GPIO_PC7} - Task(s) Signal to LA

void taskSignal(uint8_t taskIndex)
{
  uint32_t leadSpace = (17U - taskIndex) / 2; // assumes a max of 16 tasks
  uint32_t lagSpace = 17U - (taskIndex + leadSpace); // assumes a max of 16 tasks
  
  // entry flag
  BSP_setGPIO(GPIOC_AHB, GPIO_PC7, HIGH);
  BSP_setGPIO(GPIOC_AHB, GPIO_PC7, HIGH);

  // representation of task index
  while(leadSpace > 0U)
  {
    BSP_setGPIO(GPIOC_AHB, GPIO_PC7, LOW);
    --leadSpace;
  }
  while(taskIndex > 0U)
  {
    BSP_setGPIO(GPIOC_AHB, GPIO_PC7, HIGH);
    BSP_setGPIO(GPIOC_AHB, GPIO_PC7, LOW);
    --taskIndex;
  }
  while(lagSpace > 0U)
  {
    BSP_setGPIO(GPIOC_AHB, GPIO_PC7, LOW);
    --lagSpace;
  }

  // exit flag
  BSP_setGPIO(GPIOC_AHB, GPIO_PC7, HIGH);
  BSP_setGPIO(GPIOC_AHB, GPIO_PC7, HIGH);
  
  BSP_setGPIO(GPIOC_AHB, GPIO_PC7, LOW);
}

// task functions

void task_sleepBlink1()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(0U);
    for(int  i = 0; i < 25; ++i);
    sleep(4);    
  }
  exitTask();
}

void task_sleepBlink2()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(1U);
    for(int  i = 0; i < 25; ++i);
    sleep(4);    
  }
  exitTask();
}

void task_sleepBlink3()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(2U);
    for(int  i = 0; i < 25; ++i);
    sleep(4);   
  }
  exitTask();
}

void task_sleepBlink4()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(3U);
    for(int  i = 0; i < 25; ++i);
    sleep(4);     
  }
  exitTask();
}

void task_sleepBlink5()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(4U);
    for(int  i = 0; i < 25; ++i);
    sleep(4);   
  }
  exitTask();
}

void task_sleepBlink6()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(5U);
    for(int  i = 0; i < 25; ++i);
    sleep(4);    
  }
  exitTask();
}

void task_sleepBlink7()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(6U);
    for(int  i = 0; i < 25; ++i);
    sleep(4);   
  }
  exitTask();
}

void task_sleepBlink8()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(7U);
    for(int  i = 0; i < 25; ++i);
    sleep(4);     
  }
  exitTask();
}

void task_sleepBlink9()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(8U);
    for(int  i = 0; i < 25; ++i);
    sleep(4);     
  }
  exitTask();
}

void task_sleepBlink10()
{
  for(int j = 0; j < 2; ++j)
  {
    taskSignal(9U);
    for(int  i = 0; i < 25; ++i);
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
  BSP_setGPIO(GPIOF_AHB, GPIO_PF2, LOW); // BLUE LED
  BSP_setGPIO(GPIOF_AHB, GPIO_PF3, LOW); // GREEN LED
  BSP_setGPIO(GPIOB_AHB, GPIO_PB3, LOW); // PendSV Signal to LA
  BSP_setGPIO(GPIOC_AHB, GPIO_PC4, LOW); // SysTick Signal to LA
  BSP_setGPIO(GPIOC_AHB, GPIO_PC5, LOW); // Sched Signal to LA
  BSP_setGPIO(GPIOC_AHB, GPIO_PC6, LOW); // External Interrupt
  BSP_setGPIO(GPIOC_AHB, GPIO_PC7, LOW); // Task(s) Signal to LA
  
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
