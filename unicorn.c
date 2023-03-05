#include "unicorn.h"

#define OS_MAX_TASKS MAX_TASKS + 1 // one additional space for the idle task, must be <= 32

///////////////////////////////////////////////////////////////////
//
// MISCELANEOUS STUFF
//
///////////////////////////////////////////////////////////////////

Q_DEFINE_THIS_FILE   // required for using Q_ASSERT

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

// busy work for the idle task to perform
void onIdle()
{
  for(;;);
}


///////////////////////////////////////////////////////////////////
// TASK STATES
//
//      task structures below are divided by state; each state
//      appears in all caps; the states are conceptual, not
//      implemented in a very monolithic way, but each has
//      associated structure(s)
//
//      the general paradigm of accessing/changing task state
//      and associated structures is that the tasks themselves
//      perform the associated work during their own runtime
//      and avoid race conditions through the use of mutex locks
//      on each structure.
//
//      there are currently two important exceptions to this
//      general paradigm: (1) when a DORMANT task is to be made
//      READY, tasks will interact with an intermediate
//      staging structure, so that sched() and the SysTick
//      handler (which cannot interrupt each other or effectively
//      be interrupted) will certainly have unrestricted access
//      to the main READY structure; (2) when the RUNNING task
//      is to enter TICK_SLEEP, it interacts with a similar
//      intermediate staging structure, so that the SysTick
//      handler will certainly have unrestricted access to the
//      main TICK_SLEEP structure
//
///////////////////////////////////////////////////////////////////

struct Task tasks[OS_MAX_TASKS]; // table used for storing task resources; not direct access


///////////////////////////////////////////////////////////////////
// DORMANT STATE
//
//      available task memory, not yet initialized to run or
//      recycled from a previous task which exited
//
///////////////////////////////////////////////////////////////////

struct Task* dormantStack[OS_MAX_TASKS];
uint32_t dormantCount;
MutexLock dormantStackLock;


///////////////////////////////////////////////////////////////////
// READY STATE
//
//      data structure to index into tasks ready by priority level
//      and then by turn order
//
///////////////////////////////////////////////////////////////////

// structures which should be accessed/modified only by sched() or the SysTick handler
TaskList readyTasks[PRIORITY_COUNT];            // array of linked lists of READY tasks by priority level
uint32_t readyMask;                             // bit mask to mark any priority level(s) with currently ready task(s)

// clycling array which can be accessed/modified in interruptable code (code executing during task runtime)
// it is assumed that interruptable code will only add tasks to this arry, not remove them or modify the array in any other way
struct Task* stagedReadyTasks[OS_MAX_TASKS];    // array of tasks ready to be added to readyTasks
uint32_t stagedReadyTasksCount;                 // number of tasks in stagedReadyTasks
MutexLock stagedReadyTasksLock;

// assumes uninterruptable and only called by sched() or the SysTick handler
// assumes taskToAdd != NULL
// assumes taskToAdd->priority is within range
void addTaskToReadyTasks(struct Task* taskToAdd)
{
    uint32_t taskP = taskToAdd->priority;

    // one or more tasks currently ready at this priority level
    if (readyMask & (1U << taskP))
      readyTasks[taskP].tail->next = taskToAdd;
    
    else // no tasks currently ready at this priority level
    {
      readyMask |= (1U << taskP);
      readyTasks[taskP].head = taskToAdd;
    }

    readyTasks[taskP].tail = taskToAdd;
    taskToAdd->next = (struct Task*)(0U);
}

// pops/returns the next ready task in line at top priority
// assumes uninterruptable and only called by sched() or the SysTick handler
// assumes at least one task is ready (e.g. idle task) when called
struct Task* getNextReadyTask()
{
  uint32_t topPriority = getTopSetBitIndex(readyMask);
  struct Task* returnTask = readyTasks[topPriority].head;
  readyTasks[topPriority].head = returnTask->next;
  if (readyTasks[topPriority].head == (struct Task*)(0U)) // now empty at this priority level
  {
    readyTasks[topPriority].tail = (struct Task*)(0U);
    readyMask &= ~(1U << topPriority);
  }
  return returnTask;
}


///////////////////////////////////////////////////////////////////
// TICK_SLEEP STATE
//
//      min heap structure to store/track tasks sleeping
//      for a set number of system ticks
//
///////////////////////////////////////////////////////////////////

// structures which should be accessed/modified only by the SysTick handler
struct Task* tickSleepHeap[OS_MAX_TASKS];
uint32_t tickSleepCount;
uint32_t elapsed;

// clycling array which can be accessed/modified in interruptable code (code executing during task runtime)
// it is assumed that interruptable code will only add tasks to this arry, not remove them or modify the array in any other way
struct Task* stagedTickSleepTasks[OS_MAX_TASKS];        // array of tasks ready to be added to tickSleepHeap
uint32_t stagedTickSleepTasksCount;                     // number of elements in stagedTickSleepTasks
MutexLock stagedTickSleepTasksLock;

// assumes uninterruptable and only called by the SysTick handler
// assumes taskToAdd != NULL
// assumes tickSleepCount < OS_MAX_TASKS before call
// assumes taskToAdd->timeout is already set
void addTaskToTickSleepHeap(struct Task* taskToAdd)
{  
  uint32_t i = tickSleepCount;
  
  // percolate up in the heap
  while(i > 0)
  {
    uint32_t j = (i - 1) / 2; //index of parent node in the heap
    if (tickSleepHeap[j]->timeout <= taskToAdd->timeout)
      break; // i is correct heap position for taskToAdd
    
    tickSleepHeap[i] = tickSleepHeap[j]; // move the parent down
    i = j; // the parent index is the new child index
  }
  tickSleepHeap[i] = taskToAdd;
  ++tickSleepCount;
}

// returns the task with min timeout, popping it off tickSleepHeap
// and decrementing tickSleepCount by 1
// assumes uninterruptable and only called by the SysTick handler
// assumes that tickSleepCount > 0 at the time of calling
struct Task* popTaskFromTickSleepHeap()
{
  --tickSleepCount;
  struct Task* highTimeoutTask = tickSleepHeap[tickSleepCount];
  struct Task* minTimeoutTask = tickSleepHeap[0U];
  
  // find good new position for highTimeoutTask
  uint32_t i = 0U;  
  uint32_t j = 1U; // left child
  uint32_t k = 2U; //right child
  while (j < tickSleepCount)
  {
    if (k < tickSleepCount)
    {
      if (tickSleepHeap[k]->timeout < tickSleepHeap[j]->timeout)
        j = k;
    }
    
    // j now the index of the lower timeout both child nodes
    
    if (highTimeoutTask->timeout <= tickSleepHeap[j]->timeout)
      break; // i is correct heap position for highTimeoutTask
    
    tickSleepHeap[i] = tickSleepHeap[j]; // shift node up
    i = j; // child index is the new parent index
    j = (i * 2) + 1; // left child
    k = j + 1; // right child      
  }
  tickSleepHeap[i] = highTimeoutTask;
  return minTimeoutTask;
}


///////////////////////////////////////////////////////////////////
// LOCK_SLEEP STATE
//
//      data structure for indexing into a set of OS-managed
//      semaphore locks w/ associated lists of sleeping tasks
//
///////////////////////////////////////////////////////////////////

LockChannel lockChannels[MAX_LOCKS];            // OS-managed semaphore locks w/ associated lists of sleeping tasks
MutexLock lockSleepLock;

// pops/returns the next-in-line task sleeping on chan, if any
// returns NULL if there are no tasks sleeping on chan
// assumes lockSleepLock is held when called
// assumes chan < MAX_LOCKS
struct Task* getNextTaskSleepingOnChannel(uint32_t chan)
{
    struct Task* returnTask = lockChannels[chan].sleepingTasks.head;
    if(returnTask != (struct Task*)(0U)) // task(s) sleeping on chan
    {
      if (returnTask->next == (struct Task*)(0U)) // only one task sleeping on chan
        lockChannels[chan].sleepingTasks.tail = returnTask->next; // sets to NULL
      lockChannels[chan].sleepingTasks.head = returnTask->next;
    }
    return returnTask;
}

// assumes lockSleepLock is held when called
// assumes taskToSleep is not NULL
// assumes chan < MAX_LOCKS
void addTaskToSleepChannel(struct Task* taskToSleep, uint32_t chan)
{
  struct Task* chanTail = lockChannels[chan].sleepingTasks.tail;
  
  if (chanTail != (struct Task*)(0U)) // task(s) sleeping on chan
    chanTail->next = taskToSleep;
  
  else // no task sleeping on chan
    lockChannels[chan].sleepingTasks.head = taskToSleep;

  lockChannels[chan].sleepingTasks.tail = taskToSleep;    
  taskToSleep->next = (struct Task*)(0U);  
}


///////////////////////////////////////////////////////////////////
// RUNNING STATE
//
//      tracks the currently running task and the nextTask
//      heading into a context switch
//
///////////////////////////////////////////////////////////////////

typedef enum 
{
  RUNNABLE = 0U,                        // denotes currentTask is still eligible to run heading into sched
  UNRUNNABLE                            // denotes currentTask cannot be run again heading into sched (e.g. due to exit or sleep)
}RUNNING_STATE;

struct Task* volatile currentTask;      // must be here because it gets labelled for easier access in assembly code
RUNNING_STATE runningState;             // flags whether currentTask is eligible to run again or not at the time of heading into sched()
struct Task* volatile nextTask;         // must be here because it gets labelled for easier access in assembly code
volatile uint32_t blockSched;                    // used to ensure we don't reenter sched from an interrupt hander (e.g. SysTick) with higher priority
                                            //than PendSV after a sched() call

// updates currentTask and sets things up for a context shift
// interrupts must be disabled when this function is called
void sched()
{

  // FOR TESTING ONLY
  BSP_setGPIO(GPIOB_AHB, GPIO_PB3, HIGH);
  // FOR TESTING ONLY

  // one or more tasks are staged to be added to readyTasks and stagedReadyTasks is not locked
  if (stagedReadyTasksCount > 0U && tryAquireLock(&stagedReadyTasksLock) == LOCK_AQUIRED)
  {
    // add staged tasks to readyTasks
    for (uint32_t i = 0; i < stagedReadyTasksCount; ++i)
      addTaskToReadyTasks(stagedReadyTasks[i]);
    stagedReadyTasksCount = 0U;
    releaseLock(&stagedReadyTasksLock);      
  }
  // else - there are no staged tasks or the last staged task is is in the process of being added
  //            by a task which was interrupted so we just wait for the next pass after that task
  //            has completed its business
  
  // schedule the next task
  if (runningState == RUNNABLE) // currentTask can be returned to readyTasks
  {
    addTaskToReadyTasks(currentTask); // put currentTask back into readyTasks
    struct Task* nxtTsk = getNextReadyTask(); // pop the next-in-line highest priority task from readyTasks to set nextTask
    if (nxtTsk != currentTask) // a new task is ready, trigger a context switch
    {
      nextTask = nxtTsk;
      
      // NVIC_INT_CTRL_R |= 0x10000000U; //trigger PendSV exception via interrupt control and state register in system control block
      *(uint32_t volatile *)0xE000ED04 = (1U << 28U);

      blockSched = 1; // block sched() from being called from an interrupt handler until after PendSV has occurred
    }
  }
  else // runningState == UNRUNNABLE, e.g. because currentTask is sleeping or exiting
  {
    // assume one or more tasks are ready since idle task should never sleep or exit
    nextTask = getNextReadyTask(); // pop the next-in-line highest priority task from readyTasks to set nextTask
    
    // NVIC_INT_CTRL_R |= 0x10000000U; //trigger PendSV exception via interrupt control and state register in system control block
    *(uint32_t volatile *)0xE000ED04 = (1U << 28U);
    
    runningState = RUNNABLE;
    blockSched = 1; // block sched() from being called from an interrupt handler until after PendSV has occurred
  }
  
  // FOR TESTING ONLY
  BSP_setGPIO(GPIOB_AHB, GPIO_PB3, LOW);
  // FOR TESTING ONLY
  
  __asm("CPSIE I"); // enable interrupts)   
  
}


///////////////////////////////////////////////////////////////////
//
// PUBLIC API FUNCTIONS
//
///////////////////////////////////////////////////////////////////

// starting setup of everything the scheduler needs to run
// ends with the first sched() call and results in interrupts being enabled
// assumes interrupts are disabled at the time this function is called
void initializeScheduler(EntryFunction userTasksLoader, uint8_t loaderPriority)
{
  
  // initialize DORMANT structure
  initLock(&dormantStackLock);
  for(uint32_t i = 0; i < OS_MAX_TASKS; ++i)
    dormantStack[i] = &(tasks[i]);
  dormantCount = OS_MAX_TASKS;

  // initialize TICK_SLEEP structure
  tickSleepCount = 0U;
  initLock(&stagedTickSleepTasksLock);
  stagedTickSleepTasksCount = 0U;

  // initialize LOCK_SLEEP structure
  initLock(&lockSleepLock);
  for(int i = 0; i < MAX_LOCKS; ++i)
  {
    initLock(&(lockChannels[i].lock));
    lockChannels[i].sleepingTasks.head = (struct Task*)(0U);
    lockChannels[i].sleepingTasks.tail = (struct Task*)(0U);
  }
  
  // initialize READY structure
  for(int i = 0; i < PRIORITY_COUNT; ++i)
  {
    readyTasks[i].head = (struct Task*)(0U);
    readyTasks[i].tail = (struct Task*)(0U);
  }
  readyMask = 0U;
  initLock(&stagedReadyTasksLock);
  stagedReadyTasksCount = 0U;
  
  // initialize RUNNING structure
  
  // artificially mark an uninitialized dormant task as currentTask
  // (we can't use an initialized task because the first call to sched()
  // will push registers representing the program state from main();
  // since we don't want to return there, we here set up the starting
  // state as though we had called exitTask() from main such that
  // currentTask's memory, &(tasks[0]), has just been recaptured into dormantStack;
  // this only will work if we have READY task(s) cueued up before interrupts are enabled
  currentTask = &(tasks[0]);
  runningState = UNRUNNABLE;
  blockSched = 0U;

  // ready the idle task at the lowest priority
  readyNewTask(onIdle, 0);
  
  // start the userTaskLoader, entering the scheduler then enabling interrupts
  startNewTask(userTasksLoader, loaderPriority);
  
}

//peforms initial Task setup and marks new task as ready without actually scheduling
void readyNewTask(EntryFunction taskFunc, uint8_t priority)
{  
  // sanity check
  Q_ASSERT(priority < PRIORITY_COUNT);  

  // aqcuire a dormant task
  acquireLock(&dormantStackLock); // prevent concurrent changes to dormantStack
  Q_ASSERT(dormantCount > 0);
  --dormantCount;
  struct Task* tsk = dormantStack[dormantCount];
  releaseLock(&dormantStackLock);
  
  // initial task stack setup
  uint32_t stackEnd   = (uint32_t)(tsk->stack) + (TASK_STACK_WORD_SIZE * BYTES_PER_WORD); //later the stack size will be parameterized
  uint32_t remainder  = stackEnd % 8U;
  stackEnd -= remainder; //Cortex-M stack must be alligned at 8-byte boundary
  tsk->sp = (uint32_t*)(stackEnd - sizeof(ContextFrame));
  
  // starting context frame
  ContextFrame* firstFrame = (ContextFrame*)(tsk->sp);
  firstFrame->r4  = 0x0000000DU;                // throw-away value, easy to spont in memory
  firstFrame->r5  = 0x0000000CU;                // throw-away value, easy to spont in memory
  firstFrame->r6  = 0x0000000BU;                // throw-away value, easy to spont in memory
  firstFrame->r7  = 0x0000000AU;                // throw-away value, easy to spont in memory
  firstFrame->r8  = 0x00000009U;                // throw-away value, easy to spont in memory
  firstFrame->r9  = 0x00000008U;                // throw-away value, easy to spont in memory
  firstFrame->r10 = 0x00000007U;                // throw-away value, easy to spont in memory
  firstFrame->r11 = 0x00000006U;                // throw-away value, easy to spont in memory
  firstFrame->r0  = 0x00000005U;                // throw-away value, easy to spont in memory
  firstFrame->r1  = 0x00000004U;                // throw-away value, easy to spont in memory 
  firstFrame->r2  = 0x00000003U;                // throw-away value, easy to spont in memory 
  firstFrame->r3  = 0x00000002U;                // throw-away value, easy to spont in memory
  firstFrame->r12 = 0x00000001U;                // throw-away value, easy to spont in memory
  firstFrame->lr  = 0x00000000U;                // throw-away value, easy to spont in memory
  firstFrame->pc   = (uint32_t)(taskFunc);      // task function entry point
  firstFrame->xpsr = (1U << 24);                // program status register value for "thumb state"
  
  // tsk->sp now points to the last used word in stack memory

  // initialize other properties
  tsk->priority = priority;
  tsk->timeout = 0U;
  tsk->next = (struct Task*)(0U);
  
  // stage the new task for addition to readyTasks
  acquireLock(&stagedReadyTasksLock);
  
  stagedReadyTasks[stagedReadyTasksCount] = tsk;
  ++stagedReadyTasksCount;

  releaseLock(&stagedReadyTasksLock);
}

// performs same tasks as readyNewTask but also calls sched()
void startNewTask(EntryFunction taskFunc, uint8_t priority)
{
  readyNewTask(taskFunc, priority);
  __asm("CPSID I"); // disable interrupts heading into sched()
  sched();
}

// put currentTask in blocking unready state for 'ticks' number of SysTick cycles
void timeoutSleep(uint32_t ticks)
{   
  currentTask->timeout = ticks;
  
  // stage currentTask task for addition to tickSleepTasks
  acquireLock(&stagedTickSleepTasksLock); 
  
  stagedTickSleepTasks[stagedTickSleepTasksCount] = currentTask;
  ++stagedTickSleepTasksCount ;
  
  // we don't want to be interrupted after changing seting runningState
  // so we disable interrupts early
  __asm("CPSID I");
  
  runningState = UNRUNNABLE;
  
  releaseLock(&stagedTickSleepTasksLock);

  sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)  
}

// increment elapsed and compare to timeout of task (if any) at top of tickSleepHeap
void sysTickRoutine(void)
{

  // FOR TESTING ONLY
  BSP_setGPIO(GPIOF_AHB, GPIO_PF3, HIGH); // second pin
  // FOR TESTING ONLY
  
  // one or more tasks are staged to be added to tickSleepHeap and stagedTickSleepTasks is not locked
  if (stagedTickSleepTasksCount > 0U && tryAquireLock(&stagedTickSleepTasksLock) == LOCK_AQUIRED)
  {
  
    // decrement timeouts of any currently sleeping tasks by elapsed
    // which represents the number of ticks since a task was last
    // added to or removed from tickSleepHeap
    uint32_t i;
    for (i = 0; i < tickSleepCount; ++i)
    {
      if (tickSleepHeap[i]->timeout >= elapsed)
        tickSleepHeap[i]->timeout -= elapsed;
      else
        tickSleepHeap[i]->timeout = 0U;
    }
    elapsed = 0U; // reset elapsed value now that all timeouts are up to date

    // add staged tasks to tickSleepHeap
    for (i = 0; i < stagedTickSleepTasksCount; ++i)
      addTaskToTickSleepHeap(stagedTickSleepTasks[i]); // the timeout value of each of these tasks was previously set
    stagedTickSleepTasksCount = 0U;
    releaseLock(&stagedTickSleepTasksLock);
  
  }
  // else - there are no staged tasks or the last staged task is is in the process of being added
  //            by a task which was interrupted so we just wait for the next pass after that task
  //            has completed its business  
  
  // make any sleeping tasks ready if their timeouts have expired
  if (tickSleepCount != 0) // 1 or more sleeping tasks
  {
    ++elapsed;
    while (elapsed >= tickSleepHeap[0]->timeout)
    {
      struct Task* wokeTask = popTaskFromTickSleepHeap();
      addTaskToReadyTasks(wokeTask); // skip staging because this handler and sched() cannot interrupt one another
      
      if (tickSleepCount == 0)
      {
        elapsed = 0U; // reset elapsed value until more tasks sleep
        break;        
      }
    }
  }

  // FOR TESTING ONLY
  BSP_setGPIO(GPIOF_AHB, GPIO_PF3, LOW); // second pin
  // FOR TESTING ONLY

  if (blockSched == 0U) //sched() is not blocked
  {
    __asm("CPSID I"); // disable interrupts heading into sched()
    sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)    
  }
}

// a task calls this to try and acquire a lock,
// sleeps the task if it cannot be immediately acquired
// and reschedules (when the task is later awoken, the
// implementation of releaseUnicornSemaphore() should
// guarantee that this task acquires the lock on chan in due turn)
void aquireUnicornSemaphore(uint32_t chan)
{
  Q_ASSERT(chan < MAX_LOCKS); 

  acquireLock(&lockSleepLock);
  
  // we want to ensure the tryAquireLock is not failing
  // on account of being interrupted in this case
  // if the aquisition legitimately failes because the lock is held,
  // we leave interrupts disabled to avoid race conditions on currentTask
  __asm("CPSID I");
  
  if (tryAquireLock(&(lockChannels[chan].lock)) == LOCK_AQUIRED)
  {
    releaseLock(&lockSleepLock);
    __asm("CPSIE I"); //enable interrupts) 
    return; // lock acquired and currentTask can continue on with its business
  }  
  
  addTaskToSleepChannel(currentTask, chan);
  
  releaseLock(&lockSleepLock);
  
  runningState = UNRUNNABLE;
  
  sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)    
}

// a task calls this to release a held lock on chan which then wakes
// the next-in-line task which was sleeping on chan (if any)
// immediately reschedules
// assumes the caller (currentTask) is the current holder of the lock on chan
void releaseUnicornSemaphore(uint32_t chan)
{
  Q_ASSERT(chan < MAX_LOCKS); 

  acquireLock(&lockSleepLock);
  
  struct Task* wokeTask = getNextTaskSleepingOnChannel(chan);
  releaseLock(&(lockChannels[chan].lock)); // release the lock on chan
  
  releaseLock(&lockSleepLock);    

  if(wokeTask != (struct Task*)(0U)) // a task was sleeping on chan
  {  
    acquireLock(&stagedReadyTasksLock);
    stagedReadyTasks[stagedReadyTasksCount] = wokeTask; // stage wokeTask to be ready
    ++stagedReadyTasksCount;
    releaseLock(&stagedReadyTasksLock); 
  }

  __asm("CPSID I"); // disable interrupts to avoid any race conditions on currentTask  
  sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)  
}

// marks the caller as dormant and enters the scheduler, never to return to the caller
// this would cause problems if it were ever somehow called on the idle task
void exitTask()
{
  acquireLock(&dormantStackLock); // prevent concurrent changes to dormantStack

  __asm("CPSID I"); // disable interrupts to avoid any race conditions on currentTask
  
  dormantStack[dormantCount] = currentTask; // add currentTask to dormant stack
  ++dormantCount;
  
  releaseLock(&dormantStackLock);

  runningState = UNRUNNABLE;
  
  sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)  
  
  // here we should be diverted to PendSV handler never to return
  while(1); // just in case sched doesn't work on this first attempt
}