// #################################################################
//
// MSP430F149 LEDx8 Routines on P2.x ports
//
// #################################################################
#include <msp430f149.h>
#include "../device_lib/led_lib.h"
#include "../device_lib/timer_lib.h"

//=========================== MSP430 Port IO setup ==========================================
//    PxDIR 0/1: Input/Output
//    PxSEL 0/1: default function disabled/enabled
//    PxIE  0/1: disable/enable interrupt;
//    PxOUT 0/1: output low/high
//===========================================================================================    

// initialize 8 LED port output
void init_led(void) {
#if 1
  // all P2 pins used as LED Outputs
  P2DIR = 0xff;   // o/1: Input/Output, all O/P, LED1~LED7, O/P active low
  P2SEL = 0x00;   // resst all P2.x to GPIO mode 
  P2IE  = 0x00;   // disable all P2.x interrupt
  P2OUT = 0xff;   // off all LEDs
#else
  // disable P2 port output leave LEDs for hard contact signal debugging
  P2DIR = 0x00; // all I/P
  P2SEL = 0x00; // all GPIO pins
  P2IE  = 0x00; // no IRQ
#endif
}

// display 8-bit value to LEDs in reverse order 
void show(unsigned char chkpt) {
#if 1
    (chkpt & 0x01) ? LED1_1 : LED1_0;
    (chkpt & 0x02) ? LED2_1 : LED2_0;
    (chkpt & 0x04) ? LED3_1 : LED3_0;
    (chkpt & 0x08) ? LED4_1 : LED4_0;
    (chkpt & 0x10) ? LED5_1 : LED5_0;
    (chkpt & 0x20) ? LED6_1 : LED6_0;
    (chkpt & 0x40) ? LED7_1 : LED7_0;
    (chkpt & 0x80) ? LED8_1 : LED8_0;
#endif
}


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
// flash checkpoint (0~255) on 8 leds on error then halt program execution
//******************************************************************************************
void onerr(unsigned char chkpt) {
#if 1
  while (1) {
    show(chkpt);
    delay_ms(200);
    LED_ALL_0;
    delay_ms(200);
  }
#endif 
}

//******************************************************************************************
// flash checkpoint (0~255) on 8 leds on error then halt program execution
//******************************************************************************************
void display(unsigned char chkpt) {
#if 1
    show(chkpt);
    delay_ms(100);
    LED_ALL_0;
    delay_ms(10);
#endif
}
