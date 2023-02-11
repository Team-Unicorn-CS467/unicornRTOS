#include "locks.h"

void initLock(SpinLock* target)
{
  target->held = 0U;
}

void releaseLock(SpinLock* target)
{
  //a simple store (without LDREX, STREX) is acceptable
  //to release the lock if we can assume that this
  //function is only called by the task currently holding the lock
  target->held = 0U;
}

//assumes initSpinLock() has previously been called
void acquireLock(SpinLock* target)
{  
  __asm("MOV R1, #1                     \n" //load value of 1U into R1
        "CHECK_LOCK_AVAILABILITY:       \n"
        "LDREX R2, [R0, #0]             \n" //load value stored at target->held (address stored in R0) into R2
        "CBZ R2, LOCK_AVAILABLE         \n"
        "B CHECK_LOCK_AVAILABILITY      \n" //loop back if R2 is not 0 (the lock is not avaialbe)
        "LOCK_AVAILABLE:                \n"          
        "STREX R2, R1, [R0, #0]         \n" //conditional store of value in R1 to target->held (address stored in R0), R2 gets 0 on success, 1 on failure
        "CBZ R2, LOCK_AQUIRED           \n"
        "B CHECK_LOCK_AVAILABILITY      \n" //loop back if R2 is not 0
        "LOCK_AQUIRED:                  \n");
}