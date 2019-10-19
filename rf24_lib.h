#ifndef _RF24_LIB_H_
#define _RF24_LIB_H_

#include "../device_lib/rf24_if_cfg.h"  // interfaced with SPI or GPIO ports
#include "../device_lib/timer_lib.h"

#ifdef  _RF24_SPI_  // via SPI port
 #include "../device_lib/rf24_spi.h"
#else // via GPIO port
 #include "../device_lib/rf24_gpio.h"
#endif

// respective SPI port write/read function
#ifdef _RF24_SPI_
 #define SPI_RW  spi_rw     // interface to SPI via SPI ports 
#else
 #define SPI_RW  GPIO_RW    // interface to SPI via GPIO ports instead
#endif

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//      << SW Features Selection >>
// Packet TX Rate under 2Mbps, 32/32 bytes:
// ------------------------------------------------------
//  Config     | NON-AA |  AA  |  AA-PL  | Note     
//             |        |      |(1/6)pipe|
// -------------------------------------------------------
//  GPIO       |   19   |  11  |   11    | 10-07-2019
//  SPI        |   92   |  54  |  54/70  | 10-07-2019
// RF24_IRQ    |   91   |  53  |  53/70  | 
// SPI-TX-ONLY |  167   |  --  |   --    | 10-07-2019
// RF24_IRQ-TX |  167   |  --  |   --    | 10-07-2019
// --------------------------------------=----------------
// RF24-IRQ,AA-PL, Width 2&5 bytes: 109/132 of (1/6) pipes
// Received from RPi2 got maximum throughput: 
//   52.28kpbs in non-AA 32 bytes payload @ 205 packets per sec
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

// #############################################################################
//
//        << RF24 Features Configuration >>
//
// #############################################################################
#if 1
  #define ENABLE_PTX      // enable TX on RF_A
  #warning "ENABLE_PTX is ENABLED"
#endif

#if 1
  #define ENABLE_PRX      // enable RX on RF_B
  #warning "ENABLE_PRX is ENABLED"
#endif

#if 1
  #define TX_6_PIPES      // test TX on all 6 pipes
  #warning "TX_6_PIPES is ENABLED"
#endif

#ifdef  ENABLE_PRX      
 #if 1
  #define AUTO_ACK        // enable Auto_ACK onfiguration and handling code
  #if 1
    #define ACK_PL         // allow PRX to send payload with ACK in DYNPL feature
  #endif
 #endif
#endif

#ifndef DBG_RF24_ISR    // when LED reserved for RF24 IRQ ISR debugging

// LED Debugging Display Toggles
//-----------------------------------------------------------
// Features Enabled: AA, DNPL Test OK Note (Data_Width/Ack_Width)
// DSP_RX  : (32/5), (2/5)
// RSP_TX  : (32/5), (2/5)
// DSP_ACK : (32/5), (2/5)
// DSP_RATE: (32/5), (2/5)
//-----------------------------------------------------------
#if 0
 #warning "Show RX Data ENABLED"
 #define DSP_RX   
#elif 0
 #warning "Show TX Data ENABLED"
 #define DSP_TX
#elif 0
 #ifdef ACK_PL    // support when AUTO_ACK & ACK_PL defined 
  #warning "Show RX ACK Data ENABLED"
  #define DSP_ACK
 #endif
#else
 #warning "Show Maximum TX Packet Rate ENABLED"
 #define DSP_RATE    
#endif

#endif // DBG_RF24_ISR 

//****************************************************************
//
// SPI(nRF24L01) command instructions 
//
//****************************************************************
#define READ_REG 	    0x00 // Define read command to register
#define WRITE_REG 	    0x20 // Define write command to register
//#define ACTIVATE        0x50 // Not Used for nRF24L01P
#define RD_RX_PL_WID    0x60 // returns the DPL packet size at the head of the queue
#define RD_RX_PLOAD     0x61 // Define RX payload register address
#define WR_TX_PLOAD     0xA0 // Define TX payload register address
#define WR_ACK_PLOAD    0xA8 // Write an ACK payload to selected pipe in 1~32 bytes
#define WR_TX_PL_NOACK  0xB0 // Write data payload into Tx FIFO, and disable auto-ack for only that payload packet.
#define FLUSH_TX 	    0xE1 // Define flush TX register command
#define FLUSH_RX 	    0xE2 // Define flush RX register command
#define REUSE_TX_PL     0xE3 // Define reuse TX payload register command
#define NOP 		    0xFF // Define No Operation, might be used to read status register

//***************************************************
//
// SPI(nRF24L01) registers(addresses)
//
//***************************************************
#define CONFIG 		  0x00 // 'Config' register address
#define EN_AA 		  0x01 // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR 	  0x02 // 'Enabled RX addresses' register address
#define SETUP_AW 	  0x03 // 'Setup address width' register address
#define SETUP_RETR 	  0x04 // 'Setup Auto. Retrans' register address
#define RF_CH 		  0x05 // 'RF channel' register address
#define RF_SETUP 	  0x06 // 'RF setup' register address
#define STATUS 		  0x07 // 'Status' register address
#define OBSERVE_TX 	  0x08 // 'Observe TX' register address
#define CD 		      0x09 // 'Carrier Detect' register address
#define ADDR_P0 	  0x0A // 'RX address pipe0' register address
#define ADDR_P1 	  0x0B // 'RX address pipe1' register address
#define ADDR_P2 	  0x0C // 'RX address pipe2' register address
#define ADDR_P3 	  0x0D // 'RX address pipe3' register address
#define ADDR_P4 	  0x0E // 'RX address pipe4' register address
#define ADDR_P5 	  0x0F // 'RX address pipe5' register address
#define TX_ADDR 	  0x10 // 'TX address' register address
#define RX_PW_P0 	  0x11 // 'RX payload width, pipe0' register address
#define RX_PW_P1 	  0x12 // 'RX payload width, pipe1' register address
#define RX_PW_P2 	  0x13 // 'RX payload width, pipe2' register address
#define RX_PW_P3 	  0x14 // 'RX payload width, pipe3' register address
#define RX_PW_P4 	  0x15 // 'RX payload width, pipe4' register address
#define RX_PW_P5 	  0x16 // 'RX payload width, pipe5' register address
#define FIFO_STATUS   0x17 // 'FIFO Status Register' register address
#define DYNPD         0x1C // Enable or disable the dynamic payload calculation feature on the Rx pipes
#define FEATURE       0x1D // Enable or disable the dynamic payload, ack payload, and selective ack features

//***************************************************
//
// SPI(nRF24L01) Status register R/W values
//
//***************************************************
#define ST_RX_DR	  0x40	// R/W	Data Ready RX FIFO interrupt. Asserted when new data arrives RX FIFO.
                            // Write 1 to clear bit.
#define ST_TX_DS	  0x20	// R/W	Data Sent TX FIFO interrupt. Asserted when packet transmitted on TX. If AUTO_ACK is activated, this bit is set high only when ACK is received.
                            // Write 1 to clear bit.
#define ST_MAX_RT	  0x10	// R/W	Maximum number of TX retransmits interrupt.  Write 1 to clear bit.  If MAX_RT is asserted it must be cleared to enable further communication.

#define ST_RX_P_NO	  0x0e	// Data pipe number for the payload available for reading from RX_FIFO
                            // 000-101: Data Pipe Number
                            // 110: Not Used
                            // 111: RX FIFO Empty
#define ST_RX_EMPTY   0x0e  // 111: RX FIFO Empty
#define ST_TX_FULL	  0x01	// TX FIFO full.

//***************************************************
//
// SPI(nRF24L01) FIFO Status register R/W values
//
//***************************************************
#define FF_TX_REUSE	  0x40	// Reuse last transmitted data packet 
#define FF_TX_FULL	  0x20	// TX FIFO full flag.
#define FF_TX_EMPTY	  0x10  // TX FIFO empty flag.
#define Ff_RX_FULL	  0x02	// RX FIFO full flag.
#define FF_RX_EMPTY	  0x01  // RX FIFO empty flag.


/************************************************** 
 Function: SPI_Read(); 
 
 Description: 
  Read one byte from nRF24L01 register, 'reg'

 input:
  nrf24: nRF24L01P module - 0/1: A/B

 *************************************************
 */
unsigned char SPI_Read(int nrf24, unsigned char reg);

/************************************************** 
 Function: SPI_RW_Reg(); 
 
 Description: 
  send one byte register 'reg' instruction w/o parameter 

 input:
  nrf24: nRF24L01P module - 0/1: A/B

 *************************************************
 */
unsigned char SPI_Write_Reg(int nrf24, unsigned char reg);

/************************************************** 
 Function: SPI_RW_Reg(); 
  
 Description: 
  Writes value 'value' to register 'reg'

 input:
  nrf24: nRF24L01P module - 0/1: A/B

 *************************************************
 */
unsigned char SPI_RW_Reg(int nrf24, unsigned char reg, unsigned char value);

/************************************************** 
 Function: SPI_Read_Buf(); 
 
 Description: 
  Reads 'bytes' #of bytes from register 'reg' 
  Typically used to read RX payload, Rx/Tx address 

 input:
  nrf24: nRF24L01P module - 0/1: A/B

 *************************************************
 */
unsigned char SPI_Read_Buf(int nrf24, unsigned char reg, unsigned char* pBuf, unsigned char chars);

/************************************************** 
 Function: SPI_Write_Buf(); 
 
 Description: 
  Writes 'bytes' from pBuff to register 'chars' 
  Typically used to write TX payload, Rx/Tx address
 **************************************************
 */
unsigned char SPI_Write_Buf(int nrf24, unsigned char reg, unsigned char* pBuf, unsigned char chars);

//****************************************************************************************************/
// void SetRX_Mode(void) w/Auto-ACK enabled
//****************************************************************************************************/
void SetRX_Mode(int nrf24);

//******************************************************************************************************/
// unsigned unsigned char nRF24L01_RxPacket(unsigned unsigned char* rx_buf)
// RX a packet
// return:
// -1: none
// 0~5: pipe of Rx data
//******************************************************************************************************/
int nRF24L01_RxPacket(int nrf24, unsigned char* rx_buf, unsigned char *size);

//***********************************************************************************************************
//void nRF24L01_TxPacket(unsigned char * tx_buf)
//TX a packet for PTX mode
//**********************************************************************************************************/
void nRF24L01_TxPacket(int nrf24, unsigned char* tx_addr, int addr_len, unsigned char* tx_buf, int buf_size);


#endif // _RF24_LIB_H_
