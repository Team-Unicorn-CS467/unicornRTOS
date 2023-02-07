#include "ticks.h"

void resetTicks(void)
{
  ticksElapsed = 0;
}

uint32_t getTicks(void)
{
  return ticksElapsed;
}

void incrementTicks(void)
{
  ++ticksElapsed;
}