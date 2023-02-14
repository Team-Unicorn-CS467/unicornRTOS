//#include "ticks.h" //for incrementTicks()
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
void handler_PendSV(void)
{
  /*
  // if not coming from reset/idleTask (when currenTask=null) save r4-r11 and currentTask->sp to stack
  if (currentTask != (Task *)0)
  {
    //save registers R4 through R11 to currentTask's stack
    currentTask->sp = sp;
  }
  sp = nextTask->sp;
  currentTask = nextTask;
  //enable interrupts:
  __asm("CPSIE I");
  */
  
  //disable interrupts
  __asm("CPSID I\n" 

    "LDR        R2, =currentTask\n"
    "LDR        R0, [R2]\n"
        
   // if (currentTask != (Task *)0);
    "CBZ        R0, PendSV_restore\n"
  
  // save r4-r11 onto stack
    "PUSH       {r4-r11}\n"
  
  // currentTask->sp = sp;
    "STR        SP, [R0]\n" 
    
    "PendSV_restore:\n\n"
  // sp = nextTask->sp;
    "LDR        R1, =nextTask\n"
    "LDR        R0, [R1]\n"
    "LDR        SP,[R0]\n"
  
  // currentTask = nextTask
    "STR        R0, [R2]\n"

  // clear exclusive monitor (for semaphores)
    "CLREX              \n"
      
  // restore nextTask's r4-r11 regs
    "POP        {r4-r11}\n"
    
  // enable interrupts:
    "CPSIE I\n"
  
  // return
    "BX         LR\n"); 
}

void handler_SysTick(void) //incrementing ticks, scheduling/switching task
{  
  __asm("CPSID I"); //disable interrupts
  decrementTimeouts();
  sched(); //schedule next task and set PendSV to trigger (as soon as interrupts are enabled)
  __asm("CPSIE I"); //enable interrupts)
  
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