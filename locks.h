#ifndef LOCKS_H
#define LOCKS_H


#include <stdint.h>

/***SpinLock Stuff***/
typedef struct
{
  uint32_t held; //stores 0 if available, 1 if held
} SpinLock;

void initLock(SpinLock*);

void releaseLock(SpinLock*);

void acquireLock(SpinLock*); //blocking function


#endif //LOCKS_H