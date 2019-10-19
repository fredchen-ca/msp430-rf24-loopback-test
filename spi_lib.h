// #################################################################
//
// MSP430 SPI1 & SPI2 Routines headfile with optional Intrrupt supports 
//
// #################################################################
#include <msp430f149.h>

// Slave STE control
#define SPI1_STE_0  (P4OUT &= ~BIT5) // enable Slave SPI, active low
#define SPI1_STE_1  (P4OUT |= BIT5)  // disable Slave SPI

// initialize SPI
void init_spi0_slave(void);
void init_spi1_master(void);

// SPI byte write & read
unsigned char spi0_rw(unsigned char out);
unsigned char spi1_rw(unsigned char out);
