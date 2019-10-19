// #################################################################
//
// MSP430 SPI1 & SPI2 Routines w/o Interrupt 
//
// Test Result: 
// - 10042019: Test OK on swapped J21 connector of SPI1 ports
// - 10062019: Test OK on RF24_IRQ RX_DR support 
//
// #################################################################
#include "../device_lib/rf24_if_cfg.h"  // interfaced with SPI or GPIO ports

#ifdef _RF24_SPI_       // interfaced via SPI ports

#include <msp430f149.h>
#include "../device_lib/rf24_spi.h"

unsigned char rf24_ifg;  // RF24 interrupt flags on RF24_IRQ_PINS (1:on)

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << SPI0 port Write & Read >>
//
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
unsigned char spi0_rw(unsigned char out)
{ 
  // readout any pre-rx data first
  TXBUF0 = out;
  while (!(IFG1 & URXIFG0));  // RXSREG stored into RXBUF ?
  return RXBUF0;
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << SPI1 port Write & Read >>
//
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
unsigned char spi1_rw(unsigned char out)
{ 
  // send out byte to TX
  TXBUF1 = out;
  while (!(IFG2 & URXIFG1));  // RXSREG stored into RXBUF ?  
  return RXBUF1;
}

/************************************************** 
Function: spi_rw(); 
 
Description: 
  Writes one byte to nRF24L01, and return the byte read 
  from nRF24L01 during write, according to SPI protocol

input:
  nrf24: nRF24L01P module - 0/1: A/B (SPI1/SPI0)

Notes: the SPI device's CSN level will be handled by the caller

 **************************************************
 */
unsigned char spi_rw(int nrf24, unsigned char data)
{
	return (nrf24 ? spi0_rw(data) : spi1_rw(data));
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << initialize SPI1 >>
//
// ============ MSP430 Port IO setup =============
// PxDIR 0/1: Input/Output
// PxSEL 0/1: default function GPIO/selected
// PxIE  0/1: disable/enable interrupt;
// PxOUT 0/1: output low/high
//
// - 3-pin (UCLK, MISO, MOSI) Master Mode
// - Two GPIO pins used as RF24 CSN + CE
//
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void init_spi_A_master(void) {
    
  // master SPI1 Setup  
  P5SEL |= (BIT1 + BIT2 + BIT3);            // P5.1,2,3 SPI option select

  P4SEL &= ~(BIT4 + BIT5);                  // P4.4,5 as GPIO pin
  P4DIR |= (BIT4 + BIT5);                   // P4.4:CE, p4.5:CSN O/P
    
  U1CTL = CHAR + SYNC + MM + SWRST;         // 8-bit, SPI, master
  U1TCTL = CKPH + SSEL1 + STC;              // Phase=1, polarity=0, SMCLK, 3-wire
  U1BR0 = CLKDIV;                           // SPICLK = SMCLK/2 (div=1 not working)
  U1BR1 = 0x00;
  U1MCTL = 0x00;
  ME2 |= USPIE1;                            // Module enable
  U1CTL &= ~SWRST;                          // SPI enable
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << initialize SPI0 >>
//
// ============ MSP430 Port IO setup =============
// PxDIR 0/1: Input/Output
// PxSEL 0/1: default function GPIO/selected
// PxIE  0/1: disable/enable interrupt;
// PxOUT 0/1: output low/high
//
// - 3-pin (UCLK, MISO, MOSI) master Mode
// - Two GPIO pins used as RF24 CSN + CE 
//
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void init_spi_B_master(void) {
  
  // master SPI0 Setup  
  P3SEL |= (BIT1 + BIT2 + BIT3);            // P3.1,2,3 SPI option select
  P3SEL &= ~BIT0;                           // P3.0 as GPIO pin
  P3DIR |= BIT0;                            // P3.0:CSN O/P
  
  P4SEL &= ~BIT6;                           // P4.6 as GPIO pin
  P4DIR |= BIT6;                            // P4.6:CE O/P
  
  U0CTL = CHAR + SYNC + MM + SWRST;         // 8-bit, SPI, master
  U0TCTL = CKPH + SSEL1 + STC;              // Phase=1, polarity=0, SMCLK, 3-wire
  U0BR0 = CLKDIV;                           // SPICLK = SMCLK/2 (4 MHz ?)
  U0BR1 = 0x00;
  U0MCTL = 0x00;
  ME1 |= USPIE0;                            // Module enable
  U0CTL &= ~SWRST;                          // SPI enable 
}

// RF24 A&B IRQ initization routine
//
// ============ MSP430 Port IO setup =============
// PxDIR 0/1: Input/Output
// PxSEL 0/1: default function GPIO/selected
// PxIE  0/1: disable/enable interrupt;
// PxOUT 0/1: output low/high
void init_rf24_irq(void) 
{
  P1DIR &= ~RF24_IRQ_PINS;         // IRQ I/P, SPI1:P1.4, SPI0:P1.7               
  P1IES |= RF24_IRQ_PINS;          // Hi/lo edge
  P1IE  |= RF24_IRQ_PINS;          // interrupts enabled
  P1IFG  = 0x00;                   // IFG cleared

  _BIS_SR(GIE);                    // allow interrupt  
}

// Port 1 interrupt service routine for SPI0 & SPI1
#pragma vector=PORT1_VECTOR
__interrupt void RF24_isr(void)
{    
   rf24_ifg |= (P1IFG & RF24_IRQ_PINS); // set raised IFG bits 
   P1IFG    = 0x00;                     // clear all IFG bits 

#ifdef DBG_RF24_ISR
   P2OUT &= ~rf24_ifg;                // XXX:FRED Debug
#endif
}

/* check RF24 IRQ Status
 *
 * nrf24: 0/1:RF24_A/RF24_B
 * return: 1/0: true/flase
 * 
 */ 
int is_rf24_irq(int nrf24) {
    // RF24 IRQ Raised ?
    if (!(rf24_ifg & 
          (nrf24 ? RF24_B_IRQ_PIN : RF24_A_IRQ_PIN))) {
       // no IRQ 
       return 0;
    }
        
    _BIC_SR(GIE);     /* Interrupts disabled */

    // clear rf24_ifg IRQ bit
    rf24_ifg &= ~(nrf24 ? RF24_B_IRQ_PIN : RF24_A_IRQ_PIN);
         
    _BIS_SR(GIE);     /* Interrupts enabled */        
    
    return 1;
}  
#endif // _RF24_SPI_
