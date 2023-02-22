#include "unicorn.h"
#include "qassert.h" // Q_ASSERT
#include "TM4C123GH6PM.h" // NVIC_SystemReset()
#include "locks.h"


Q_DEFINE_THIS_FILE   // required for using Q_ASSERT


#define LOC_RUNNING     0U      // currentTask can return to readyTasks
#define LOC_DORMANT     1U      // currentTask is in dormantStack
#define LOC_TICKSLEEP   2U      // currentTask is in tickSleepHeap
#define LOC_LOCKSLEEP   3U      // currentTask in one of lockChannels


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

// data structure to index into different priority levels
TaskList readyTasks[PRIORITY_COUNT];            // array of linked lists of READY tasks by priority level
uint32_t readyCount;                            //the total number of tasks ready to run in readyTasks
uint32_t readyMask;                             //bit mask to mark any priority level(s) with currently ready task(s)
MutexLock readyTasksLock;


// assumes readyTasksLock is held when called
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
    ++readyCount;
}

// assumes readyTasksLock is held when called
// pops/returns the next ready task in line at top priority
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
  --readyCount;
  return returnTask;
}


/** peforms initial Task setup and marks new task as ready
*INPUTS: 
*  EntryFunction taskFunc :  function pointer for function the task uses
*  uint8_t priority       :  the scheduling priority of the Task (higher=more priority)
*/
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
  
  // add the task to readyTasks structure
  acquireLock(&readyTasksLock); //prevent concurrent changes to readyTasks
  addTaskToReadyTasks(tsk);
  releaseLock(&readyTasksLock);
}


// performs same tasks as readyNewTask but also calls sched()
void startNewTask(EntryFunction taskFunc, uint8_t priority)
{
  readyNewTask(taskFunc, priority);
  __asm("CPSID I"); // disable interrupts heading into sched()
  sched();
}


/*** TICK_SLEEP STATE ***/

struct Task* tickSleepHeap[MAX_TASKS + 1];
uint32_t tickSleepCount;
uint32_t elapsed;
MutexLock tickSleepLock;


// assumes taskToAdd != NULL
// assumes tickSleepCount <= MAX_TASKS before call
// assumes tickSleepLock is held at the time of calling
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
// reduces tickSleepCount by 1
// assumes that tickSleepCount > 0 at the time of calling
// assumes tickSleepLock is held at the time of calling
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


/*** LOCK_SLEEP STATE ***/

// data structure to index into different lock sleep groups
LockChannel lockChannels[MAX_LOCKS];            // OS-managed semaphore locks w/ associated lists of sleeping tasks
MutexLock lockSleepLock;


// assumes lockSleepLock is held when called
// assumes chan < MAX_LOCKS
// pops/returns the next-in-line task sleeping on chan, if any
// returns NULL if there are no tasks sleeping on chan
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


/*** RUNNING STATE ***/

struct Task* volatile currentTask;     // must be here because it gets labelled for easier access in assembly code
struct Task* volatile nextTask;        // must be here because it gets labelled for easier access in assembly code
uint32_t currentTaskLoc;               // encodes the location of currentTask heading into the next sched() call


// marks the caller as dormant and enters the scheduler, never to return to the caller
// this would cause problems if it were ever somehow called on the idle task
void exitTask()
{
  acquireLock(&dormantStackLock); // prevent concurrent changes to dormantStack

  __asm("CPSID I"); // disable interrupts to avoid any race conditions on currentTask
  
  dormantStack[dormantCount] = currentTask; // add currentTask to dormant stack
  ++dormantCount;
  currentTaskLoc = LOC_DORMANT;
  
  releaseLock(&dormantStackLock);
  
  sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)  
  
  // here we should be diverted to PendSV handler never to return
  while(1); // just in case sched doesn't work on this first attempt
}


// put currentTask in blocking unready state for 'ticks' number of SysTick cycles
void timeoutSleep(uint32_t ticks)
{   
  acquireLock(&tickSleepLock);

  __asm("CPSID I"); // disable interrupts to avoid any race conditions on currentTask
  
  // decrement timeouts of any currently sleeping tasks by elapsed
  // which represents the number of ticks since a task was last
  // added to or removed from tickSleepHeap
  for (uint32_t i = 0; i < tickSleepCount; ++i)
  {
    if (tickSleepHeap[i]->timeout >= elapsed)
      tickSleepHeap[i]->timeout -= elapsed;
    else
      tickSleepHeap[i]->timeout = 0U;
  }
  currentTask->timeout = ticks;
  elapsed = 0U; // reset elapsed value now that all timeouts are up to date
  
  addTaskToTickSleepHeap(currentTask);
  currentTaskLoc = LOC_TICKSLEEP;
  
  releaseLock(&tickSleepLock);
  
  sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)  
}   


// increment elapsed and compare to timeout of task (if any) at top of tickSleepHeap
void decrementTimeouts(void)
{
  acquireLock(&tickSleepLock);
  if (tickSleepCount != 0) // 1 or more sleeping tasks
  {
    ++elapsed;
    while (elapsed >= tickSleepHeap[0]->timeout)
    {
      struct Task* wokeTask = popTaskFromTickSleepHeap();
      
      acquireLock(&readyTasksLock);
      addTaskToReadyTasks(wokeTask);
      releaseLock(&readyTasksLock);
      
      if (tickSleepCount == 0)
        break;
    }
  }
  releaseLock(&tickSleepLock);
  __asm("CPSID I"); // disable interrupts heading into sched()
  sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)
}


// currentTask calls this to try and acquire a lock,
// sleeps the task if it cannot be immediately acquired
// and reschedules (when the task is later awoken, the
// implementation of releaseUnicornSemaphore() should
// guarantee that this task acquires the lock on chan in due turn)
void aquireUnicornSemaphore(uint32_t chan)
{
  Q_ASSERT(chan < MAX_LOCKS); 

  acquireLock(&lockSleepLock);
  
  if (tryAquireLock(&(lockChannels[chan].lock)) == LOCK_AQUIRED)
  {
    releaseLock(&lockSleepLock);
    return; // lock acquired and currentTask can continue on with its business
  }
  
  __asm("CPSID I"); // disable interrupts to avoid any race conditions on currentTask
  
  addTaskToSleepChannel(currentTask, chan);
  currentTaskLoc = LOC_LOCKSLEEP;
  
  releaseLock(&lockSleepLock);
  
  sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)    
}

// currentTask calls this to wake and transfer holding of the
// lock on chan to the next-in-line task sleeping on chan
// immediately reschedules if there are any tasks leeping on chan
// and holding of the lock on chan is transferred in this way
// if there are no tasks sleeping on chan, then simply releases the lock on chan
// assumes currentTask is the current holder of the lock on chan
void releaseUnicornSemaphore(uint32_t chan)
{
  Q_ASSERT(chan < MAX_LOCKS); 

  acquireLock(&lockSleepLock);
  
  struct Task* wokeTask = getNextTaskSleepingOnChannel(chan);
  if(wokeTask != (struct Task*)(0U)) // a task was sleeping on chan
  {  
    // we don't even release the lock on chan, we just keep it held and
    // make the next-in-line wokeTask available to be scheduled
    releaseLock(&lockSleepLock);

    acquireLock(&readyTasksLock);
    addTaskToReadyTasks(wokeTask); // mark wokeTask as ready
    releaseLock(&readyTasksLock);

    __asm("CPSID I"); // disable interrupts to avoid any race conditions on currentTask  
    sched(); // schedule next task and set PendSV to trigger (as soon as interrupts are enabled)   
  }
  
  else // no tasks were sleeping on chan
  {
    releaseLock(&(lockChannels[chan].lock)); // release the lock on chan  
    releaseLock(&lockSleepLock);     
  }
}


// busy work for the idle task to perform
void onIdle()
{
  for(;;);
}


// starting setup of everything the scheduler needs to run
// assumes interrupts are disabled for the durration of this function
void initializeScheduler(EntryFunction userTasksLoader, uint8_t loaderPriority)
{
  
  // initialize locks
  initLock(&dormantStackLock);
  initLock(&readyTasksLock);
  initLock(&tickSleepLock);
  
  // initialize DORMANT structure
  acquireLock(&dormantStackLock);
  for(uint32_t i = 0; i <=  MAX_TASKS; ++i)
    dormantStack[i] = &(tasks[i]);
  dormantCount = MAX_TASKS + 1;
  releaseLock(&dormantStackLock);

  // initialize READY structure
  acquireLock(&readyTasksLock);
  for(int i = 0; i < PRIORITY_COUNT; ++i)
  {
    readyTasks[i].head = (struct Task*)(0U);
    readyTasks[i].tail = (struct Task*)(0U);
  }
  readyCount = 0U;
  readyMask = 0U;
  releaseLock(&readyTasksLock);

  // initialize TICK_SLEEP structure
  acquireLock(&tickSleepLock);
  tickSleepCount = 0U;
  releaseLock(&tickSleepLock);

  // initialized LOCK_SLEEP structure
  acquireLock(&lockSleepLock);
  for(int i = 0; i < MAX_LOCKS; ++i)
    initLock(&(lockChannels[i].lock));  
  releaseLock(&lockSleepLock);
  
  // initialize RUNNING structure
  
  // artificially mark an uninitialized dormant task as currentTask
  // (we can't use an initialized task because the first call to sched()
  // will push registers representing the program state from main();
  // since we don't want to return there, we here set up the starting
  // state as though we had called exitTask() from main such that
  // currentTask's memory, &(tasks[0]), has just been recaptured into dormantStack;
  // this only will work if we have READY task(s) cueued up before interrupts are enabled
  currentTask = &(tasks[0]);
  currentTaskLoc = LOC_DORMANT;

  // ready the idle task at the lowest priority
  readyNewTask(onIdle, 0);
  
  // start the userTaskLoader, entering the scheduler then enabling interrupts
  startNewTask(userTasksLoader, loaderPriority);
  
}


// updates currentTask and sets things up for a context shift
// interrupts must be disabled when this function is called
void sched()
{
  
  // try to aquire the locks and if they are held elsewhere, abort
  if (tryAquireLock(&readyTasksLock) == LOCK_UNAVAILABLE) // aquire lock failed
  {
    __asm("CPSIE I"); //enable interrupts) 
    return;
  }
  
  // readyHeapLock is now aquired   

  if (currentTaskLoc == LOC_RUNNING) //currentTask is runnable (can be returned to readyTasks
  {
    if (readyCount > 0)
    {
      addTaskToReadyTasks(currentTask); // put currentTask back into readyTasks
      nextTask = getNextReadyTask(); // pop the next-in-line highest priority task from readyTasks to set nextTask
      
      //NVIC_INT_CTRL_R |= 0x10000000U; //trigger PendSV exception via interrupt control and state register in system control block
      *(uint32_t volatile *)0xE000ED04 = (1U << 28U);
    }
    // else - currentTask will remain unchanged and no context switch will trigger
  }
  else //currentTaskLoc == LOC_TICKSLEEP || currentTaskLoc == LOC_DORMANT
  {
    // assume readyTaskCount > 0 since idle task should never sleep or exit
    nextTask = getNextReadyTask(); // pop the next-in-line highest priority task from readyTasks to set nextTask
    
    //NVIC_INT_CTRL_R |= 0x10000000U; //trigger PendSV exception via interrupt control and state register in system control block
    *(uint32_t volatile *)0xE000ED04 = (1U << 28U);
    
    currentTaskLoc = LOC_RUNNING;
  }
  
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