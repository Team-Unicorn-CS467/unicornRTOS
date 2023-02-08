#include "locks.h"

uint32_t TaskTableLock;
uint32_t* TaskTableLockAddress; //must be here because it gets labelled for easier access in assembly code

void initTaskTableLock()
{
  TaskTableLockAddress = &TaskTableLock;
}

//assumes initTaskTableLock() has previously been called
void releaseTaskTableLock()
{
  //save registers R0, R1, R2 to stack
  __asm("PUSH {r0-r2}");
  
  __asm("MOV R1, #0"); //load value of 0U into R1
  __asm("LDR R0, =TaskTableLockAddress"); //load address of TaskTableLock into r1
  
  //conditional store of value in R1 to address of R0, R2 gets 0 on succes, 1 on failure
  __asm label("TRY_RELEASE:");
  __asm("STREX R2, R1, [R0]");
  __asm("CBNZ R2, TRY_RELEASE"); //loop back if R2 is not 0
  
  //restore registers R0, R1, R2
  __asm("POP {r0-r2}");
}

void acquireTaskTableLock()
{
  *TaskTableLockAddress = 1U; //change this later
}