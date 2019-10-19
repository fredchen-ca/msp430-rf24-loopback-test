/*
 * << nRF24L01p Transceiver Module Access Control Library >>
 * Can be access via
 * - GPIO
 * - SPI w/o IRQ 
 * - SPI w/IRQ 
 */
#include "../device_lib/rf24_lib.h"

/************************************************** 
Function: SPI_Read(); 
 
Description: 
  Read one byte from nRF24L01 register, 'reg'  

input:
  nrf24: nRF24L01P module - 0/1: A/B

 **************************************************/
unsigned char SPI_Read(int nrf24, unsigned char reg)
{
    unsigned char reg_val;
    
    nrf24 ? RF24L01_B_CSN_0 : RF24L01_A_CSN_0; // CSN low, initialize SPI communication...
    SPI_RW(nrf24, reg); // Select register to read from..
    reg_val = SPI_RW(nrf24, NOP); // ..then read registervalue
    nrf24 ? RF24L01_B_CSN_1 : RF24L01_A_CSN_1; // CSN high, terminate SPI communication
    
    return (reg_val); //  return register value
}

/************************************************** 
Function: SPI_RW_Reg(); 
 
Description: 
  send one byte register 'reg' instruction w/o parameter 

input:
  nrf24: nRF24L01P module - 0/1: A/B

 **************************************************/
unsigned char SPI_Write_Reg(int nrf24, unsigned char reg)
{
    unsigned char status;
    
    nrf24 ? RF24L01_B_CSN_0 : RF24L01_A_CSN_0; // CSN low, init SPI transaction
    status = SPI_RW(nrf24, reg); // select register
    nrf24 ? RF24L01_B_CSN_1 : RF24L01_A_CSN_1; // CSN high again
    
    return (status); // return nRF24L01 status uchar
}

/************************************************** 
Function: SPI_RW_Reg(); 
 
Description: 
  Writes value 'value' to register 'reg'

input:
  nrf24: nRF24L01P module - 0/1: A/B

 **************************************************/
unsigned char SPI_RW_Reg(int nrf24, unsigned char reg, unsigned char value)
{
    unsigned char status;
    
    nrf24 ? RF24L01_B_CSN_0 : RF24L01_A_CSN_0; // CSN low, init SPI transaction
    status = SPI_RW(nrf24, reg); // select register
    SPI_RW(nrf24, value); // ..and write value to it..
    nrf24 ? RF24L01_B_CSN_1 : RF24L01_A_CSN_1; // CSN high again
    
    return (status); // return nRF24L01 status uchar
}

/************************************************** 
Function: SPI_Read_Buf(); 
 
Description: 
  Reads 'bytes' #of bytes from register 'reg' 
  Typically used to read RX payload, Rx/Tx address 

input:
  nrf24: nRF24L01P module - 0/1: A/B

 **************************************************/
unsigned char SPI_Read_Buf(int nrf24, unsigned char reg, unsigned char* pBuf, unsigned char chars)
{
    unsigned char status, uchar_ctr;
    
    nrf24 ? RF24L01_B_CSN_0 : RF24L01_A_CSN_0; // Set CSN low, init SPI tranaction
    status = SPI_RW(nrf24,reg); // Select register to write to and read status uchar
    for (uchar_ctr = 0; uchar_ctr < chars; uchar_ctr++) {
        pBuf[uchar_ctr] = SPI_RW(nrf24,0); //
    }
    nrf24 ? RF24L01_B_CSN_1 : RF24L01_A_CSN_1; // Set CSN high
    return (status); // return nRF24L01 status uchar
}

/************************************************** 
Function: SPI_Write_Buf(); 
 
Description: 
  Writes 'bytes' from pBuff to register 'chars' 
  Typically used to write TX payload, Rx/Tx address */
/**************************************************/
unsigned char SPI_Write_Buf(int nrf24, unsigned char reg, unsigned char* pBuf, unsigned char chars)
{
    unsigned char status, uchar_ctr;
    
    nrf24 ? RF24L01_B_CSN_0 : RF24L01_A_CSN_0; // Set CSN low, init SPI tranaction
    status = SPI_RW(nrf24,reg); // Select register to write to and read status byte
    for (uchar_ctr = 0; uchar_ctr < chars; uchar_ctr++) // then write all byte in buffer(*pBuf)
    {
        SPI_RW(nrf24, *pBuf++);
    }
    nrf24 ? RF24L01_B_CSN_1 : RF24L01_A_CSN_1; // Set CSN high
    return (status); 
}

//****************************************************************************************************/
// void SetRX_Mode(void) w/Auto-ACK enabled
//****************************************************************************************************/
void SetRX_Mode(int nrf24)
{
    nrf24 ? RF24L01_B_CE_0 : RF24L01_A_CE_0;    // stop radio TX/RX transmission

#ifdef RF24_IRQ     
    SPI_RW_Reg(nrf24, WRITE_REG + CONFIG, 0x3f); // Enable IRQ pin, sel PWR_UP bit, enable CRC(2 bytes) & Prim:RX. RX_DR enabled..
#else
    SPI_RW_Reg(nrf24, WRITE_REG + CONFIG, 0x7f); // Disable IRQ pin, set PWR_UP bit, enable CRC(2 bytes) & Prim:RX. RX_DR enabled..
#endif
    
    nrf24 ? RF24L01_B_CE_1 : RF24L01_A_CE_1; // Set CE pin high to enable radio TX/RX transmission
    inerDelay(200); // wait for RX ready
}

//******************************************************************************************************/
// unsigned unsigned char nRF24L01_RxPacket(unsigned unsigned char* rx_buf)
// RX a packet
// return:
// -1: none
// 0~5: pipe of Rx data
//******************************************************************************************************/
int nRF24L01_RxPacket(int nrf24, unsigned char* rx_buf, unsigned char *size)
{
    unsigned char status;    
    
    *size = 0;
    
#ifdef RF24_IRQ
    // RF24 IRQ ready ?
    if (!is_rf24_irq(nrf24)) return -1;
#endif
        
    if ((status = SPI_Read(nrf24, READ_REG + STATUS)) & ST_RX_DR)
    {
        *size = SPI_Read(nrf24, RD_RX_PL_WID); 
        SPI_Read_Buf(nrf24, RD_RX_PLOAD, rx_buf, *size); // read receive payload from RX_FIFO buffer
        SPI_RW_Reg(nrf24, WRITE_REG + STATUS, ST_RX_DR); // clear RX DR ready flags
        return ((status & ST_RX_P_NO)>>1);  // return pipe # data received
    } else {
        return -1; // no data received
    }
}

//***********************************************************************************************************
//void nRF24L01_TxPacket(unsigned char * tx_buf)
//TX a packet for PTX mode
//**********************************************************************************************************/
void nRF24L01_TxPacket(int nrf24, unsigned char* tx_addr, int addr_len, unsigned char* tx_buf, int buf_size)
{
    nrf24 ? RF24L01_B_CE_0 : RF24L01_A_CE_0; // disble RF TX/RX
    SPI_Write_Buf(nrf24, WRITE_REG + TX_ADDR, tx_addr, addr_len); // Writes destination TX_Address to nRF24L01

#ifdef AUTO_ACK    
    SPI_Write_Buf(nrf24, WRITE_REG + ADDR_P0, tx_addr, addr_len); // Writes RX_Addr0 same as TX_Adr for Auto.Ack
#endif
    
    SPI_Write_Buf(nrf24, WR_TX_PLOAD, tx_buf, buf_size); // Writes data to TX payload
    
#ifdef RF24_IRQ     
    SPI_RW_Reg(nrf24, WRITE_REG + CONFIG, 0x3e); // Enable RX_DR IRQ pin, sel PWR_UP bit, enable CRC(2 bytes) & Prim:TX..
#else
    SPI_RW_Reg(nrf24, WRITE_REG + CONFIG, 0x7e); // Disable IRQ pin, set PWR_UP bit, enable CRC(2 bytes) & Prim:TX..
#endif
    
    nrf24 ? RF24L01_B_CE_1 : RF24L01_A_CE_1; // enable RF TX/RX
}
