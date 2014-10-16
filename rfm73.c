/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    rfm73.c
  * @brief   RFM73P Radio Module Library
  *          This file contains the initialization and functions for
  *          the RFM73P radio module.
  * @info    http://www.hoperf.com/rf/2.4g_module/RFM73P.htm
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "main.h"
#include "globals.h"

#include "spi.h"
#include "rfm73.h"

uint8_t rf_status = 0;

/*** Addresses ***/
const char RFM73_TX_Address[]  = { 0xE0, 0xE7, 0xE7, 0xE7, 0xE7 };
const char RFM73_RX0_Address[] = { 0xE0, 0xE7, 0xE7, 0xE7, 0xE7 };
const char RFM73_RX1_Address[] = { 0xEA, 0xE7, 0xE7, 0xE7, 0xE7 };
const char RFM73_RX2_Address   =   0xEB;
const char RFM73_RX3_Address   =   0xEC;
const char RFM73_RX4_Address   =   0xED;
const char RFM73_RX5_Address   =   0xEE;

/*** SET CE, TREN, PAEN ************************************************************/
void rfm_CE(uint8_t val) 	/* PE2 */
{
	if(val) { GPIO_SetBits(GPIOE, GPIO_Pin_2); 	 }
	else 	{ GPIO_ResetBits(GPIOE, GPIO_Pin_2); }
}
void rfm_TREN(uint8_t val)	/* PE4 */
{
	if(val) { GPIO_SetBits(GPIOE, GPIO_Pin_4); 	 }
	else 	{ GPIO_ResetBits(GPIOE, GPIO_Pin_4); }
}
void rfm_PAEN(uint8_t val)	/* PE5 */
{
	if(val) { GPIO_SetBits(GPIOE, GPIO_Pin_5); 	 }
	else 	{ GPIO_ResetBits(GPIOE, GPIO_Pin_5); }
}

/*** RFM73P INIT *******************************************************************/
uint8_t rfm_init(void)
{
	sysDms(100);	/* wait for 100ms */
	
	/* Write Bank0 registers */
	rfm_bank_select(0);	
	
	rfm_reg_write(0x00, 0x3B);		/* PRX, power up, CRC 1byte, RX_DR IT enable, other ITs disable */
	rfm_reg_write(0x01, 0x3F);		/* enable auto ack on all pipes */
	rfm_reg_write(0x02, 0x0F);		/* enable pipe 0,1,2,3 */
	rfm_reg_write(0x03, 0x03);		/* set address width: 5byte */
	rfm_reg_write(0x04, 0x1F);		/* auto retransmission: wait 500us, max. 15 retransmission */
	rfm_reg_write(0x05, 0x53);		/* select channel 2.483GHz */
	rfm_reg_write(0x06, 0x07);		/* 1Mbps, +5dBm, LNA high gain */
	rfm_reg_write(0x07, 0x07);		/* idk why */
	rfm_reg_write(0x08, 0x00);		/* reserved */
	rfm_reg_write(0x09, 0x00);		/* reserved */
	rfm_reg_write(0x0C, RFM73_RX2_Address);		/* RX2 Address */
	rfm_reg_write(0x0D, RFM73_RX3_Address);		/* RX3 Address */
	rfm_reg_write(0x0E, RFM73_RX4_Address);		/* RX4 Address */
	rfm_reg_write(0x0F, RFM73_RX5_Address);		/* RX5 Address */
	rfm_reg_write(0x11, 0x20);		/* 32 bytes in RX payload in pipe 0 */
	rfm_reg_write(0x12, 0x20);		/* 32 bytes in RX payload in pipe 1 */
	rfm_reg_write(0x13, 0x20);		/* 32 bytes in RX payload in pipe 2 */
	rfm_reg_write(0x14, 0x20);		/* 32 bytes in RX payload in pipe 3 */
	rfm_reg_write(0x15, 0x20);		/* 32 bytes in RX payload in pipe 4 */
	rfm_reg_write(0x16, 0x20);		/* 32 bytes in RX payload in pipe 5 */
	rfm_reg_write(0x17, 0x00);		/* fifo status */
	
	
	/* Set Rx, Tx Addresses */
	rfm_buffer_write(RFM73_REG_TX_ADDR | RFM73_CMD_W_REGISTER, RFM73_TX_Address, 5);
	rfm_buffer_write(RFM73_REG_RX_ADDR_P0 | RFM73_CMD_W_REGISTER, RFM73_RX0_Address, 5);
	rfm_buffer_write(RFM73_REG_RX_ADDR_P1 | RFM73_CMD_W_REGISTER, RFM73_RX1_Address, 5);
	//rfm_reg_write(RFM73_REG_RX_ADDR_P2, RFM73_RX2_Address);
	//rfm_reg_write(RFM73_REG_RX_ADDR_P3, RFM73_RX3_Address);
	
	/* Activate Extra Features */
	uint8_t feature = rfm_reg_read(0x1D);
	if(feature == 0) { rfm_cmd_write(RFM73_CMD_ACTIVATE, 0x73); } 
	
	/* Enable Dynamic Payload Length, with ACK, W_TX_PAYLOAD_NOACK */
	rfm_reg_write(0x1D, 0x07);
	
	/* Enable Dynamic Payload Length on pipe 0-5 */
	rfm_reg_write(0x1C, 0x3F);

	/* Switch Bank */
	rfm_bank_select(1);
	
	/* Write Bank1 registers */
	rfm_bank1_write_0to8(0x00, 0x404B01E2);
	rfm_bank1_write_0to8(0x01, 0xC04B0000);
	rfm_bank1_write_0to8(0x02, 0xD0FC8C02);
	rfm_bank1_write_0to8(0x03, 0x99003941);
	rfm_bank1_write_0to8(0x04, 0xD996821B);
	rfm_bank1_write_0to8(0x05, 0x24067FA6);
	rfm_bank1_write_0to8(0x06, 0x00000000);
	rfm_bank1_write_0to8(0x07, 0x00000000);
	
	rfm_bank1_write_9to13(0x0C, 0x00731200);
	rfm_bank1_write_9to13(0x0D, 0x0080B446);
	rfm_bank1_write_14();
	
	/* Toggle Reg4<25,26> */
	char bank1_reg4[] = {0x1B, 0x82, 0x96, 0xD9 };
	bank1_reg4[0] |= 0x06;
	rfm_buffer_write(0x04 | RFM73_CMD_W_REGISTER, bank1_reg4, 4);
	bank1_reg4[0] &= 0xF9;
	rfm_buffer_write(0x04 | RFM73_CMD_W_REGISTER, bank1_reg4, 4);
	
	sysDms(50);
	
	/* Switch Bank back to 0 */
	rfm_bank_select(0);
	
	/* Set Receive Mode */
	rfm_mode_rx();
	sysDms(100);
	
	/* Check init success (PWR_UP bit at reg address 0x00 */
	uint8_t val = rfm_reg_read(0x00);
	if(val & 0x02) { return 1; }
	return 0;
}

/*** CHECK FIFOS *******************************************************************/
uint8_t rfm_txfifo_empty(void)
{
	uint8_t fifo = rfm_reg_read(0x17);
	return (fifo & (1<<4)) ? 1 : 0;
}

uint8_t rfm_rxfifo_empty(void)
{
	uint8_t fifo = rfm_reg_read(0x17);
	return (fifo & 0x01) ? 1 : 0;
}

/*** SET RX/TX MODE ****************************************************************/
void rfm_mode_rx(void)
{
	uint8_t value = 0;
	
	rfm_TREN(0);
	rfm_PAEN(1);
	
	/* flush rx queue */
	rfm_cmd_write(RFM73_CMD_FLUSH_RX, 0);
	
	/* clear interrupt: read status and write back */
	value = rfm_reg_read(RFM73_REG_STATUS);
	rfm_reg_write(RFM73_REG_STATUS, value);
	
	rfm_CE(0);
	
	value = rfm_reg_read(0x00);
	value |= 0x03;					/* PRX and PWR_UP */
	rfm_reg_write(0x00, value);
	
	sysDms(1);
	rfm_CE(1);
}

void rfm_mode_tx(void)
{
	uint8_t value = 0;
	
	rfm_TREN(1);
	rfm_PAEN(1);
	
	/* flush rx queue */
	rfm_cmd_write(RFM73_CMD_FLUSH_TX, 0);
	
	/* clear interrupt: read status and write back */
	value = rfm_reg_read(RFM73_REG_STATUS);
	rfm_reg_write(RFM73_REG_STATUS, value);
	
	rfm_CE(0);
	
	value = rfm_reg_read(0x00);
	value &= 0xFE;		/* PTX and PWR_UP */
	rfm_reg_write(0x00, value);
	
	sysDms(1);
	rfm_CE(1);
	sysDms(50);
}

/*** CHECK RX **********************************************************************/
uint8_t rfm_check_rx(void)
{
	uint8_t val = rfm_reg_read(RFM73_REG_STATUS);
	if(val & (1<<6)) { return 1; }
	return 0;
}

/*** SET RX_P0 AND TX ADDRESS ******************************************************/
void rfm_set_addr(char addr[])
{
	rfm_CE(0);
	rfm_buffer_write(RFM73_REG_RX_ADDR_P0 | RFM73_CMD_W_REGISTER, 	addr, 5);	
	rfm_buffer_write(RFM73_REG_TX_ADDR | RFM73_CMD_W_REGISTER, 	addr, 5);
	sysDms(1);
	rfm_CE(1);
	
	sysDms(50);
}

/*** SEND DATA *********************************************************************/
void rfm_send(const char data[], const uint8_t length)
{
	/* check if MAX_RT is asserted. if yes we must clear the bit to enable communication */
	uint8_t rfval = rfm_reg_read(RFM73_REG_STATUS);
	if(rfval & (1<<4))
	{
		rfm_reg_write(RFM73_REG_STATUS, rfval);
		RF_spi_select();
		RF_spi_send(RFM73_CMD_FLUSH_TX);
		RF_spi_deselect();
	}
	
	/* send data */
	if(rfm_txfifo_empty())
	{
		rfm_buffer_write( RFM73_CMD_W_TX_PAYLOAD, data, length );
	}
	
	/* Wait for ACK */
	sysDms(20);
}

/*** RECEIVE DATA ******************************************************************/
void rfm_rec(char data[], uint8_t* length)
{
	uint8_t rfval = rfm_reg_read(RFM73_REG_STATUS);
	rfm_rxfifo_read(RFM73_CMD_R_RX_PAYLOAD, data, length);
	rfm_reg_write(RFM73_REG_STATUS, rfval);
}

/*** BUFFER WRITE ******************************************************************/
uint8_t rfm_reg_buffer_write(const char addr, const char data[], const uint8_t length)
{
	return rfm_buffer_write( (addr | RFM73_CMD_W_REGISTER), data, length );
}

uint8_t rfm_buffer_write(const char addr, const char data[], const uint8_t length)
{
	/* write min. 1byte and max. 32byte */
	if(!length || length > 32) { return 0; }
	
	RF_spi_select();	
	rf_status = RF_spi_send(addr);
	
	uint8_t i = 0;
	for(i=0; i<length; ++i)
	{
		RF_spi_send(data[i]);
	}
	
	RF_spi_deselect();
	return 1;
}

/*** BUFFER READ *******************************************************************/
uint8_t rfm_buffer_read(const char addr, char data[], uint8_t length)
{
	uint8_t i = 0;
	
	RF_spi_select();
	RF_spi_send(addr);
	for(i=0; i<(length); ++i)
	{
		data[i] = RF_spi_send(0);
	}	
	RF_spi_deselect();
	
	return 0;
}

/*** RX FIFO READ ******************************************************************/
uint8_t rfm_rxfifo_read(const char addr, char data[], uint8_t* length)
{
	uint8_t i = 0;	
	*length = rfm_reg_read(RFM73_CMD_R_RX_PL_WID);
	
	RF_spi_select();
	RF_spi_send(addr);
	for(i=0; i<(*length); ++i)
	{
		data[i] = RF_spi_send(0);
	}	
	RF_spi_deselect();
	
	return 0;
}

/*** REGISTER BANK SELECT **********************************************************/
void rfm_bank_select(uint8_t bank)
{
	uint8_t status = (rfm_reg_read(RFM73_REG_STATUS)) & 0x80;
	if( (status && bank == 0) || (status == 0 && bank == 1) )
	{
		rfm_cmd_write(RFM73_CMD_ACTIVATE, 0x53);
	}
	sysDms(5);
}

/*** CMD AND REGISTER READ/WRITE ***************************************************/
void rfm_cmd_write(char cmd, char data)
{
	RF_spi_select();
	rf_status = RF_spi_send(cmd);
	RF_spi_send(data);
	RF_spi_deselect();
}
void rfm_reg_write(char address, char data)
{	
	address |= RFM73_CMD_W_REGISTER;	/* set bit 5 (in 0-7 scale) to 1 in command */
	
	RF_spi_select();
	rf_status = RF_spi_send(address);
	RF_spi_send(data);
	RF_spi_deselect();
}

char rfm_reg_read(char address)
{
	char value = 0;
	
	RF_spi_select();
	rf_status = RF_spi_send(address);
	value = RF_spi_send(0x00);
	RF_spi_deselect();
	
	return value;
}

/*** REGISTER BANK 1 WRITE *********************************************************/
void rfm_bank1_write_0to8(uint8_t address, uint32_t val)	/* MSB byte to LSB byte */
{
	uint8_t i, byte = 0;
	RF_spi_select();
	
	//address
	address |= RFM73_CMD_W_REGISTER;
	RF_spi_send(address);
	
	//value
	for(i=0; i<4; ++i)
	{
		byte = (val >> (8*(3-i))) & 0xFF;
		RF_spi_send(byte);
	}
	RF_spi_deselect();
}

void rfm_bank1_write_9to13(uint8_t address, uint32_t val)	/* LSB byte to MSB byte */
{
	uint8_t i, byte = 0;
	RF_spi_select();
	
	//address
	address |= RFM73_CMD_W_REGISTER;
	RF_spi_send(address);
	
	//value
	for(i=0; i<4; ++i)
	{
		byte = (val >> (8*i)) & 0xFF;
		RF_spi_send(byte);
	}
	RF_spi_deselect();
}

void rfm_bank1_write_14(void)	/* LSB to MSB, default RAMP value: 0x FF FF FE F7 CF 20 81 04 08 20 41 */
{
	uint8_t i, byte = 0;
	uint8_t address = 0x0E;
	uint32_t lower  = 0x04082041;
	uint32_t mid    = 0xF7CF2081;
	uint32_t upper  = 0x00FFFFFE;
		
	RF_spi_select();
	
	//address
	address |= RFM73_CMD_W_REGISTER;
	RF_spi_send(address);
	
	//value lower
	for(i=0; i<4; ++i)
	{
		byte = (lower >> (8*i)) & 0xFF;
		RF_spi_send(byte);
	}
	
	//value mid
	for(i=0; i<4; ++i)
	{
		byte = (mid >> (8*i)) & 0xFF;
		RF_spi_send(byte);
	}
	
	//value upper
	for(i=0; i<3; ++i)
	{
		byte = (upper >> (8*i)) & 0xFF;
		RF_spi_send(byte);
	}
	RF_spi_deselect();
}
