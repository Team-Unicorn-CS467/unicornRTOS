#include "locks.h"


void initLock(MutexLock* target)
{
  target->held = 0U;
}

void releaseLock(MutexLock* target)
{
  //a simple store (without LDREX, STREX) is acceptable
  //to release the lock if we can assume that this
  //function is only called by the task currently holding the lock
  target->held = 0U;
}

//assumes initLock() has previously been called
void acquireLock(MutexLock* target)
{  
  __asm("MOV R1, #1                           \n" //load value of 1U into R1
        "AQUIRELOCK_CHECK_AVAILABILITY:       \n"
        "LDREX R2, [R0, #0]                   \n" //load value stored at target->held (address stored in R0) into R2
        "CBZ R2, AQUIRELOCK_AVAILABLE         \n"
        "B AQUIRELOCK_CHECK_AVAILABILITY      \n" //loop back if R2 is not 0 (the lock is not avaialable)
        "AQUIRELOCK_AVAILABLE:                \n"          
        "STREX R2, R1, [R0, #0]               \n" //conditional store of value in R1 to target->held (address stored in R0), R2 gets 0 on success, 1 on failure
        "CBZ R2, AQUIRELOCK_AQUIRED           \n"
        "B AQUIRELOCK_CHECK_AVAILABILITY      \n" //loop back if R2 is not 0
        "AQUIRELOCK_AQUIRED:                  \n");
}

//assumes initLock() has previously been called
//returns 0 (LOCK_AQUIRED) if the lock was available/aquired, 1 (LOCK_UNAVAILABLE) otherwise
uint32_t tryAquireLock(MutexLock* target)
{
  __asm("MOV R1, #1                             \n" //load value of 1U into R1
        "LDREX R2, [R0, #0]                     \n" //load value stored at target->held (address stored in R0) into R2
        "CBZ R2, TRYAQUIRELOCK_AVAILABLE        \n"
        "B TRYAQUIRELOCK_RETURN                 \n" //proceed to return (the lock is not avaialable)
        "TRYAQUIRELOCK_AVAILABLE:               \n"          
        "STREX R2, R1, [R0, #0]                 \n" //conditional store of value in R1 to target->held (address stored in R0), R2 gets 0 on success, 1 on failure
        "TRYAQUIRELOCK_RETURN:                  \n"
        "MOV R0, R2                             \n" //R0 gets 0 if exclusive store succeeded, 1 if it failed
        "BX LR                                  \n"
        "MOV R0, R0                             \n"); //return
  return 1; //this statement should never be reached, but just included to make the compiler happy
}