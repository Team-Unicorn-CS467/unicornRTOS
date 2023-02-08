#ifndef LOCKS_H
#define LOCKS_H

#include <stdint.h>

extern uint32_t* TaskTableLockAddress; //initialized in locks.c

void initTaskTableLock();

void releaseTaskTableLock();

void acquireTaskTableLock();



#endif //LOCKS_H