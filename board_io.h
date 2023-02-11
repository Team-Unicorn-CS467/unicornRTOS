#ifndef BOARD_IO_H
#define BOARD_IO_H

#define RED   (1U << 1)
#define BLUE  (1U << 2)
#define GREEN (1U << 3)
#define OFF   (0U)

#include "TM4C123GH6PM.h"


// gate clocking, systick speed, digitial/direction enable
// set Systick/PendSV priority values
void boardStartup(void);

void ledRedOn();
void ledRedOff();

void ledGreenOn(); 
void ledGreenOff();
    
void ledBlueOn();  
void ledBlueOff(); 


#endif //BOARD_IO_H