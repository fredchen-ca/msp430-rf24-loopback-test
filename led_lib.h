// #################################################################
//
// MSP430F149 LEDx8 Routines on P2.x ports 
//
// #################################################################
#ifndef _LED_LIB_H_
#define _LED_LIB_H_

#include <msp430f149.h>

//==============================================================================
//
// MSP430 LED array output toggles
// in LSB(R) -> MSB(L) order
//
//==============================================================================
#define LED8_1 		(P2OUT &= ~BIT7) 	// L1 ON, active low
#define LED8_0 		(P2OUT |= BIT7) 	// L1 OFF
#define LED7_1 		(P2OUT &= ~BIT6) 	// L2 ON
#define LED7_0 		(P2OUT |= BIT6) 	// L2 OFF
#define LED6_1 		(P2OUT &= ~BIT5) 	// L3 ON 
#define LED6_0 		(P2OUT |= BIT5) 	// L3 OFF
#define LED5_1 		(P2OUT &= ~BIT4) 	// L4 ON
#define LED5_0 		(P2OUT |= BIT4) 	// L4 OFF
#define LED4_1 		(P2OUT &= ~BIT3) 	// L5 ON 
#define LED4_0 		(P2OUT |= BIT3) 	// L5 OFF
#define LED3_1 		(P2OUT &= ~BIT2) 	// L6 ON
#define LED3_0 		(P2OUT |= BIT2) 	// L6 OFF
#define LED2_1 		(P2OUT &= ~BIT1) 	// L7 ON 
#define LED2_0 		(P2OUT |= BIT1) 	// L7 OFF
#define LED1_1 		(P2OUT &= ~BIT0) 	// L8 ON
#define LED1_0 		(P2OUT |= BIT0) 	// L8 OFF
#define LED_ALL_0   (P2OUT |= 0xff)     // ALL LEDs OFF
#define LED_ALL_1   (P2OUT &= 0x00)     // ALL LEDs ON

//********************************************************************
//
// Unit Test Register Setting Validation Macro
//
// n: 0/1: RRF24L01_A/B
// r: Register Address
// v: asserted value
// m: mask bits of v
// p: checkpoint value to be displayed with LED
//********************************************************************
#if 1
  #define check(n, r, m, v, p)  if ((SPI_Read(n, READ_REG + r) & m) != v) onerr(p) 
#else
  #define check(n, r, m, v, p)  {}
#endif

//******************************************************************************************
// initialize 8 LED ports output
//******************************************************************************************
void init_led(void);

//******************************************************************************************
// display 8-bit value to LEDs in reverse order 
//******************************************************************************************
void show(unsigned char chkpt);

//******************************************************************************************
// flash checkpoint (0~255) on 8 leds on error then halt program execution
//******************************************************************************************
void onerr(unsigned char chkpt);

//******************************************************************************************
// flash checkpoint (0~255) on 8 leds on error then halt program execution
//******************************************************************************************
void display(unsigned char chkpt);

#endif  // _LED_LIB_H_


 
