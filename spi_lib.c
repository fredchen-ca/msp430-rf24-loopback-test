// #################################################################
//
// MSP430 SPI1 & SPI2 Routines with optional Intrrupt supports 
//
// #################################################################
#include <msp430f149.h>
#include "../device_lib/spi_if_cfg.h"
#include "../device_lib/spi_lib.h"

#ifdef SPI_IRQ 
 unsigned int rx_rdy;   // new byte read from Master SPI
 volatile unsigned char rx_byte; // read byte
#endif
 
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << SPI0 port Write & Read >>
//
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
unsigned char spi0_rw(unsigned char out)
{ 
  // send out byte to TX
  TXBUF0 = out;

  // read RX byte when received
  while (!(IFG1 & URXIFG0));
  
  return (RXBUF0);  
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << SPI1 port Write & Read >>
// When connect to
// - 4-pin slave: toggle STE to slave to enable slave SPI operation
// - 3-pin slave: no STE control
// 
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
unsigned char spi1_rw(unsigned char out)
{ 
  volatile unsigned char data;
  
#ifdef  SLAVE_4PIN_MODE 
  SPI1_STE_0; // enable slave STE
#endif

#ifdef SPI_IRQ  // wait for RX_RDY interrupt
  rx_rdy = 0;
  
  // send out byte to TX
  TXBUF1 = out;
  
  while (!rx_rdy);
  data = rx_byte;
#else  
  // send out byte to TX
  TXBUF1 = out;
  
  // read RX byte when received
  while (!(IFG2 & URXIFG1));
  data = RXBUF1;
#endif
  
#ifdef  SLAVE_4PIN_MODE 
  SPI1_STE_1; // disable slave STE
#endif
  
  return (data);
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << Master SPI1 INTR >>
//
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#pragma vector=USART1RX_VECTOR
__interrupt void SPI1_rx (void)
{
#ifdef SPI_IRQ  
    rx_byte = RXBUF1;          
    rx_rdy = 1;
#endif
}

#pragma vector=USART1TX_VECTOR
__interrupt void SPI1_tx (void)
{
  // do nothing
  // TXBUF1 = 0x55;                          // Transmit character
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << Slave SPI0 INTR >>
//
// Description: Just loopback the RX_DATA to TX_BUF
//
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
#pragma vector=USART0RX_VECTOR
__interrupt void SPI0_rx (void)
{
  volatile unsigned char data;
  
  data = RXBUF0;        // RXBUF0 to TXBUF0
//  P2OUT = ~data;        // display RX data
  TXBUF0 = data & 0x0f; // Transmit character by maskout the MSB 4 bits
}

// Reserved 
#pragma vector=USART0TX_VECTOR
__interrupt void SPI0_tx (void)
{
//  do nothing
//  TXBUF0 = data;                             // Transmit character
}


//=========================== MSP430 Port IO setup ==========================================
//    PxDIR 0/1: Input/Output
//    PxSEL 0/1: default function disabled/enabled
//    PxIE  0/1: disable/enable interrupt;
//    PxOUT 0/1: output low/high
//===========================================================================================    

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << initialize SPI1 to 3-pin mater mode >>
//
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void init_spi1_master(void) {
    
  // Master SPI1 Setup  
  P5SEL |= 0x0E;                            // P5.1,2,3 SPI option select

#ifdef  SLAVE_4PIN_MODE  
  P4SEL &= ~BIT5;                           // P4.5 as GPIO pin
  P4DIR |= BIT5;                            // P4.5 as CSN O/P, active low
  P4OUT |= BIT5;                            // disable CSN
#endif
  
  U1CTL = CHAR + SYNC + MM + SWRST;         // 8-bit, SPI, Master
  U1TCTL = CKPH + SSEL1 + STC;              // Phase=1, polarity=0, SMCLK, 3-pin
  U1BR0 = CLKDIV;                           // SPICLK = SMCLK/2 (div=1 not working)
  U1BR1 = 0x00;
  U1MCTL = 0x00;
  ME2 |= USPIE1;                            // Module enable
  U1CTL &= ~SWRST;                          // SPI enable

  // Enable Master SPI INTR
#if 0
  // Enable Master/Slave INTR to start Transmission
  // IE2 |= URXIE1 + UTXIE1;                // RX and TX interrupt enable
#endif
  
#ifdef SPI_IRQ
  IE2 |= URXIE1;                            // RX interrupt enable
#endif
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << initialize SPI0 to slave 3-pin or 4-pin mode >>
//
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
void init_spi0_slave(void) {
  
  // Slave SPI0 Setup  
#ifdef  SLAVE_4PIN_MODE  
  P3SEL |= 0x0F;                            // P3.0,1,2,3 SPI option select
#else
  P3SEL |= 0x0E;                            // P3.1,2,3 SPI option select
  P3SEL &= ~BIT0;                           // P3.0 as GPIO pin
#endif
  
  U0CTL = CHAR + SYNC + SWRST;              // 8-bit, SPI, Slave
#ifdef  SLAVE_4PIN_MODE  
  U0TCTL = CKPH;                            // Phase=1, polarity=0, 4-pin
#else
  U0TCTL = CKPH + STC;                      // Phase=1, polarity=0, 3-pin
#endif
  
  U0BR0 = CLKDIV;                           // SPICLK = SMCLK/2 (4 MHz ?)
  U0BR1 = 0x00;
  U0MCTL = 0x00;
  ME1 |= USPIE0;                            // Module enable
  U0CTL &= ~SWRST;                          // SPI enable  

  // Enable Slave SPI INTR
#if 0
  // IE1 |= URXIE0 + UTXIE0;                // RX and TX interrupt enable
#else
    IE1 |= URXIE0;                          // RX interrupt enable
#endif
}
