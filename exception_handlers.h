#ifndef EXCEPTION_HANDLERS_H
#define EXCEPTION_HANDLERS_H


/*** exception handler declarations ***/

void handler_NMI(void);
void handler_HardFault(void);
void handler_MemoryManagementFault(void);
void handler_BusFault(void);
void handler_UsageFault(void);
void handler_SVCall(void);
void handler_PendSV(void);
void handler_SysTick(void);


#endif //EXCEPTION_HANDLERS_H