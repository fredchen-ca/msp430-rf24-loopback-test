/*
 * << MSP430F190 TA0 Timer Array Interrupt Library >>
 */
#ifndef _TIMER_LIB_H_
#define _TIMER_LIB_H_

#include <msp430f149.h>

//=======================================================================
//
// Timer Interrupt TA0 (10ms duration) Configuration:
// 
//=======================================================================
#define TM_MAX      5    // total timer count (>= 1)

//-----------------------------------------------------------------------
//
// System pre-defined timers
//
//-----------------------------------------------------------------------
#if 0   // in 10ms duration
 #define CCR0_BASE   8000  // TA0 intruupt duration clicks count based on 8Mhz
 #define TM_TIME_MS  10    // TA0 ISR count per ms
#else   // in 1 ms duration
 #define CCR0_BASE   800  // TA0 intruupt duration clicks count based on 8Mhz
 #define TM_TIME_MS  1    // TA0 ISR count per ms
#endif
#define TM_SEC      (1000/TM_TIME_MS)   // 1 second interrupt count  
#define TM_SYS      0     // timer ID #0  // system timer
#define TM_1        1     // timer ID #1  // else: application timers
#define TM_2        2     // timer ID #2
#define TM_3        3     // timer ID #3
#define TM_4        4     // timer ID #4
#define TM_5        5     // timer ID #5
#define TM_6        6     // timer ID #6
#define TM_7        7     // timer ID #7
#define TM_8        8     // timer ID #8
#define TM_9        9     // timer ID #9

//-----------------------------------------------------------------------
//
// Application defined timer names
//
//-----------------------------------------------------------------------
#define TM_TX       1     // timer ID #1
#define TM_RX       2     // timer ID #2
#define TM_RATE     3     // timer ID #3
#define TX_TMOUT    (200/TM_TIME_MS)    // TX timeout in TA msec unit
#define RX_TMOUT    (200/TM_TIME_MS)    // RX timeout in TA msec unit

//=======================================================================
//
//  << Public Functions Prototype >>
//
//=======================================================================

// timer TA0 init. routine
void init_tm(void);

// Reset specified timer to start count up 
void reset_tm(unsigned int idx);

// Retrieve specified timer value (elapsed count) 
unsigned int get_tm(unsigned int idx);

// Delay for about n operations via looping
void inerDelay(unsigned int n);

// delay for desired msec with interrupt
// input: ms must < 65535/TM_TIME_MS
void delay_ms(unsigned int ms);

#endif // _TIMER_LIB_H_
