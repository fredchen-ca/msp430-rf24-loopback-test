/*
 * << RF packet delivery using two nRF modules as PTX, PRX via SPI or GPIO ports over multiple pipes >>
 * ================================================================================
 *
 * Note: Master Copy of both AA and non-AA mode
 *
 * External loopback test with two nRF24L01P modules connected 
 * to single MSP430F149 target board
 * nRF24L01P programmed in GPIO mode via SPI interface, AutoACK enabled
 *
 * Based on the rf24_autoack_new_spi
 *
 * This program can be configured (TX/RX only) to work with the transmitter/receiber programs 
 * located at RPi2B ~/RF24/simple_example directory
 *
 * Test Results: 
 * - 10-04-2019: Test Ok on both GPIO and SPI configurartions with swapped J21 connector of SPI1 
 * - 10-07-2019: Test OK on SPI on either IRQ or NON-IRQ with swapped J21 connector of SPI1 for all pipes (0~5)
 */

/*======================================================
 *
 *     << SPI Interface Configuraion >>
 *
 * RF24 Interfaced via SPI (or GPIO if not defined )
 * - defined use SPI ports
 * - undefined use GPIO ports
 *
 *====================================================== 
 */
#include "../device_lib/rf24_if_cfg.h"  // interfaced with SPI or GPIO ports

// MSP430 family includes
#include <string.h>
#include <msp430f149.h>
#include "../device_lib/timer_lib.h"
#include "../device_lib/led_lib.h"
#include "../device_lib/pb_lib.h"
#include "../device_lib/rf24_lib.h"

#ifdef  _RF24_SPI_  
 // via SPI port
 #include "../device_lib/rf24_spi.h"
#else 
 // via GPIO port
 #include "../device_lib/rf24_gpio.h"
#endif

#if 1
  #define DBG_STATUS
#endif

#define LOOP  10    // display debug value loop

//=====================================================================
//
//      << nRF24L01p modules configuration >>
//
//=====================================================================

#define RF_CHANNEL    112             // 0~125 prefer:101~119
#if 1
 #define RF_SETUP_V  0x0f              // Datarate:2Mbps, PA:0dBm, LNA:HCURR
#else
 #define RF_SETUP_V  0x07              // Datarate:1Mbps, PA:0dBm, LNA:HCURR
#endif

#define RX_PIPE     5                 // Tested ACK/RX pipe # (0~5)
//--------------------------------------------------------------------------------
#define PIPE_SLOT   (0x01 << RX_PIPE) // RF_B Pipe slot (bitwise 0x01 ~ 0x20)
#define EN_AA_PIPES 0x3f              // AA PIPES used (must enable all pipes (?) 
#if 1   // enable all or single pipe
  #define EN_RX_PIPES 0x3f            // all six PIPES used 
#else
  #define EN_RX_PIPES PIPE_SLOT       // single RX PIPES used 
#endif
//---------------------------------------------
// Auto ACK related toggles 
// Modes: (All 3 modes test ok on Aug 14, 2019)
// - No ACK
// - AUTO_ACK
// - AUTO_ACK + ACK_PL
//---------------------------------------------
#define ACK_IDX   1     // ack_cnt location index in ACK buffer
#define RETRY     0x01  // AA retry timeout, max

//==========================NRF24L01============================================
#define TX_ADR_WIDTH 	5 	        // 5 bytes TX address width
#define RX_ADR_WIDTH 	5 	        // 5 bytes RX address width
#define DATA_SIZE       2           // 32 or 1~31 (DYNPL)
#define TX_PL_WIDTH 	DATA_SIZE 	// TX payload size
#define RX_PL_WIDTH 	DATA_SIZE 	// RX payload size
#define ACK_PL_WIDTH    5           // ACK payload size 

// DYN_PL only enabled when ACK_PL feature is used
#ifdef  AUTO_ACK
 #ifdef ACK_PL
  #define RF_FEATURE    0x06        // EN_DPL, EN_ACK_PAY
  #define RF_DYNPD       0x3f        // enable on all 6 pipes 
 #else
  #define RF_FEATURE    0x00        // no EN_DPL, no EN_ACK_PAY
  #define RF_DYNPD       0x00        // disable all 6 pipes 
 #endif
#else // NO AUTO_ACK
 #define RF_FEATURE     0x00        // disable EN_DPL, EN_ACK_PAY
 #define RF_DYNPD       0x00        // disable all 6 pipes 
#endif

//============================================================
// TX/RX Address bytes in LSB -> MSB order
// P0
// Test Results:
// - Pipe #0:
//   tx1=rx1=tx2=rx2: OK
//   tx1=rx2, tx2=rx1: OK
//
//============================================================

// XXX: PRX mode no need to setup TX_ADDRESS (dummy)
unsigned char ADDR_P0_BUF[RX_ADR_WIDTH] = { '0', 'R','O','O','T' }; //Pipe#0 address
unsigned char ADDR_P1_BUF[RX_ADR_WIDTH] = { '1', 'N','O','D','E' }; //Pipe#1 address
unsigned char ADDR_P2_BUF[RX_ADR_WIDTH] = { '2', 'N','O','D','E' }; //Pipe#2 address
unsigned char ADDR_P3_BUF[RX_ADR_WIDTH] = { '3', 'N','O','D','E' }; //Pipe#3 address
unsigned char ADDR_P4_BUF[RX_ADR_WIDTH] = { '4', 'N','O','D','E' }; //Pipe#4 address
unsigned char ADDR_P5_BUF[RX_ADR_WIDTH] = { '5', 'N','O','D','E' }; //Pipe#5 address
unsigned char *PIPE_ADDR_LIST[6] = {ADDR_P0_BUF, ADDR_P1_BUF, ADDR_P2_BUF, ADDR_P3_BUF, ADDR_P4_BUF, ADDR_P5_BUF}; //RF_A PTX TX/RX_P1~5 address

// Globals Declaration
unsigned char ACK_Buf[32]; 
unsigned char tf, Rx1_Buf[32], Tx1_Buf[32], Rx2_Buf[32], Tx2_Buf[32];
unsigned char data[] = "Tested Message";
int  mode_a;   // RF A process mode
int  mode_b;   // RF B process mode
int  sts1, sts2, sts3, sts4;
unsigned char rt_cnt,to_a_cnt, to_b_cnt;   
unsigned char tx_pkt_rate, rx_pkt_rate;
int  rx_cnt, tx_cnt, has_rx;
int  halt_led_toggle;
int  tx_pipe_no;   // current TX pipe in used
unsigned char ack_cnt;

//****************************************************************************************
//
// NRF24L01p A init as PTX mode
//
//***************************************************************************************/
void init_NRF24L01_A(void)
{   
//     unsigned char byte;
     
    /*  nRF24L01p Soft-reset sequence
     *  1)use power down mode (PWR_UP = 0) 
     *  2)clear data ready flag and data sent flag in status register 
     *  3)flush tx/rx buffer 4)write status register as 0x0e
     */
  
    // for NRF24_A
    RF24L01_A_CE_0;  // disable RF TX/RX until start TX or into RX mode
    RF24L01_A_CSN_1; // disable SPI operations

#ifndef  _RF24_SPI_
    RF24L01_A_SCK_0; // Spi clock line init high
#endif 
    inerDelay(100);
    
    /*  nRF24L01p Soft-reset sequence
     *  1)use power down mode (PWR_UP = 0) 
     *  2)clear data ready flag and data sent flag in status register 
     *  3)flush tx/rx buffer 
     */
    SPI_RW_Reg(RF24L01_A, WRITE_REG + CONFIG, 0x72);  // disable IRQ pin, clear PWR_UP bit
    SPI_RW_Reg(RF24L01_A, WRITE_REG + STATUS, 0x70);  // clear RX_DR, TX_DS, MAX_RT bits
    SPI_Write_Reg(RF24L01_A, FLUSH_TX);               // flush TX buffer    
    SPI_Write_Reg(RF24L01_A, FLUSH_RX);               // flush RX buffer    
  
#ifdef AUTO_ACK
    SPI_RW_Reg(RF24L01_A, WRITE_REG + EN_AA, EN_AA_PIPES);  // enable Auto.Ack:Pipe0~5
    SPI_RW_Reg(RF24L01_A, WRITE_REG + FEATURE, RF_FEATURE); // enable EN_DPL, disable EN_ACK_PL
    SPI_RW_Reg(RF24L01_A, WRITE_REG + DYNPD, EN_AA_PIPES);  // enable DYNPL on Pipe0~5
    SPI_RW_Reg(RF24L01_A, WRITE_REG + SETUP_RETR, RETRY);   // timeout:5, retry:5
    SPI_RW_Reg(RF24L01_A, WRITE_REG + EN_RXADDR, 0x01);     // Enable RX Pipe0 for the ACK
#else
    SPI_RW_Reg(RF24L01_A, WRITE_REG + EN_AA, 0x0);          // disable Auto.Ack for all Pipe0~5
    SPI_RW_Reg(RF24L01_A, WRITE_REG + FEATURE, 0x0);        // disable EN_ACK_PL & EN_DPL
    SPI_RW_Reg(RF24L01_A, WRITE_REG + DYNPD, 0x0);          // disable DYNPL on Pipe0~5
    SPI_RW_Reg(RF24L01_A, WRITE_REG + SETUP_RETR, 0x0);     // no TX timeoue & retry
    SPI_RW_Reg(RF24L01_A, WRITE_REG + EN_RXADDR, 0x0);      // Disable all RX Pipe0~5
#endif
    SPI_RW_Reg(RF24L01_A, WRITE_REG + RF_CH, RF_CHANNEL);   // Select channel
    SPI_RW_Reg(RF24L01_A, WRITE_REG + RF_SETUP, RF_SETUP_V);    // TX_PWR:0dBm, Datarate:1-2Mbps, LNA:HCURR

#ifdef RF24_IRQ     
    SPI_RW_Reg(RF24L01_A, WRITE_REG + CONFIG, 0x3e); // Enable RX_DR IRQ pin, sel PWR_UP bit, enable CRC(2 bytes) & Prim:TX. RX_DR enabled..
#else
    SPI_RW_Reg(RF24L01_A, WRITE_REG + CONFIG, 0x7e); // Disable IRQ pin, set PWR_UP bit, enable CRC(2 bytes) & Prim:TX. RX_DR enabled..
#endif

#ifdef AUTO_ACK 
    // setup validation if RF module accessible
    check(RF24L01_A, EN_AA, 0x3f, 0x3f, 1);
#endif   
}

//****************************************************************************************
//
// NRF24L01p B init as PRX mode
//
//***************************************************************************************/
void init_NRF24L01_B(void)
{
//     unsigned char byte;

    // for NRF24_B
    RF24L01_B_CE_0;  // disable RF TX/RX until start TX or into RX mode
    RF24L01_B_CSN_1; // Spi disable

#ifndef _RF24_SPI_   
    RF24L01_B_SCK_0; // Spi clock line init high
#endif
    inerDelay(100);
    
    // Setup all six RX pipe Addresses & payload width
    SPI_Write_Buf(RF24L01_B, WRITE_REG + ADDR_P0, ADDR_P0_BUF, RX_ADR_WIDTH);
    SPI_Write_Buf(RF24L01_B, WRITE_REG + ADDR_P1, ADDR_P1_BUF, RX_ADR_WIDTH);
    SPI_Write_Buf(RF24L01_B, WRITE_REG + ADDR_P2, ADDR_P2_BUF, 1); // LSB only
    SPI_Write_Buf(RF24L01_B, WRITE_REG + ADDR_P3, ADDR_P3_BUF, 1); // LSB only
    SPI_Write_Buf(RF24L01_B, WRITE_REG + ADDR_P4, ADDR_P4_BUF, 1); // LSB only
    SPI_Write_Buf(RF24L01_B, WRITE_REG + ADDR_P5, ADDR_P5_BUF, 1); // LSB only
    
    // Set static rx payload width if DYN_PL not enabled
#if (RF_DYNPD == 0x00)
    SPI_RW_Reg(RF24L01_B, WRITE_REG + RX_PW_P0, RX_PL_WIDTH);
    SPI_RW_Reg(RF24L01_B, WRITE_REG + RX_PW_P1, RX_PL_WIDTH);
    SPI_RW_Reg(RF24L01_B, WRITE_REG + RX_PW_P2, RX_PL_WIDTH);
    SPI_RW_Reg(RF24L01_B, WRITE_REG + RX_PW_P3, RX_PL_WIDTH);
    SPI_RW_Reg(RF24L01_B, WRITE_REG + RX_PW_P4, RX_PL_WIDTH);
    SPI_RW_Reg(RF24L01_B, WRITE_REG + RX_PW_P5, RX_PL_WIDTH);
#endif
    /*  nRF24L01p Soft-reset sequence
     *  1)use power down mode (PWR_UP = 0) 
     *  2)clear data ready flag and data sent flag in status register 
     *  3)flush tx/rx buffer 
     */
    SPI_RW_Reg(RF24L01_B, WRITE_REG + CONFIG, 0x72);  // disable IRQ pin, clear PWR_UP bit
    SPI_RW_Reg(RF24L01_B, WRITE_REG + STATUS, 0x70);  // clear RX_DR, TX_DS, MAX_RT bits
    SPI_Write_Reg(RF24L01_B, FLUSH_TX);               // flush TX buffer    
    SPI_Write_Reg(RF24L01_B, FLUSH_RX);               // flush RX buffer    

#ifdef AUTO_ACK
    SPI_RW_Reg(RF24L01_B, WRITE_REG + EN_AA, EN_AA_PIPES);  // enable Auto.Ack for all Pipe0~5
    SPI_RW_Reg(RF24L01_B, WRITE_REG + FEATURE, RF_FEATURE); // enable EN_ACK_PL & EN_DPL
    SPI_RW_Reg(RF24L01_B, WRITE_REG + DYNPD, EN_AA_PIPES);  // enable DYNPL on all Pipe0~5
    SPI_RW_Reg(RF24L01_B, WRITE_REG + SETUP_RETR, RETRY);   // timeout and retry
#else
    SPI_RW_Reg(RF24L01_B, WRITE_REG + EN_AA, 0x0);        // disable Auto.Ack for all Pipe0~5
    SPI_RW_Reg(RF24L01_B, WRITE_REG + FEATURE, 0x0);      // disable EN_ACK_PL & EN_DPL
    SPI_RW_Reg(RF24L01_B, WRITE_REG + DYNPD, 0x0);        // disable DYNPL on Pipe0~5
    SPI_RW_Reg(RF24L01_B, WRITE_REG + SETUP_RETR, 0x0);   // no TX timeoue & retry
#endif
    SPI_RW_Reg(RF24L01_B, WRITE_REG + EN_RXADDR, EN_RX_PIPES);  // Enable RX Pipe (one or all 6 pipes)
    SPI_RW_Reg(RF24L01_B, WRITE_REG + RF_CH, RF_CHANNEL); // Select channel
    SPI_RW_Reg(RF24L01_B, WRITE_REG + RF_SETUP, RF_SETUP_V);    // TX_PWR:0dBm, Datarate:1-2Mbps, LNA:HCURR

#ifdef RF24_IRQ     
    SPI_RW_Reg(RF24L01_B, WRITE_REG + CONFIG, 0x3f); // Enable RX_DR IRQ pin, sel PWR_UP bit, enable CRC(2 bytes) & Prim:RX. RX_DR enabled..
#else
    SPI_RW_Reg(RF24L01_B, WRITE_REG + CONFIG, 0x7f); // Disable IRQ pin, set PWR_UP bit, enable CRC(2 bytes) & Prim:RX. RX_DR enabled..
#endif

            // Writes ACK data to the pipe payload will Rx    
#ifdef TX_6_PIPES
    ACK_Buf[ACK_IDX] = ++ack_cnt;
    SPI_Write_Buf(RF24L01_B, WR_ACK_PLOAD + 0, ACK_Buf, ACK_PL_WIDTH); 
    ACK_Buf[ACK_IDX] = ++ack_cnt;
    SPI_Write_Buf(RF24L01_B, WR_ACK_PLOAD + 2, ACK_Buf, ACK_PL_WIDTH); 
    ACK_Buf[ACK_IDX] = ++ack_cnt;
    SPI_Write_Buf(RF24L01_B, WR_ACK_PLOAD + 4, ACK_Buf, ACK_PL_WIDTH); 
#else
    ACK_Buf[ACK_IDX] = ++ack_cnt;
    // Writes ACK data to the pipe payload will Rx
    SPI_Write_Buf(RF24L01_B, WR_ACK_PLOAD + RX_PIPE, ACK_Buf, ACK_PL_WIDTH); 
#endif
            
#ifdef AUTO_ACK
    // setup validation if RF module accessible
    check(RF24L01_B, EN_AA, 0x3f, 0x3f, 2);    
#endif
}

/*===============================================
 *
 *  nRF24L01p A handling
 *
 *===============================================
 */
void RF_A_process(int *mode_p)
{
    static unsigned char rec_cnt = 0;     
#ifdef  AUTO_ACK    
    int pipe;
    unsigned char size;
#endif
    
    switch (*mode_p)
    {
    case 0:
      #ifndef DSP_RATE
        LED_ALL_0;
      #endif
        if (halt_led_toggle) LED1_1;
        // ready to send next packet
        // send next record
             
        sts1 = SPI_Read(RF24L01_A, READ_REG + STATUS);
        if (sts1 & ST_RX_DR) {
          *mode_p = 2;  // need read the ACK data
        } else {  
          strcpy((char *)&Tx1_Buf[1], (char *)&data[0]);
          Tx1_Buf[0] = ++rec_cnt; //just any value will do
          SPI_RW_Reg(RF24L01_A, WRITE_REG + STATUS, (ST_TX_DS | ST_MAX_RT));  //clear TX bits
          
          //
          // TX on single pipe or six pipes in turn ?
          // 
#ifdef TX_6_PIPES
          nRF24L01_TxPacket(RF24L01_A, PIPE_ADDR_LIST[tx_pipe_no++], TX_ADR_WIDTH, Tx1_Buf, TX_PL_WIDTH); // send the buffer packet
          tx_pipe_no %= 6;  // set next one
#else
          nRF24L01_TxPacket(RF24L01_A, PIPE_ADDR_LIST[RX_PIPE], TX_ADR_WIDTH, Tx1_Buf, TX_PL_WIDTH); // send the buffer packet
#endif
          reset_tm(TM_TX);   // TX KA
      
        #ifdef DSP_TX
          display(Tx1_Buf[0]);
        #endif
          *mode_p = 1;  // next step
        }

      #ifndef DSP_RATE
        LED_ALL_0;
      #endif
        break;
        
    case 1:
      #ifndef DSP_RATE
        LED_ALL_0;
      #endif
        if (halt_led_toggle) LED2_1;
        
#if 1
        sts1 = SPI_Read(RF24L01_A, READ_REG + STATUS);
        sts2 = SPI_Read(RF24L01_A, READ_REG + FIFO_STATUS);
        
        // is TX/ACK ready
        if (sts1 & ST_TX_DS) {
          // clear this status bit
          SPI_RW_Reg(RF24L01_A, WRITE_REG + STATUS, ST_TX_DS); // clear RX_DR ready flags
          *mode_p = 2; // go read ACK payload 
        } else if (sts1 & ST_MAX_RT) {
          if (++rt_cnt == 0) rt_cnt--;
          // flush tx data first
          SPI_Write_Reg(RF24L01_A, FLUSH_TX);               // flush TX buffer 
          // clear this status bit
          SPI_RW_Reg(RF24L01_A, WRITE_REG + STATUS, ST_MAX_RT); // clear MAX_RT flags
          *mode_p = 0; // drop this packet and send next packet
        } else if (get_tm(TM_TX) >= TX_TMOUT) {
          if (++to_a_cnt == 0) to_a_cnt--;  
          init_NRF24L01_A();
          *mode_p = 0; // restart TX
        }     
#else
        *mode_p = 2; // need to get ACK
#endif 
        
      #ifndef DSP_RATE
        LED_ALL_0;
      #endif
        break;

    case 2:
      #ifndef DSP_RATE
        LED_ALL_0;
      #endif
        if (halt_led_toggle) LED3_1;
        // TX done 
#ifdef AUTO_ACK
        
#ifdef DBG_STATUS
        sts1 = SPI_Read(RF24L01_A, READ_REG + STATUS);
        sts2 = SPI_Read(RF24L01_A, READ_REG + FIFO_STATUS);
#endif
        
        // check for optional ACK payload
        if ((pipe = nRF24L01_RxPacket(RF24L01_A, Rx1_Buf, &size)) != -1) 
        {
          // RX packet size validation
          if (size != ACK_PL_WIDTH) onerr(10);    
          
          // RX ACK alway from pipe #0
          if (pipe != 0) onerr(11);    
          
          // RX ACK
#ifdef DSP_ACK
          display(Rx1_Buf[ACK_IDX]);
#endif
        }
#endif 
        
        // caculate the TX packet rate 
        tx_cnt++;
        
        if (get_tm(TM_RATE) >= TM_SEC) {
            if (tx_cnt > tx_pkt_rate) { 
                tx_pkt_rate = (tx_cnt < 256) ? tx_cnt : 255;
#ifdef DSP_RATE

                // flash rate update indicator
                for (int i=0; i<LOOP; i++) {
                  display(0x55);
                }
               
                // any data rx at PRX ?
            #ifdef ENABLE_PRX
                if (has_rx) {
                  show(tx_pkt_rate);
                } else {
                  onerr(0xff);
                }
            #else
                show(tx_pkt_rate);
            #endif
#endif
            }
            tx_cnt = 0;
            reset_tm(TM_RATE);
        }
            
        *mode_p = 0;  // TX next packet
        // FALLTHRU
    default:
      #ifndef DSP_RATE
        LED_ALL_0;
      #endif
        break;
    }
}

/*===============================================
 *
 *  nRF24L01p B handling
 *
 *===============================================
 */
void RF_B_process(int *mode_p)
{
    unsigned char size;
    int pipe, cnt;
    
    switch (*mode_p)
    {
    case 0:
      #ifndef DSP_RATE
        LED_ALL_0;
      #endif
        if (halt_led_toggle) LED5_1;
        *mode_p = 1;
        reset_tm(TM_RX);   // RX KA
        
      #ifndef DSP_RATE
        LED_ALL_0;
      #endif
        break;

    case 1:
      #ifndef DSP_RATE
        LED_ALL_0;
      #endif
        if (halt_led_toggle) LED5_1;

#ifdef DBG_STATUS
        sts3 = SPI_Read(RF24L01_B, READ_REG + STATUS);
#endif  
        sts4 = SPI_Read(RF24L01_B, READ_REG + FIFO_STATUS);
        
        // RX packet from the sender ?
        if ((pipe = nRF24L01_RxPacket(RF24L01_B, Rx2_Buf, &size)) != -1) 
        {
            // RX packet size validation
            if (size != DATA_SIZE) onerr(12);;  
            
            // packet Rx
            has_rx = 1; // PRX  received data
#ifdef DSP_RX
            display(Rx2_Buf[0]);
#endif  

#ifdef ACK_PL
            sts3 = SPI_Read(RF24L01_B, READ_REG + STATUS);
            if ((sts3 & ST_TX_FULL) == 0) {
              // setup first auto-ACK if TX BIFO not full (max 3 ACK_PL)
              ACK_Buf[ACK_IDX] = ++ack_cnt;
              // Writes ACK data to the pipe payload will Rx
              SPI_Write_Buf(RF24L01_B, WR_ACK_PLOAD + pipe, ACK_Buf, ACK_PL_WIDTH); 
            }
#endif

#ifndef ENABLE_PTX
            // caculate the RX packet rate 
            rx_cnt++;
            cnt = rx_cnt;
            
            if (get_tm(TM_RATE) >= TM_SEC) {
              if (cnt > rx_pkt_rate) { 
              rx_pkt_rate = (cnt < 256) ? cnt : 255;
  #ifdef DSP_RATE

                // flash rate update indicator
                for (int i=0; i<LOOP; i++) {
                  display(0x55);
                }
               
                show(rx_pkt_rate);
  #endif
              }
              rx_cnt = 0;
              reset_tm(TM_RATE);
            }
#endif
            *mode_p = 0;
#ifdef AUTO_ACK            
        } else if (sts4 & FF_TX_EMPTY) {
            // Rreset ACK gone, TX FIFO become empty, reset mode to get ready to RX
            *mode_p = 0;
#endif            
        } else if (get_tm(TM_RX) >= RX_TMOUT) {
            // treat as KA heartbeat 
            if (++to_b_cnt == 0) to_b_cnt--;
            // is on TX_FIFO full ? (comm. dead)
            if (sts4 & FF_RX_EMPTY) {
                init_NRF24L01_B();  // soft-reset nRF24
                SetRX_Mode(RF24L01_B);  // RF B receive mode only (RF A TX mode only)
            }
            *mode_p = 0; // restart TX
        }
        // FALLTHRU
        
    default:
      #ifndef DSP_RATE
        LED_ALL_0;
      #endif
        break;
    }
}


//#####################################################
//
// LED status display debugging invoked by push botton 
//
//#####################################################
void PB_DBG(void) {
  int i;
  
    // simulate RF_A reset case
    if (KEY1_KIN) {
        halt_led_toggle = 0;
        LED_ALL_0;
        LED1_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(mode_a);      // debug info
        }
        init_NRF24L01_A();
        delay_ms(250);
        LED_ALL_0;
    }

    // simulate RF_B reset case
    if (KEY2_KIN) {
        halt_led_toggle = 0;
        LED_ALL_0;
        LED2_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(mode_b);      // debug info
        }
        init_NRF24L01_B();
        SetRX_Mode(RF24L01_B);  // RF B receive mode only (RF A TX mode only)
        mode_b = 0;
        delay_ms(250);
        LED_ALL_0;
    }
    
    if (KEY3_KIN) {
        halt_led_toggle = 1;
        LED_ALL_0;
        LED3_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(mode_a);      // debug info
        }

        LED_ALL_0;
        LED3_1;
        delay_ms(250);
        for (i=0; i<LOOP; i++) {
          display(sts1);      // debug info
        }
        
        LED_ALL_0;
        LED3_1;
        delay_ms(250);
        for (i=0; i<LOOP; i++) {
          display(sts2);      // debug info
        }
        
        LED_ALL_0;
        LED3_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(rt_cnt);      // debug info
        }

        LED_ALL_0;
        LED3_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(to_a_cnt);      // debug info
        }
        
        LED_ALL_0;
        LED3_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(tx_pkt_rate);      // debug info
        }
        
#ifdef RF24_IRQ
        LED_ALL_0;
        LED3_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(is_rf24_irq(RF24L01_A)); // debug info
        }
#endif
        
        LED_ALL_0;
    }

    if (KEY4_KIN) {
        halt_led_toggle = 1;
        LED_ALL_0;
        LED4_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(mode_b);      // debug info
        }

        LED_ALL_0;
        LED4_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(sts3);      // debug info
        }

        LED_ALL_0;
        LED4_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(sts4);      // debug info
        }

        LED_ALL_0;
        LED4_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(to_b_cnt);      // debug info
        }

#ifdef RF24_IRQ
        LED_ALL_0;
        LED4_1;
        delay_ms(250);
        // display desired debug status here
        for (i=0; i<LOOP; i++) {
          display(is_rf24_irq(RF24L01_B)); // debug info
        }
#endif
        
        LED_ALL_0;
    }
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//
//           << MAIN PROGRAM >>
//
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
main()
{    
    WDTCTL = WDTPW + WDTHOLD; //disabel watchdog timer
    
    init_led();
    init_pb();
    init_tm();
    
#ifdef _RF24_SPI_
    // connect to RF24 via SPI 3-pin mode
  #ifdef ENABLE_PTX     
    init_spi_A_master();  // SPI1 ports
  #endif

  #ifdef  ENABLE_PRX      
    init_spi_B_master();  // SPI0 ports
  #endif
    
  #ifdef RF24_IRQ
    init_rf24_irq();      // IRQ Ports
    #warning "init_rd_irq() will be called"
  #endif
#else
    // connect to RF24 via GPIO ports
    init_rf24_gpio();
#endif

    // Initialize RF24 modules    
#ifdef ENABLE_PTX     
    init_NRF24L01_A();
#endif
    
#ifdef  ENABLE_PRX         
    init_NRF24L01_B();    
    SetRX_Mode(RF24L01_B);  // RF B receive mode only (RF A TX mode only)
#endif

    while (1) {
#ifdef  ENABLE_PRX      
        RF_B_process(&mode_b);      // PRX
#endif
        
#ifdef ENABLE_PTX
        RF_A_process(&mode_a);      // PTX
#endif
        
#if 1        
        PB_DBG();   // show debug info using push buttons
#endif
    }
}

