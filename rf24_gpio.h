// #################################################################
//
// MSP430 SPI1 & SPI2 Headfile via GPIO ports 
//
// #################################################################
#ifndef _RF24_GPIO_H_
#define _RF24_GPIO_H_

#include <msp430f149.h>

//
// RF24 Module ID defines
//
#define RF24L01_A   0
#define RF24L01_B   1

// make compatible to MSP430 SPI1 master mode pin assignment
// defined: use swap cable connect to SPI1 RF24
// undefined: insert RF24 into J21 connector on MSP430 board
//
// Test OK on both swap and non-swap J21 connector on Oct 4, 2019
//
#if 1
 #define SWAP_MIMO   
 #warning "SWAP_MIMO defined"

#endif

//###############################################################################
// 
// First nRF24L01P (A) GPIO configuration for SPI interface 
//
//###############################################################################
#define RF24L01_A_CE_0 		(P4OUT &= ~BIT4)  // RF24 pin 3
#define RF24L01_A_CE_1 		(P4OUT |= BIT4)
#define RF24L01_A_CSN_0 	(P4OUT &= ~BIT5)  // RF24 pin 4
#define RF24L01_A_CSN_1 	(P4OUT |= BIT5)
#define RF24L01_A_SCK_0 	(P5OUT &= ~BIT3)  // RF24 pin 5
#define RF24L01_A_SCK_1 	(P5OUT |= BIT3)
#ifdef SWAP_MIMO  
  #warning "SWAP_MIMO swapped"

 #define RF24L01_A_MISO 	(P5IN & BIT2)     // RF24 pin 7
 #define RF24L01_A_MOSI_0 	(P5OUT &= ~BIT1)  // RF24 pin 6
 #define RF24L01_A_MOSI_1 	(P5OUT |= BIT1)
#else
  #warning "SWAP_MIMO NOT swapped"

 #define RF24L01_A_MISO 	(P5IN & BIT1)     // RF24 pin 7
 #define RF24L01_A_MOSI_0 	(P5OUT &= ~BIT2)  // RF24 pin 6
 #define RF24L01_A_MOSI_1 	(P5OUT |= BIT2)
#endif
// Note: IRQ pin for reference only
//#define RF24L01_A_IRQ 	P1.4              // RF24 pin 8

//###############################################################################
// 
// Second nRF24L01P (B) GPIO configuration for SPI interface 
//
//###############################################################################
#define RF24L01_B_CE_0 		(P4OUT &= ~BIT6)  // RF24 pin 3
#define RF24L01_B_CE_1 		(P4OUT |= BIT6)
#define RF24L01_B_CSN_0 	(P3OUT &= ~BIT0)  // RF24 pin 4
#define RF24L01_B_CSN_1 	(P3OUT |= BIT0)
#define RF24L01_B_SCK_0 	(P3OUT &= ~BIT3)  // RF24 pin 5
#define RF24L01_B_SCK_1 	(P3OUT |= BIT3)
#define RF24L01_B_MISO 		(P3IN & BIT2)     // RF24 pin 7
#define RF24L01_B_MOSI_0 	(P3OUT &= ~BIT1)  // RF24 pin 6
#define RF24L01_B_MOSI_1 	(P3OUT |= BIT1)
// Note: IRQ pin for reference only
//#define RF24L01_B_IRQ 	P1.7              // RF24 pin 8

void init_rf24_gpio(void);
unsigned char GPIO_RW(int nrf24, unsigned char data);

#endif // _RF24_GPIO_H_
