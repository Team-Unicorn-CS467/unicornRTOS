#include "unicorn.h"
#include "qassert.h" // Q_ASSERT
#include "TM4C123GH6PM.h" // NVIC_SystemReset()
#include "locks.h"


Q_DEFINE_THIS_FILE   // required for using Q_ASSERT


struct Task tasks[MAX_TASKS + 1]; // table used for storing task resources; not used for direct access

///////////////////////////////////////////////////////////////////
// TASK STATE ACCESS/CHANGE STUFF
//
// tasks will be divided here by state 
// each state appears in all caps, but
// the states are conceptual, not actually
// implemented in any monolithic way
//
///////////////////////////////////////////////////////////////////


/*** DORMANT STATE ***/

struct Task* dormantStack[MAX_TASKS + 1];
uint32_t dormantCount;
MutexLock dormantStackLock;


/*** READY STATE ***/


// linked list of tasks which share the same priority
typedef struct
{
  struct Task* head;
  struct Task* tail;
} PriorityLevel;


// data structure to index into different priority levels
PriorityLevel readyTasks[PRIORITY_COUNT];
uint32_t readyCount;            //the total number of tasks ready to run in readyTasks
uint32_t topPriority;           //this gets set to the highest priority available in readyNewTask()
MutexLock readyTasksLock;

// assumes readyTasksLock is held when called
// assumes taskToAdd != NULL
// assumes taskToAdd->priority is within range
void addTaskToReadyTasks(struct Task* taskToAdd)
{
    uint32_t p = taskToAdd->priority;
    if (p > topPriority) // a new top priority is established, taskToAdd is the only node at this priority
    {
      topPriority = p;
      readyTasks[p].head = taskToAdd;
      readyTasks[p].tail = taskToAdd;
    }
    else
    {
      
      // empty at this priority level
      if(readyTasks[p].tail == (struct Task*)(0U))
      {
        readyTasks[p].head = taskToAdd;
        readyTasks[p].tail = taskToAdd;        
      }
      
      // not empty at this priority level
      else
      {
        readyTasks[p].tail->next = taskToAdd;
        readyTasks[p].tail = taskToAdd;
      }
      
    }
    taskToAdd->next = (struct Task*)(0U);
    ++readyCount;
}

// assumes readyTasksLock is held when called
// pops/returns the next ready task in line at top priority
struct Task* getNextReadyTask()
{
  struct Task* returnTask = readyTasks[topPriority].head;
  readyTasks[topPriority].head = returnTask->next;
  if (readyTasks[topPriority].head == (struct Task*)(0U)) // now empty at this priority level
  {
    readyTasks[topPriority].tail = (struct Task*)(0U);
    do
    {
      --topPriority; // find the next highest populated priority level
    }while(readyTasks[topPriority].head == (struct Task*)(0U));
  }
  --readyCount;
  return returnTask;
}
    
/*

Task* readyHeap[MAX_TASKS + 1];



// assumes taskToAdd != NULL
// assumes readySize <= MAX_TASKS before call
void addTaskToReadyHeap(Task* taskToAdd)
{
  uint32_t i = readySize;
  
  // percolate up in the heap
  while(i > 0)
  {
    uint32_t j = (i - 1) / 2; //index of parent node in the heap
    if (readyHeap[j]->priority >= taskToAdd->priority)
      break; // i is correct heap position for taskToAdd
    
    readyHeap[i] = readyHeap[j]; // move the parent down
    i = j; // the parent index is the new child index
  }
  readyHeap[i] = taskToAdd;
  ++readySize;
}


// returns the highest priority READY, popping it off of readyHeap
// reduces readySize by 1
// assumes that readySize > 0 at the time of calling
Task* popTaskFromReadyHeap()
{
  --readySize;
  Task* lowPrioTask = readyHeap[readySize];
  Task* maxPrioTask = readyHeap[0U];
  
  // find good new position for lowPrioTask
  uint32_t i = 0U;  
  uint32_t j = 1U; // left child
  uint32_t k = 2U; //right child
  while (j < readySize)
  {
    if (k < readySize)
    {
      if (readyHeap[k]->priority > readyHeap[j]->priority)
        j = k;
    }
    
    // j now the index of the higher priority both child nodes
    
    if (lowPrioTask->priority >= readyHeap[j]->priority)
      break; //heap integrity maintained
    
    readyHeap[i] = readyHeap[j]; // shift node up
    i = j; // child index is the new parent index
    j = (i * 2) + 1; // left child
    k = j + 1; // right child      
  }
  readyHeap[i] = lowPrioTask;
  return maxPrioTask;
}

*/

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
  target->pc   = (uint32_t)(taskFunc); //task function entry point
  target->xpsr = (1U << 24); //program status register value for "thumb state"
}


/** peforms initial Task setup and marks new task as ready
*INPUTS: 
*  EntryFunction taskFunc :  function pointer for function the task uses
*  uint8_t priority       :  the scheduling priority of the Task (higher=more priority)
*/
void readyNewTask(EntryFunction taskFunc, uint8_t priority)
{  
  // aqcuire a dormant task
  acquireLock(&dormantStackLock); //prevent concurrent changes to dormantStack
  Q_ASSERT(dormantCount > 0);
  --dormantCount;
  struct Task* tsk = dormantStack[dormantCount];
  releaseLock(&dormantStackLock);
  
  // initial task stack setup (with the first, mostly fabricated context frame)
  uint32_t stackEnd   = (uint32_t)(tsk->stack) + (TASK_STACK_WORD_SIZE * BYTES_PER_WORD); //later the stack size will be parameterized
  uint32_t remainder  = stackEnd % 8U;
  stackEnd -= remainder; //Cortex-M stack must be alligned at 8-byte boundary
  tsk->sp = (uint32_t*)(stackEnd - sizeof(ContextFrame));
  ContextFrame* firstFrame = (ContextFrame*)(tsk->sp);
  initializeFirstFrame(firstFrame, taskFunc);
  
  // tsk->sp now points to the last used word in stack memory

  // initialize other task properties
  Q_ASSERT(priority < PRIORITY_COUNT);  
  tsk->priority = priority;
  tsk->timeout = 0U;
  tsk->lockChannel = (MutexLock*)(0U);
  
  //add the task to readyTasks structure
  acquireLock(&readyTasksLock); //prevent concurrent changes to readyHeap
  addTaskToReadyTasks(tsk);
  releaseLock(&readyTasksLock);
}


void startNewTask(EntryFunction taskFunc, uint8_t priority)
{
  readyNewTask(taskFunc, priority);
  sched();
}


/*** TICK_SLEEP STATE ***/



/*** LOCK_SLEEP STATE ***/


/*** RUNNING STATE ***/

struct Task* volatile currentTask;     // must be here because it gets labelled for easier access in assembly code
struct Task* volatile nextTask;        // must be here because it gets labelled for easier access in assembly code
MutexLock runningLock;







// busy work for the idle task to perform
void onIdle()
{
  for(;;);
}


//starting setup of everything the scheduler needs to run
void initializeScheduler()
{
  
  // locks
  initLock(&dormantStackLock);
  initLock(&readyTasksLock);
  initLock(&runningLock);
  
  // initialize dormantStack (populate with every member of tasks)
  acquireLock(&dormantStackLock);
  for(int i = MAX_TASKS; i >= 0; --i)
    dormantStack[i] = &(tasks[i]);
  dormantCount = MAX_TASKS + 1;
  releaseLock(&dormantStackLock);

  // initialize readyTasks
  acquireLock(&readyTasksLock);
  for(int i = 0; i < PRIORITY_COUNT; ++i)
  {
    readyTasks[i].head = (struct Task*)(0U);
    readyTasks[i].tail = (struct Task*)(0U);
  }
  readyCount = 0U;
  topPriority = 0U; // assumes idle task is about to be loaded
  releaseLock(&readyTasksLock);
  
  acquireLock(&runningLock);
  currentTask = (struct Task*)0U;
  releaseLock(&runningLock);

  // start the idle task at lowest priority
  readyNewTask(onIdle, 0);
}


/*

// decrement timeout on sleepingTasks bitmask
// inspired by the great Miro Samek's version: https://github.com/QuantumLeaps/modern-embedded-programming-course/tree/main/lesson-26
// assumes that interrupts are disabled when called, so no taskChangeLock is aquired
void decrementTimeouts(void)
{
  uint32_t tasksToDecrement = timeoutSleepTasks;
  uint32_t priorityBit;
  
  while(tasksToDecrement != 0U)
  {
    Task *tsk = &(taskTable[getHighestSetBit(tasksToDecrement)]);  // get highest priority task
    
    Q_ASSERT(tsk->timeout != 0U);     // if a timeout was already zero we got issues
    
    priorityBit = (1U << (tsk->priority));
    --tsk->timeout;
    if(tsk->timeout == 0U) // Task timeout just reached 0
    {
      timeoutSleepTasks &= ~priorityBit;  // remove Task from timeoutSleepTasks

      if ((channelSleepTasks & priorityBit) == 0U) // Task is not currently also sleeping on any sleep channel
        readyTasks |= priorityBit;  // set Task to ready
    }
    
    tasksToDecrement &= ~priorityBit;  // remove task from task timeouts to decrement
  }
}


// put currentTask in blocking unready state for 'ticks' number of SysTick cycles
void timeoutSleep(uint32_t ticks)
{   
    acquireLock(&taskChangeLock); //prevent any other tasks from making concurrent changes to the task table
    
    // don't allow idleTask to enter sleep (REQUIRE same as ASSERT)
    Q_REQUIRE(currentTask != &(taskTable[0]));
    
    currentTask->timeout = ticks;

    timeoutSleepTasks   |=  (1U << (currentTask->priority));       // set task as sleeping
    readyTasks          &= ~(1U << (currentTask->priority));       // set task to not ready

    releaseLock(&taskChangeLock);
    
    __asm("CPSID I"); //disable interrupts
    sched();                                                 // schedule to another task
    __asm("CPSIE I");
}   


//currentTask joins the specified sleep channel
void joinSleepChannel(uint8_t channel)
{
    acquireLock(&taskChangeLock); //prevent any other tasks from making concurrent changes to the task table
    
    Q_ASSERT(currentTask != &(taskTable[0])); // don't allow idleTask to enter sleep
    Q_ASSERT(channel < MAX_SLEEP_CHANNELS); //ensure sleep channel is valid
    
    currentTask->sleepChannel = channel;

    releaseLock(&taskChangeLock);  
}




// WORK FROM HERE - THIS IS A CONCERN BECAUSE LEAVING WHILE ANOTHER TASK IS ASLEEP ON THE SAME CHANNEL COULD LEAVE THAT TASK HIGH AND DRY (AT LEAST UNTIL ANOTHER TASK JOINS THE SAME CHANNEL)





//currentTask leaves its current sleep channel, if any
void leaveSleepChannel()
{
    acquireLock(&taskChangeLock); //prevent any other tasks from making concurrent changes to the task table
    currentTask->sleepChannel = MAX_SLEEP_CHANNELS; //flags no sleep channel
    //currentTask can't be sleeping if it has entered this function so it is unneccessary to unset a bit in channelSleepTasks
    releaseLock(&taskChangeLock);  
}

//currentTask sleeps on its current sleep channel, if any,
//waking the highest priority task on this channel, if any other than currentTask
void sleepOnChannel()
{
    acquireLock(&taskChangeLock); //prevent any other tasks from making concurrent changes to the task table
    
    uint8_t channel = currentTask->sleepChannel;
    if(channel >= MAX_SLEEP_CHANNELS)
    {
      releaseLock(&taskChangeLock);
      return; //no effect because currentTask is not a member of any sleep channel
    }
    
    channelSleepTasks   |=  (1U << (currentTask->priority));       // set task as sleeping
    readyTasks          &= ~(1U << (currentTask->priority));       // set task to not ready
    

    releaseLock(&taskChangeLock);  
    
    __asm("CPSID I"); //disable interrupts
    sched();                                                 // schedule to another task
    __asm("CPSIE I");
}

*/


// marks the caller as dormant and enters the scheduler, never to return to the caller
// this would cause problems if it were ever somehow called on the idle task
void exitTask()
{
  acquireLock(&dormantStackLock); // prevent concurrent changes to dormantStack
  acquireLock(&runningLock); // prevent concurrent changes to RUNNING task
  
  dormantStack[dormantCount] = currentTask; // return RUNNING task to dormant stack
  ++dormantCount;
  currentTask = (struct Task*)(0U); // mark as no RUNNING task
  
  releaseLock(&runningLock);
  releaseLock(&dormantStackLock);
  
  sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)  
  
  // here we should be diverted to PendSV handler never to return
  while(1); // just in case sched doesn't work on this first attempt
}


// updates nextTaskIdx and nextTask
// interrupts must be disabled when this function is called
void sched()
{

  __asm("CPSID I"); // disable interrupts
  
  // try to aquire the locks and if they are held elsewhere, abort
  if (tryAquireLock(&readyTasksLock) == 1U) // aquire lock failed
  {
    __asm("CPSIE I"); //enable interrupts) 
    return;
  }
  if (tryAquireLock(&runningLock) == 1U) // aquire lock failed
  {
    
    releaseLock(&readyTasksLock);
    __asm("CPSIE I"); // enable interrupts) 
    return;
  }
  
  // readyHeapLock and runningLock are now aquired   

  // at least one task other than currentTask is ready
  if (readyCount > 0)
  {

    if (currentTask != (struct Task*)(0U)) // currentTask is not NULL
      addTaskToReadyTasks(currentTask); // put currentTask back into readyTasks
    nextTask = getNextReadyTask(); // pop the next-in-line highest priority task from readyTasks to set nextTask


    // comparing currentTask and nextTask (two volatile variables) is leads to
    // unpredictable access order *Warning[Pa082]* see:
    // https://www.iar.com/knowledge/support/technical-notes/compiler/warningpa082-undefined-behavior-the-order-of-volatile-accesses-is-undefined-in-this-statement/
    struct Task const *next = nextTask;    
    if (next != currentTask)
    {
      //NVIC_INT_CTRL_R |= 0x10000000U; //trigger PendSV exception via interrupt control and state register in system control block
      *(uint32_t volatile *)0xE000ED04 = (1U << 28U);
    }
  }

  releaseLock(&runningLock);
  releaseLock(&readyTasksLock);
  __asm("CPSIE I"); //enable interrupts)   
  
}


void Q_onAssert(char const *module, int loc)
{
  /* TBD: damage control */
  (void)module; /* avoid the "unused parameter" compiler warning */
  (void)loc;    /* avoid the "unused parameter" compiler warning */
  
  // inlined function copied from core_cm4.h (line 1790)
  // because using #include core_cm4.h was causing all
  // kinds of havoc
  NVIC_SystemReset();
}