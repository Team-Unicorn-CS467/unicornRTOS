#ifndef UNICORN_H
#define UNICORN_H

#include <stdint.h>

#define TASK_STACK_WORD_SIZE 64U     // later this will not be a constant value
#define BYTES_PER_WORD       4U      // for 32 bit architecture
#define MAX_TASKS            8U

#define TASK_STATE_DORMANT   0U
#define TASK_STATE_READY     1U
#define TASK_STATE_ACTIVE    2U

/*** note: the ARM Application Procedure Call Standard (AAPCS) disallows
    clobbering of registers R4 through R11. I believe this means the compiler
    has responsibility to ensure these register values, by the time of procedure exit,
    have been returned to the values they posessed at the time of procedure entry. A
    procedure can modify them in the course of it's operations, so long as it
    restores them before exit.

    However, since the values of these registers can change mid-procedure, the
    kernel must save their current values on the current task's stack at the start
    of each context switch. It must also restore the last values of these registers for
    the next task just before completion of the context switch.

    The context switch also needs to perform similar actions to save and restore values
    of registers which are allowed to be colbered in AAPCS, but these are treated differently
    in part because, they have already been pushed to stack (which one???) via the
    Cortex-M exception stack frame functionality***/

typedef void (*EntryFunction)(); //a pointer to a void function which takes no arguments


/*** ContextFrame - stored register values to facilitate context shifting ***/

typedef struct
{
  
  /*** fabricated stack frame of registers dissallowed from being clobbered by
      a procedure call under the ARM Application Procedure Call Standard (AAPCS) ***/
  uint32_t r4;          //stored R4 register value
  uint32_t r5;          //stored R5 register value
  uint32_t r6;          //stored R6 register value
  uint32_t r7;          //stored R7 register value
  uint32_t r8;          //stored R8 register value
  uint32_t r9;          //stored R9 register value
  uint32_t r10;         //stored R10 register value
  uint32_t r11;         //stored R11 register value

  /*** fabricated Cortex-M interrupt stack frame
        (follows ARM exception frame layout, w/out the alligner word) ***/  
  uint32_t r0;          //stored R0 register value
  uint32_t r1;          //stored R1 register value  
  uint32_t r2;          //stored R2 register value
  uint32_t r3;          //stored R3 register value
  uint32_t r12;         //stored R12 register value  
  uint32_t lr;          //stored LR register value
  uint32_t pc;          //stored PC register value
  uint32_t xpsr;        //stored xPSR register value

} ContextFrame;

void initializeFirstFrame(ContextFrame*, EntryFunction);


/*** Task ***/

typedef struct
{
  uint32_t* sp; //stack pointer
  uint32_t  stack[TASK_STACK_WORD_SIZE]; //memory allocation for the stack - ***later this will be alocated outside this struct and passed in via a pointer***
  uint8_t   state;
} Task;

//set's up a Task's initial stack and member variables
void initializeTask(Task*, EntryFunction);

extern Task* currentTask; //initialized in unicorn.c
extern Task* nextTask; //initialized in unicorn.c


/*** Scheduling Stuff ***/

//the idleTask's job
void pointlessWork();

//starting setup of the task table, idleTask
void initializeScheduler();

//initializes a new Task and marks it as ready to run
void readyTaskStart(EntryFunction);

//called by a task to exit
//assumes initializeScheduler() has previously been called
void readyTaskEnd();

//potentially schedules a new stack and context switches
void sched();

#endif //UNICORN_H