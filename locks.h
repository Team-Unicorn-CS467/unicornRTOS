#ifndef LOCKS_H
#define LOCKS_H


#include <stdint.h>

typedef struct
{
  uint32_t held; //stores 0 if available, 1 if held
} SpinLock;

void initLock(SpinLock*);

void releaseLock(SpinLock*);

void acquireLock(SpinLock*);


#endif //LOCKS_H