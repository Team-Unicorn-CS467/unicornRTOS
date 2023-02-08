#include "unicorn.h"


Task idleTask;
Task taskTable[MAX_TASKS];
Task* currentTask; //must be here because it gets labelled for easier access in assembly code
Task* nextTask; //must be here because it gets labelled for easier access in assembly code

//used to create the starting, mostly fake, ContextFrame in a new task's stack memory
void initializeFirstFrame(ContextFrame* target, EntryFunction taskFunc)
{
  //throw away values which will be easy to spot in stack memory
  target->r4  = 0x0000000DU;
  target->r5  = 0x0000000CU;
  target->r6  = 0x0000000BU;
  target->r7  = 0x0000000AU;
  target->r8  = 0x00000009U;
  target->r9  = 0x00000008U;
  target->r10 = 0x00000007U;
  target->r11 = 0x00000006U;
  target->r0  = 0x00000005U;
  target->r1  = 0x00000004U; 
  target->r2  = 0x00000003U; 
  target->r3  = 0x00000002U;
  target->r12 = 0x00000001U;
  target->lr  = 0x00000000U;
  
  //values which actually matter
  target->pc = (uint32_t)taskFunc; //task function entry point
  target->xpsr = (1U << 24); //program status register value for "thumb state"
}

//peforms initial Task setup
//taskFunc is this task's method
void initializeTask(Task* target, EntryFunction taskFunc)
{
  uint32_t stackEnd = (uint32_t)target->stack + (TASK_STACK_WORD_SIZE * BYTES_PER_WORD); //later the stack size will be parameterized
  uint32_t remainder = stackEnd % 8U;
  stackEnd -= remainder; //Cortex-M stack must be alligned at 8-byte boundary
  
  //initialize the first (mostly fabricated) context frame
  target->sp = (uint32_t*)(stackEnd - sizeof(ContextFrame));
  ContextFrame* firstFrame = (ContextFrame*)(target->sp);
  initializeFirstFrame(firstFrame, taskFunc);
  
  //target->sp now point to the last used word in stack memory
  
  target->state = TASK_STATE_READY;
};

void pointlessWork()
{
  while(1);
}

//starting setup of the task table, idle task
void initializeScheduler()
{
  for(int i = 0; i < MAX_TASKS; ++i)
    taskTable[i].state = TASK_STATE_DORMANT;
  
  initializeTask(&idleTask, &pointlessWork);
  
  currentTask = (Task*)0U;
  nextTask = (Task*)0U;
}

void readyTaskStart(EntryFunction taskFunc)
{  
  unsigned int i;
  
  //find a dormant task
  for (i = 0; i < MAX_TASKS; ++i)
  {
    if(taskTable[i].state == TASK_STATE_DORMANT)
      break;
  }

  //***NEED AN ASSERT HERE THAT i < MAX_TASKS
  
  //initialize the dormant task and mark as ready
  initializeTask(&(taskTable[i]), taskFunc);
  
  // is this redundant?  initializeTask puts target->state = TASK_STATE_READY
  taskTable[i].state = TASK_STATE_READY; 
}

void readyTaskEnd()
{
  if(currentTask == (Task*)0U || currentTask == &idleTask) //sanity check
    return;
  
}

//interrupts must be disabled when this function is called
//updates currentTask and nextTask and their states
void sched()
{
  int i;
  int readyTaskCount = 0;
  int activeTaskIndex = -1;
  for(i = 0; i < MAX_TASKS; ++i)
  {
    if(taskTable[i].state == TASK_STATE_READY)
      ++readyTaskCount;
    else if(taskTable[i].state == TASK_STATE_ACTIVE)
      activeTaskIndex = i;
  }

  if(readyTaskCount == 0)
  {
    currentTask = &idleTask;
    nextTask = &idleTask;
  }

  else if (activeTaskIndex == -1) //current task is the idleTask and other task(s) ready to run
  {
    for(i = 0; i < MAX_TASKS; ++i)
    {
      if(taskTable[i].state == TASK_STATE_READY)
        break;
    }
    currentTask = &idleTask;
    nextTask = &(taskTable[i]);
    nextTask->state = TASK_STATE_ACTIVE;
  }
  
  else //active task is not the idleTask
  {
    //find the ready task which is "next in line"
    i = activeTaskIndex + 1;
    do
    {
      if(i == MAX_TASKS)
        i = 0;
      else if(taskTable[i].state == TASK_STATE_READY)
        break;
      else
        ++i;
    } while(i != activeTaskIndex);
    currentTask = &(taskTable[activeTaskIndex]);
    currentTask->state = TASK_STATE_READY;
    nextTask = &(taskTable[i]);
    nextTask->state = TASK_STATE_ACTIVE;
  }
}