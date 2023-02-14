#ifndef USERTASKS_H
#define USERTASKS_H


#include "board_io.h"
#include "locks.h"
#include "unicorn.h" //for readyNewTask() ...


//function to spawn all user tasks at startup
void userTaskLoad();

///////////////////////////////////////////////////////////////////
// ADD ANY USER TASK DECLARATIONS BELOW
///////////////////////////////////////////////////////////////////

void blinkRed();
void blinkBlue();
void blinkGreen();
void blinky1();
void blinky2();
void anti_blinky();


#endif //USERTASKS_H