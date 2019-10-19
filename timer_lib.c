/*
 * << MSP430F190 TA0 Timer Array Interrupt Library >>
 *
 * 
 */

// MSP430 family heads file
#include <msp430f149.h>
#include "timer_lib.h"

unsigned int tm[TM_MAX]; // timer counter accumulated on TA0 interrupt

// ---------------------------------------
// 
// Reset specified timer to start count up 
//
//----------------------------------------
void reset_tm(unsigned int idx) {
  if (idx < TM_MAX) {
      _BIC_SR(GIE);     /* Interrupts disabled */
      tm[idx] = 0;      /* reset TM0 counter */
      _BIS_SR(GIE);     /* Interrupts enabled */
  }
}

// ---------------------------------------------
// 
// Retrieve specified timer value (elapsed count) 
//
//----------------------------------------------
unsigned int get_tm(unsigned int idx) {
  if (idx < TM_MAX) {
      return tm[idx];      /* reset TM0 counter */
  } else {
      return 0;
  }
}

//******************************************************************************************
// Delay for about n operations
//******************************************************************************************
void inerDelay(unsigned int n)
{
    for (; n>0; n--);
}

//========================
//
// delay for desired msec
//
// input: ms must < 65535/TM_TIME_MS
//========================
void delay_ms(unsigned int ms)
{
    unsigned int end, elapse;

    reset_tm(TM_SYS);
    elapse = 0;
    end = (ms > (65530/TM_TIME_MS)) ?
      65530 : TM_TIME_MS * ms;      // how many msec units to wait ?

    while (elapse < end)
    {
        inerDelay(100);  // sleep for a while
        elapse = get_tm(TM_SYS);
    }
}


//######################################
//
// timer TA0 init. routine
//
//######################################
void init_tm(void)
{
  CCTL0 = CCIE;               // CCR0 interrupt enabled
  TACTL = TASSEL_2 + MC_2;    // SMCLK, contmode
  CCR0  = CCR0_BASE;

  _BIS_SR(GIE);               // Enter AM w/ interrupt
}

//=====================================================================
// 
// Timer A0 interrupt service routine every CCR0_BASE time accumulated 
//
//=====================================================================
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
    int i;
    
    CCR0 += CCR0_BASE;  // Reset CCR0 base
    
    // increase all timers
    for (i=0; i< TM_MAX; i++) {
        tm[i]++;
    }
}

