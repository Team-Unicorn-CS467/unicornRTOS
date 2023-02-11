#ifndef LOCKS_H
#define LOCKS_H


#include <stdint.h>

extern uint32_t SpinLock; //initialized in locks.c

void initSpinLock();

void releaseSpinLock();

void acquireSpinLock();


#endif //LOCKS_H