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
//PE4 is UART Tx pin
//PE5 is UART Rx pin

void taskSignal(uint8_t taskIndex)
{
  
  /***UART TASK SIGNALING IMPLEMENTATION***/
  UART5_SendByte(taskIndex);
  
  /***ALTERNATIVE SINGLE PIN TASK SIGNALING IMPLEMENTATION***
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
  */
}

// semaphore task functions

void longPulse1Semaphore() 
{
    while(1)
    {
      aquireUnicornSemaphore(0);
      UART5_SendByte(1U);
      for (int j = 0; j < 10000; ++j);
      releaseUnicornSemaphore(0);
    }
}

void longPulse2Semaphore() 
{
    while(1)
    {
      aquireUnicornSemaphore(0);
      UART5_SendByte(2U);
      for (int j = 0; j < 10000; ++j);
      releaseUnicornSemaphore(0);
    }
}

void longPulse3Semaphore() 
{
    while(1)
    {
      aquireUnicornSemaphore(0);
      UART5_SendByte(3U);
      for (int j = 0; j < 10000; ++j);
      releaseUnicornSemaphore(0);
    }
}

//non-semaphore task functions

const uint32_t TASK_SLEEP_TIME = 3U;

void task_sleepBlink1()
{
  taskSignal(0U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink2()
{
  taskSignal(1U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink3()
{
  taskSignal(2U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink4()
{
  taskSignal(3U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink5()
{
  taskSignal(4U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink6()
{
  taskSignal(5U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink7()
{
  taskSignal(6U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink8()
{
  taskSignal(7U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink9()
{
  taskSignal(8U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink10()
{
  taskSignal(9U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink11()
{
  taskSignal(10U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

void task_sleepBlink12()
{
  taskSignal(11U);
  timeoutSleep(TASK_SLEEP_TIME);    
  exitTask();
}

#define TOTAL_TASK_COUNT 12 // must be < MAX_TASKS to avoid overwriting usertTaskLoad() priority - see main()
void (*tsks[TOTAL_TASK_COUNT])() = 
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
  task_sleepBlink11,
  task_sleepBlink12,
};

#define RUN_COUNT 4
const int taskCounts[RUN_COUNT] = {1, 2, 6, 12}; // value of last element must be <= TOTAL_TASK_COUNT

//***user tasks to be loaded at startup***
void userTaskLoad()
{

  ///////////////////////////////////////////////////////////////////
  // ADD STARTUP TASKS BELOW THIS LINE
  ///////////////////////////////////////////////////////////////////
  
  //semaphore implementation
  startNewTask(longPulse1Semaphore, 1);
  startNewTask(longPulse2Semaphore, 1);
  startNewTask(longPulse3Semaphore, 1);
  exitTask();
  
  //non-semaphore implementation
  for(int i = 0; i < RUN_COUNT; ++i)
  {
    for(int j = 0; j < taskCounts[i]; ++j)
      readyNewTask(tsks[j], TOTAL_TASK_COUNT - j);
    timeoutSleep(6);
  }
  exitTask();

  ///////////////////////////////////////////////////////////////////
  // NO CHANGES BELOW THIS LINE IN THIS FUNCTION
  ///////////////////////////////////////////////////////////////////
}