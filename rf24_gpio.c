// #################################################################
//
// MSP430 SPI1 & SPI2 Routines via GPIO ports 
//
// #################################################################
#include "../device_lib/rf24_if_cfg.h"  // interfaced with SPI or GPIO ports

#ifndef _RF24_SPI_        // interfaced via GPIO port

#include <msp430f149.h>
#include "../device_lib/rf24_gpio.h"

#ifdef SWAP_SWAP_MOMO
  #message "SWAP_MIMO defined"
#endif

//=========================== MSP430 Port IO setup ==========================================
//    PxDIR 0/1: Input/Output
//    PxSEL 0/1: default function disabled/enabled
//    PxIE  0/1: disable/enable interrupt;
//    PxOUT 0/1: output low/high
//===========================================================================================    
void init_rf24_gpio(void)
{
    // setup SPI #1 & SPI #2 as master connect to RF24 as slave
    P3DIR = 0xfb;   // o/1: Input/Output, bit2 MISO #1 I/P
    P3SEL = 0x00;  

    P4DIR = 0xff;   // o/1: Input/Output, all O/P
    P4SEL = 0x00;
    
#ifdef SWAP_MIMO

    #warning "SWAP_MIMO swapped"

    P5DIR = ~BIT2;   // only P5.2:MISO as I/P    
#else
    #warning "SWAP_MIMO NOT swapped"

    P5DIR = ~BIT1;   // only P5.1:MISO as I/P
#endif
    P5SEL = 0x00;    
}

/************************************************** 
Function: GPIO_RW(); 
 
Description: 
  Writes one byte to nRF24L01, and return the byte read 
  from nRF24L01 during write, according to SPI protocol

input:
  nrf24: nRF24L01P module - 0/1: A/B

 **************************************************
 */
unsigned char GPIO_RW(int nrf24, unsigned char data)
{
    unsigned char i, temp = 0;
    for (i = 0; i < 8; i++) // output 8-bit
    {
        // output 'uchar', MSB to MOSI
        if ((data & 0x80) == 0x80) {
	        nrf24 ? RF24L01_B_MOSI_1 : RF24L01_A_MOSI_1; 
        }else {
            nrf24 ? RF24L01_B_MOSI_0 : RF24L01_A_MOSI_0;
        }
        data = (data << 1); // shift next bit into MSB..
        temp <<= 1;
        if (nrf24) {
	        RF24L01_B_SCK_1; // Set SCK high..
            if (RF24L01_B_MISO) {
                temp++; // capture current MISO bit
            }
            RF24L01_B_SCK_0; // ..then set SCK low again
        } else {
            RF24L01_A_SCK_1; // Set SCK high..
            if (RF24L01_A_MISO) {
                temp++; // capture current MISO bit
            }
            RF24L01_A_SCK_0; // ..then set SCK low again
        }
    }
    return (temp); // return read uchar
}

#endif // _RF24_SPI_