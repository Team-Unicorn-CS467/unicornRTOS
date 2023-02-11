#include "unicorn.h"
#include "qassert.h" // Q_ASSERT
#include "TM4C123GH6PM.h" // NVIC_SystemReset()

Q_DEFINE_THIS_FILE   // required for using Q_ASSERT

//Task idleTask;
Task taskTable[MAX_TASKS + 1];
Task* volatile currentTask; //must be here because it gets labelled for easier access in assembly code
Task* volatile nextTask; //must be here because it gets labelled for easier access in assembly code

uint8_t  numTasks;       // the total number of Tasks initialized into taskTable;
uint8_t  currTaskIdx;    // index of the active task in taskTable
uint32_t readyTasks;    // bitmask where asserted bit index = index of ready task in taskTable

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

uint8_t getDormantTaskIndex()
{  
  uint8_t i;
  
  //find a dormant task
  for (i = 0; i < MAX_TASKS; ++i)
  {
    // if readTasks[i] is 0, taskTable[i] is dormant
    if ((readyTasks & (1U << i)) == 0) break;
  }

  //no dormant tasks!!!
  Q_ASSERT(i < MAX_TASKS);
  
  return i;
};

// busy work for the idleTask to perform
void onIdle()
{
  for(;;);
}

//starting setup of the task table, idle task
void initializeScheduler()
{
  currentTask = (Task*)0U;
  nextTask = (Task*)0U;
  numTasks = 0U;
  readyTasks = 0U;
  
  // initialize the idle task
  readyNewTask(onIdle);
}

//peforms initial Task setup
//taskFunc is this task's method
void readyNewTask(EntryFunction taskFunc)
{  

  uint8_t taskIndex = getDormantTaskIndex();
  Task* tsk = &(taskTable[taskIndex]);
  
  uint32_t stackEnd = (uint32_t)tsk->stack + (TASK_STACK_WORD_SIZE * BYTES_PER_WORD); //later the stack size will be parameterized
  uint32_t remainder = stackEnd % 8U;
  stackEnd -= remainder; //Cortex-M stack must be alligned at 8-byte boundary
  
  //initialize the first (mostly fabricated) context frame
  tsk->sp = (uint32_t*)(stackEnd - sizeof(ContextFrame));
  ContextFrame* firstFrame = (ContextFrame*)(tsk->sp);
  initializeFirstFrame(firstFrame, taskFunc);
  
  //tsk->sp now points to the last used word in stack memory
  
  // increment number of initialized tasks:
  ++numTasks;
  
  // set the new Task into the ready state (set readyTasks mask)
  readyTasks |= (1U << taskIndex); 
  
}

//interrupts must be disabled when this function is called
//updates currentTask and nextTask and their states
void sched()
{
  
  if(readyTasks == 0U)  // no threads running ? switch to idleTask
    currTaskIdx = 0U;
  else 
  {
    do 
    {
      ++currTaskIdx;
      if (currTaskIdx == numTasks)  // iterate only over initialized Tasks, if counter at highest filled index in taskTable ? --> start at 1 to skip idleTask
        currTaskIdx =  1U;
      
       nextTask = &(taskTable[currTaskIdx]);
        
    } while ( (readyTasks & (1U << (currTaskIdx - 1U))) == 0U ); // task index 0 at taskTable[1]
  } 
  nextTask = &(taskTable[currTaskIdx]);
  
  // comparing currentTask and nextTask (two volatile variables) is leads to
  // unpredictable access order *Warning[Pa082]* see:
  // https://www.iar.com/knowledge/support/technical-notes/compiler/warningpa082-undefined-behavior-the-order-of-volatile-accesses-is-undefined-in-this-statement/
  Task const *next = nextTask;
  if (currentTask != next)
      //NVIC_INT_CTRL_R |= 0x10000000U; //trigger PendSV exception via interrupt control and state register in system control block
      *(uint32_t volatile *)0xE000ED04 = (1U << 28U);  
}

void Q_onAssert(char const *module, int loc) {
    /* TBD: damage control */
    (void)module; /* avoid the "unused parameter" compiler warning */
    (void)loc;    /* avoid the "unused parameter" compiler warning */
    
    // inlined function copied from core_cm4.h (line 1790)
    // because using #include core_cm4.h was causing all
    // kinds of havoc
    NVIC_SystemReset();
    
}

