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
  UART5_SendByte(taskIndex);
}

// task functions

void task_sleepBlink1()
{
  taskSignal(0U);
  sleep(4);    
  exitTask();
}

void task_sleepBlink2()
{
  taskSignal(1U);
  sleep(4);    
  exitTask();
}

void task_sleepBlink3()
{
  taskSignal(2U);
  sleep(4);    
  exitTask();
}

void task_sleepBlink4()
{
  taskSignal(3U);
  sleep(4);    
  exitTask();
}

void task_sleepBlink5()
{
  taskSignal(4U);
  sleep(4);    
  exitTask();
}

void task_sleepBlink6()
{
  taskSignal(5U);
  sleep(4);    
  exitTask();
}

void task_sleepBlink7()
{
  taskSignal(6U);
  sleep(4);    
  exitTask();
}

void task_sleepBlink8()
{
  taskSignal(7U);
  sleep(4);    
  exitTask();
}

void task_sleepBlink9()
{
  taskSignal(8U);
  sleep(4);    
  exitTask();
}

void task_sleepBlink10()
{
  taskSignal(9U);
  sleep(4);    
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
