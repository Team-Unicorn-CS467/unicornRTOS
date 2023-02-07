#ifndef MASKS_H
#define MASKS_H

#include <stdint.h>

//system control

///////////////////////  these are unnecessary //////////////////////////
//#define OPEN_GPIOF_CLOCK_GATE_MASK            0b00100000U //OR with SYSCTL_RCGCGPIO_R to open clock gating to GPIO port F
//#define CLOSE_GPIOF_CLOCK_GATE_MASK           0b11011111U //AND with SYSCTL_RCGCGPIO_R to close clock gating to GPIO port F

//gpio control
//#define SET_GPIO_PIN_TO_OUTPUT_MASK           0b00001110U //OR with GPIO_PORTF_DIR_R to set specified pins direction as output
//#define SET_GPIO_PIN_TO_DIGITAL_MASK          0b00001110U //OR with GPIO_PORTF_DEN_R to set specified pins as digital
//////////////////////////////////////////////////////////////////////////////

//general purpose
/*
#define BYTE_BIT_0                            0b00000001U
#define BYTE_BIT_1                            0b00000010U
#define BYTE_BIT_2                            0b00000100U
#define BYTE_BIT_3                            0b00001000U                                    
#define BYTE_BIT_4                            0b00010000U
#define BYTE_BIT_5                            0b00100000U
#define BYTE_BIT_6                            0b01000000U
#define BYTE_BIT_7                            0b10000000U
#define BYTE_BIT_ALL                          0b11111111U
#define BYTE_BIT_NONE                         0b00000000U
*/
#endif //MASKS_H