// #################################################################
//
// MSP430 SPI interface configuration
// - SMCLK Divider for SPI SCK
// - using SPI or GPIO
// - in 3-pin/4-pin SPI mode
//
// #################################################################
#include <msp430f149.h>

/*
 * << 4-pin/3-pin Slave Mode Option >>
 *
 * Both modes test ok in fet140_spi0_08_spi1 on Oct-3-2019
 */
#if 1
 #define  SLAVE_4PIN_MODE   // enable STE output control to slave SPI
#endif

/* 
 * << fet140_spi0_08_spi1 Master SPI TX/RX Method configuration >>
 *
 */
#if 0
 #define SPI_IRQ   // TX/RX one byte use IRQ or spi1_rw()  
#endif

// << Available 8MHz SMCLK Divider >>
//
// Note: This devider won't affect TA0
// MSP430 maximum SPI clock is SMCLK/2 in master mode
//
#define CLKDIV    0x02        // SPI UCLK = SMCLK/CLKDIV
