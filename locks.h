#ifndef LOCKS_H
#define LOCKS_H


#include <stdint.h>

/***MutexLock Stuff***/
typedef struct
{
  uint32_t held; //stores 0 if available, 1 if held
} MutexLock;

void initLock(MutexLock*);

void releaseLock(MutexLock*);

void acquireLock(MutexLock*); //blocking function

uint32_t tryAquireLock(MutexLock*); //non blocking function

#endif //LOCKS_H