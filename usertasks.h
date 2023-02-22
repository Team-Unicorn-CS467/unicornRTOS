#ifndef USERTASKS_H
#define USERTASKS_H


#include "board_io.h"
#include "locks.h"
#include "unicorn.h" // for readyNewTask() ...


// function to spawn all user tasks at startup
void userTaskLoad();


#endif //USERTASKS_H