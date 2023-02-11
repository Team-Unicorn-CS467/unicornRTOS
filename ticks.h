#ifndef TICKS_H
#define TICKS_H

#include <stdint.h>


#define SYSTICKS_PER_SEC 1000;

//static sets scope to file level;  volatile means compiler won't modify it w/optimization
static uint32_t volatile ticksElapsed;

void resetTicks(void);

uint32_t getTicks(void);

void incrementTicks(void);

#endif //TICKS_H