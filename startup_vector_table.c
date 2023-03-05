//#include "ticks.h" //for incrementTicks()
#include "exception_handlers.h"
#include "unicorn.h"

// FOR TESTING ONLY
#include "bsp.h"
uint32_t FirstPinAHBAddress = (uint32_t)(GPIOF_AHB); // redefined here to provide easy access in inline assembly
// FOR TESTING ONLY

//used to substitute instead of the IAR generic definition of the interrupt vector table

extern int CSTACK$$Limit; //just introducing the symbol to the compiler know this value will be pulled in later (during linking???)
void __iar_program_start(void); //again just introducing the function which will be pulled in later by the compiler

void handler_NMI(void) //denial of service for now
{
  while(1) {}
}

void handler_HardFault(void) //denial of service for now
{
  while(1) { NVIC_SystemReset(); }
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
  //disable interrupts
  __asm("CPSID I\n" 

  // FOR TESTING ONLY
  // modifying these registers is ok because they were already pushed upon interrupt entry
  // equivalent to BSP_setGPIO(GPIOF_AHB, GPIO_PF2, HIGH) which asserts the first pin 
    "LDR        R1, =FirstPinAHBAddress\n"
    "LDR        R0, [R1]\n"
    "MOVS       R1, #4\n"
    "MOVS       R2, #4\n"
    "STR        R2, [R0, R1, LSL #2]\n"
  // FOR TESTING ONLY  
        
  // save r4-r11 onto stack
    "PUSH       {r4-r11}\n"
  
  // currentTask->sp = sp;
    "LDR        R2, =currentTask\n"
    "LDR        R0, [R2]\n"
    "STR        SP, [R0]\n" 
    
  // sp = nextTask->sp;
    "LDR        R1, =nextTask\n"
    "LDR        R0, [R1]\n"
    "LDR        SP,[R0]\n"
  
  // currentTask = nextTask
    "STR        R0, [R2]\n"

  // clear exclusive monitor (for semaphores)
    "CLREX              \n"

  // blockSched = 0;
    "LDR        R2, =blockSched\n"
    "MOVS       R1, #0\n"
    "STR        R1, [R2]\n"
      
  // restore nextTask's r4-r11 regs
    "POP        {r4-r11}\n"
        
  // FOR TESTING ONLY
  // modifying these registers is ok because they were already pushed upon interrupt entry
  // equivalent to BSP_setGPIO(GPIOF_AHB, GPIO_PF2, LOW) which deasserts the first pin 
    "LDR        R1, =FirstPinAHBAddress\n"
    "LDR        R0, [R1]\n"
    "MOVS       R1, #4\n"
    "MOVS       R2, #0\n"
    "STR        R2, [R0, R1, LSL #2]\n"
  // FOR TESTING ONLY
      
  // enable interrupts:
    "CPSIE I\n"
  
  // return
    "BX         LR\n"); 
}

// tracking timeout tasks, incrementing ticks, then sched()
void handler_SysTick(void)
{  
  sysTickRoutine();
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