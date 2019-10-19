// #################################################################
//
// MSP430 SPI1 & SPI2 Headfile via GPIO ports 
//
// #################################################################
#ifndef _RF24_SPI_H_
#define _RF24_SPI_H_

#include <msp430f149.h>

//
// RF24L01 Module ID defines
//
#define RF24L01_A   0   // connect to SPI1
#define RF24L01_B   1   // connect to SPI0

// RF24 INQ PINs (must connect on the same port group (eg. port 1)
#define RF24_A_IRQ_PIN  BIT4  // connect to P1.4
#define RF24_B_IRQ_PIN  BIT7  // connect to P1.7
#define RF24_IRQ_PINS   (RF24_A_IRQ_PIN + RF24_B_IRQ_PIN) // as bits mask
 
//###############################################################################
// 
// First nRF24L01P (A) GPIO configuration for SPI1 interface on RF socket
//
//###############################################################################
#define RF24L01_A_CE_0 	  (P4OUT &= ~BIT4)  // RF24L01 pin 3
#define RF24L01_A_CE_1 	  (P4OUT |= BIT4)
#define RF24L01_A_CSN_0   (P4OUT &= ~BIT5)  // RF24L01 pin 4
#define RF24L01_A_CSN_1   (P4OUT |= BIT5)

//###############################################################################
// 
// Second nRF24L01P (B) GPIO configuration for SPI0 interface on generic GPIO socket
//
//###############################################################################
#define RF24L01_B_CE_0 	  (P4OUT &= ~BIT6)  // RF24L01 pin 3
#define RF24L01_B_CE_1 	  (P4OUT |= BIT6)
#define RF24L01_B_CSN_0   (P3OUT &= ~BIT0)  // RF24L01 pin 4
#define RF24L01_B_CSN_1   (P3OUT |= BIT0)

// initialize SPI
void init_spi_A_master(void);  // SPI1
void init_spi_B_master(void);  // SPI0

// SPI byte write & read
unsigned char spi_rw(int nrf24, unsigned char out);

// RF24 A&B IRQ initization routine
void init_rf24_irq(void);
        
// check RF24 IRQ Status
int is_rf24_irq(int nrf24);

#endif // _RF24_SPI_H_