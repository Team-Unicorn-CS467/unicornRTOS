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

//note: they are not bundled into a single function 
// because Unicorn OS tasks currently can't be parameterized
void blinkRed();
void blinkBlue();
void blinkGreen();

#endif //USERTASKS_H