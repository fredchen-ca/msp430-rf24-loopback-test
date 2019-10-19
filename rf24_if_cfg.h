// #################################################################
//
// MSP430 SPI interface configuration
// - SMCLK Divider for SPI SCK
// - using SPI or GPIO
// - in 3-pin/4-pin SPI mode
//
// #################################################################
#ifndef _RF24_IF_CFG_H_
#define _RF24_IF_CFG_H_

#include <msp430f149.h>

// DEBUG toggle
#define   ZERO    0

/* << SPI Interface Configuraion >>
 *
 * #define _RF24_SPI_ 
 * - when defined use rf24_spi.c  
 * - when undefined use rf24_gpio.c 
 *     
 * RF24 Interfaced via SPI (or GPIO if not defined )
 * - defined use SPI ports
 * - undefined use GPIO ports
 */
#if 1
//#####################################################################

  #define _RF24_SPI_  // interfaced with SPI port (via GPIO if undefined)
  #warning _RF24_SPI_ is ENABLED" 

  //---------------------------------------
  //
  // << RF24 IRQ (PIN-8) Support >>
  //
  //---------------------------------------
  #if 1
    #define RF24_IRQ   // RF24 IRQ support 
    #warning RF24_IRQ is ENABLED" 

    #if ZERO
       // enable ISR LED O/P
       #define DBG_RF24_ISR 
    #endif
  #endif

  // << Available 8MHz SMCLK Divider >>
  //
  // Note: This devider won't affect TA0
  // MSP430 maximum SPI clock is SMCLK/2 in master mode
  //
  #define CLKDIV    0x02        // SPI UCLK = SMCLK/CLKDIV

#endif

#endif // _RF24_IF_CFG_H_