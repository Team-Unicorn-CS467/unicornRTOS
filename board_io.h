
#ifndef BOARD_IO_H
#define BOARD_IO_H

#define RED   (1U << 1)
#define BLUE  (1U << 2)
#define GREEN (1U << 3)
#define OFF   0b00000000

#define SYSTICKS_PER_SEC 1000;


//#define RED (BYTE_BIT_1)
//#define BLUE (BYTE_BIT_2)
//#define GREEN (BYTE_BIT_3)

//note: they are not bundled into a single function 
// because Unicorn OS tasks currently can't be parameterized
void blinkRed();
void blinkBlue();
void blinkGreen();

// gate clocking, systick speed, digitial/direction enable
// set Systick/PendSV priority values
void boardStartup(void);



#endif //BOARD_IO_H