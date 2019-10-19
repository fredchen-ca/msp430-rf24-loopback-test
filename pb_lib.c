// #################################################################
//
// MSP430F149 four push bottons Routines on P1.x ports
//
// #################################################################
#include <msp430f149.h>
#include "../device_lib/pb_lib.h"

//=========================== MSP430 Port IO setup ==========================================
//    PxDIR 0/1: Input/Output
//    PxSEL 0/1: default function disabled/enabled
//    PxIE  0/1: disable/enable interrupt;
//    PxOUT 0/1: output low/high
//===========================================================================================    

// initialize push botton port inputs
void init_pb(void) {
    // PB_SW Inputs
    P1DIR &= ~(BIT0 + BIT1 + BIT2 + BIT3);  // reset to 0 as Input, P1.0~3, KEY1~KEY4, active low
    P1SEL &= ~(BIT0 + BIT1 + BIT2 + BIT3);  // reset to GPIO mode
    P1IE  &= ~(BIT0 + BIT1 + BIT2 + BIT3);  // disable all pb.0~3 interrupts   
}