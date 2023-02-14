#include "unicorn.h"
#include "qassert.h" // Q_ASSERT
#include "TM4C123GH6PM.h" // NVIC_SystemReset()
#include "locks.h"

Q_DEFINE_THIS_FILE   // required for using Q_ASSERT

Task taskTable[MAX_TASKS + 1];
Task* volatile currentTask;     // must be here because it gets labelled for easier access in assembly code
Task* volatile nextTask;        // must be here because it gets labelled for easier access in assembly code

uint8_t  numTasks;              // the total number of Tasks initialized into taskTable (includes idle task);
uint8_t  currTaskIdx;           // taskTable index of the task currently running (or about to be run in sched)
uint32_t readyTasks;            // bitmask where set bits correspond to ready task indices, unset bits to dormant task indices in taskTable
uint32_t sleepingTasks;         // bitmask of Tasks with active timeout values (nonzero)
SpinLock taskChangeLock;

// decrement timeout on sleepingTasks bitmask
// inspired by the great Miro Samek's version: https://github.com/QuantumLeaps/modern-embedded-programming-course/tree/main/lesson-26
void decrementTimeouts(void)
{
  uint32_t tasksToDecrement = sleepingTasks;
  uint32_t priorityBit;
  
  while(tasksToDecrement != 0U)
  {
    Task *tsk = &(taskTable[getPriority(tasksToDecrement)]);  // get highest priority task
    Q_ASSERT((tsk != (Task *)0) && (tsk->timeout != 0U));     // if a null task had a timeout or that timeout was already zero we got issues
    
    priorityBit = (1U << (tsk->priority));
    --tsk->timeout;
    if(tsk->timeout == 0U)
    {
      readyTasks     |= priorityBit;  // set Task to ready if timer runs out
      sleepingTasks  &= ~priorityBit;  // remove Task from sleepingTasks if it's timeout reached 0
    }
    
    tasksToDecrement &= ~priorityBit;  // remove task from task timeouts to decrement
  }
}
  /*uint8_t i;
  for (i=1U; i<MAX_TASKS; i++)
  {
    Task* tsk  = &(taskTable[i]); 
    if (tsk->timeout != 0U)
    {
      --tsk->timeout;
      if (tsk->timeout == 0U)
        readyTasks |= (1U << (i));
    }
  }*/


// put currentTask in blocking unready state for 'ticks' number of SysTick cycles
void sleep(uint32_t ticks)
{
    __asm("CPSID I");                           // disable interrupts
    
    // don't allow idleTask to enter sleep (REQUIRE same as ASSERT)
    Q_REQUIRE(currentTask != &(taskTable[0]));
    
    currentTask->timeout = ticks;
  
    readyTasks    &= ~(1U << (currentTask->priority));       // set task to not ready
    sleepingTasks |=  (1U << (currentTask->priority));       // set task as sleeping
    
    sched();                                                 // schedule to another task
    __asm("CPSIE I");
}   



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
  target->pc   = (uint32_t)taskFunc; //task function entry point
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

// busy work for the idle task to perform
void onIdle()
{
  for(;;);
}

//starting setup of the task table lock, task table, and idle task
void initializeScheduler()
{
  initLock(&taskChangeLock);

  currentTask   = (Task*)0U;
  nextTask      = (Task*)0U;
  numTasks      = 0U;
  readyTasks    = 0U;
  sleepingTasks = 0U;
  
  // initialize the idle task
  readyNewTask(onIdle, 0);
}

/** peforms initial Task setup and marks new task as ready
*INPUTS: 
*  EntryFunction taskFunc :  function pointer for function the task uses
*  uint8_t priority       :  the scheduling priority of the Task (higher=more priority)
*/
void readyNewTask(EntryFunction taskFunc, uint8_t priority)
{  
  acquireLock(&taskChangeLock); //prevent any other tasks from making concurrent changes to the task table
  
  Task* tsk           = &(taskTable[priority]);
  
  // set the priority of the Task:
  tsk->priority       = priority; 
  //uint8_t taskIndex   = getDormantTaskIndex();
  
  // Task index in taskTable corresponds to its priority
  Q_REQUIRE(priority <= MAX_TASKS);                // check priority fits in taskTable 
  Q_REQUIRE(&(taskTable[priority]) != (Task *)0);  // check that priority is free (can't share priorities)
  
  uint32_t stackEnd   = (uint32_t)tsk->stack + (TASK_STACK_WORD_SIZE * BYTES_PER_WORD); //later the stack size will be parameterized
  uint32_t remainder  = stackEnd % 8U;
  
  stackEnd -= remainder; //Cortex-M stack must be alligned at 8-byte boundary
  
  //initialize the first (mostly fabricated) context frame
  tsk->sp = (uint32_t*)(stackEnd - sizeof(ContextFrame));
  ContextFrame* firstFrame = (ContextFrame*)(tsk->sp);
  initializeFirstFrame(firstFrame, taskFunc);
  
  //tsk->sp now points to the last used word in stack memory

  // set Task->timeout to 0  (Systick will ignore it for decrementing )
  tsk->timeout = 0U;
  
  
  ++numTasks; // increment number of initialized tasks
  readyTasks |= (1U << priority); // mark task as ready (set readyTasks mask)
  
  releaseLock(&taskChangeLock); 
}

//marks the caller as dormant and enters the scheduler, never to return to the caller
//this would cause problems if it were ever called on the idle task
void exitTask()
{
  acquireLock(&taskChangeLock); //prevent any other tasks from making concurrent changes to the task table

  --numTasks; // decrement number of initialized tasks
  readyTasks &= ~(1U << currTaskIdx); // mark task as dormant (unset set readyTasks mask)
  
  releaseLock(&taskChangeLock);

  __asm("CPSID I"); //disable interrupts
  sched(); //schedule next task and set PendSV to trigger (as soon as interrupts are enabled)
  __asm("CPSIE I"); //enable interrupts)  
  
  //here we should be diverted to PendSV handler never to return
}

//updates nextTaskIdx and nextTask
//interrupts must be disabled when this function is called
//assumes numTasks > 0
void sched()
{
  
  
  if(readyTasks == 1U)  // only idle task is ready
  {  
    nextTask = &(taskTable[0]);
  }
  else // at least one user task is ready (this may be the current task)
  {    
    nextTask = &(taskTable[getPriority(readyTasks)]); // set nextTask to Task with most sig. bit set in readyTasks
    Q_ASSERT(nextTask != (Task *)0);                   // sanity check that you're not schedulinga non-existent task
  }
  
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

