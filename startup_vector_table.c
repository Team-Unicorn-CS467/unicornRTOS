#include "ticks.h" //for incrementTicks()
#include "exception_handlers.h"
#include "lm4f120h5qr.h"
#include "unicorn.h"

//used to substitute instead of the IAR generic definition of the interrupt vector table

extern int CSTACK$$Limit; //just introducing the symbol to the compiler know this value will be pulled in later (during linking???)
void __iar_program_start(void); //again just introducing the function which will be pulled in later by the compiler

void handler_NMI(void) //denial of service for now
{
  while(1) {}
}

void handler_HardFault(void) //denial of service for now
{
  while(1) {}
}

void handler_MemoryManagementFault(void) //denial of service for now
{
  while(1) {}
}

void handler_BusFault(void) //denial of service for now
{
  while(1) {}
}

void handler_UsageFault(void) //denial of service for now
{
  while(1) {}
}

void handler_SVCall(void) //no implementation for now
{
  while(1) {}
}

//***NOTE: THE CURRENT IMPLEMENTATION REQUIRES THE FPU TO BE TURNED OFF***
void handler_PendSV(void) //no implementation for now
{ 
  __asm("CPSID I"); //disable interrupts
  
  //currentTask and nextTask are not the same, switch context
  if (currentTask != nextTask)
  { 
    //save registers R4 through R11 to currentTask's stack
    __asm("PUSH {r4-r11}");
    
    //save the current sp register value to currentTask->sp
    __asm("LDR r1, =currentTask"); //load address of currentTask into r1
    __asm("LDR r1, [r1]"); //load (pointer) value stored at currentTask into r1
    __asm("STR sp, [r1, #0x00]"); //save current value of sp register to currentTask->sp (sp at offset 0x00 in Task struct)

    //store nextTask->sp into sp register
    __asm("LDR r1, =nextTask"); //load address of nextTask into r1
    __asm("LDR r1, [r1]"); //load (pointer) value stored at nextTask into r1
    __asm("LDR r1, [r1, #0x00]"); //load nextTask->sp value into r1
    __asm("MOV sp, r1"); //copy nextTask->sp value into sp register
    
    //set currentTask = nextTask
    __asm("LDR r1, =nextTask"); //load address of nextTask into r1
    __asm("LDR r1, [r1]"); //load (pointer) value stored at nextTask into r1
    __asm("LDR r2, =currentTask"); //load address of currentTask into r2
    __asm("STR r1, [r2]"); //store (pointer) value of nextTask at currentTask address
    
    //save registers R4 through R11 to the stack of nextTask (now also currentTask)
    __asm("POP {r4-r11}");
  }
  
  __asm("CPSIE I"); //enable interrupts)
}

void handler_SysTick(void) //incrementing ticks, scheduling/switching task
{  
  __asm("CPSID I"); //disable interrupts
  incrementTicks();
  sched(); //schedule next task
  __asm("CPSIE I"); //enable interrupts)
  
  NVIC_INT_CTRL_R |= 0x10000000U; //trigger PendSV exception via interrupt control and state register in system control block
}

int const __vector_table[] @ ".intvec" = //the @ ".intvec" syntax is not standard c, but IAR supports it for replacing the generic vector table IAR generates in the linking process from its own library???
{
  (int)&CSTACK$$Limit, //starting stack pointer
  (int)&__iar_program_start, //reset - initial value copied to the pc register when the microcontroller comes out of reset (the address in ROM where the ARM processor starts executing code)

  //system exception handlers (all with negative IRQ numbers)
  (int)&handler_NMI,
  (int)&handler_HardFault,
  (int)&handler_MemoryManagementFault,
  (int)&handler_BusFault,
  (int)&handler_UsageFault,
  0, //reserved
  0, //reserved
  0, //reserved
  0, //reserved
  (int)&handler_SVCall,
  0, //reserved for debug
  0, //reserved
  (int)&handler_PendSV,
  (int)&handler_SysTick
    
  //handlers with positive IRQ numbers below ...
    
};