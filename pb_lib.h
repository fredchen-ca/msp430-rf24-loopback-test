//==============================================================================
//
// MSP430 Push Button Inputs
//
//==============================================================================
#ifndef _PB_LIB_H_
#define _PB_LIB_H_

#include <msp430f149.h>

#define KEY1_KIN    !(P1IN & BIT0)		// KEY1: p1.0, active low
#define KEY2_KIN    !(P1IN & BIT1)		// KEY2: p1.1, active low
#define KEY3_KIN    !(P1IN & BIT2)		// KEY3: p1.0, active low
#define KEY4_KIN    !(P1IN & BIT3)		// KEY4: p1.1, active low

// initialize push botton port inputs
void init_pb(void);

#endif // _PB_LIB_H_
