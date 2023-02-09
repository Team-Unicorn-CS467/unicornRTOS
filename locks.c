#include "locks.h"

uint32_t SpinLock; //must be here because it gets labelled for easier access in assembly code

void initSpinLock()
{
  SpinLock = 0U;
}

//assumes initSpinLock() has previously been called
void releaseSpinLock()
{
  __asm("MOV R1, #0                     \n" //load value of 0U into R1
        "LDR R0, =SpinLock              \n" //load address of SpinLock into R0
        "RETRY_LOCK_RELEASE:            \n"
        "LDREX R2, [R0]                 \n" //load value stored at SpinLock into R2
        "STREX R2, R1, [R0]             \n" //conditional store of value in R1 to address of R0, R2 gets 0 on success, 1 on failure
        "CBZ R2, LOCK_RELEASED          \n"
        "B RETRY_LOCK_RELEASE           \n" //loop back if R2 is not 0
        "LOCK_RELEASED:                 \n");
}

//assumes initSpinLock() has previously been called
void acquireSpinLock()
{  
  __asm("MOV R1, #1                     \n" //load value of 1U into R1
        "LDR R0, =SpinLock              \n" //load address of SpinLock into R0
        "CHECK_LOCK_AVAILABILITY:       \n"
        "LDREX R2, [R0]                 \n" //load value stored at SpinLock into R2
        "CBZ R2, LOCK_AVAILABLE         \n"
        "B CHECK_LOCK_AVAILABILITY      \n" //loop back if R2 is not 0 (the lock is not avaialbe)
        "LOCK_AVAILABLE:                \n"
        "STREX R2, R1, [R0]             \n" //conditional store of value in R1 to address of R0, R2 gets 0 on success, 1 on failure
        "CBZ R2, LOCK_AQUIRED           \n"
        "B CHECK_LOCK_AVAILABILITY      \n" //loop back if R2 is not 0
        "LOCK_AQUIRED:                  \n");
}