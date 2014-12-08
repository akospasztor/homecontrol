/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    spi.c
  * @brief   SPI functions
  *          This file contains the GPIO initializations and functions
  *          for SPI peripherials.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "main.h"
#include "spi.h"

/*** SPI INIT **********************************************************************/
void SPI_init(void)
{	
    /* FT800 pin configuration
	 * SCK  = PA5
	 * MISO = PA6
	 * MOSI = PB5
	 * CS   = PA4
	 * PD   = PE8
     */
    
    SPI_InitTypeDef SPI_InitTypeDefStruct;
    GPIO_InitTypeDef GPIO_InitTypeDefStruct;
    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
     
    SPI_InitTypeDefStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitTypeDefStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitTypeDefStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitTypeDefStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitTypeDefStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitTypeDefStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitTypeDefStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    SPI_InitTypeDefStruct.SPI_FirstBit = SPI_FirstBit_MSB;
     
    SPI_Init(SPI1, &SPI_InitTypeDefStruct);
    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB , ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE , ENABLE);
    
    //SCK, MISO
    GPIO_InitTypeDefStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitTypeDefStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitTypeDefStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitTypeDefStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitTypeDefStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitTypeDefStruct);
	
	//MOSI
    GPIO_InitTypeDefStruct.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitTypeDefStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitTypeDefStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitTypeDefStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitTypeDefStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitTypeDefStruct);
    
    //CS
    GPIO_InitTypeDefStruct.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitTypeDefStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitTypeDefStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitTypeDefStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitTypeDefStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitTypeDefStruct);
    
    //PD
    GPIO_InitTypeDefStruct.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitTypeDefStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitTypeDefStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitTypeDefStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitTypeDefStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOE, &GPIO_InitTypeDefStruct);
     
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);
    
	//CS, PD up
    GPIO_SetBits(GPIOE, GPIO_Pin_8);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
    
    SPI_Cmd(SPI1, ENABLE);
}

void SPI_speedup(void)
{
    SPI_InitTypeDef SPI_InitTypeDefStruct;
     
    SPI_InitTypeDefStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitTypeDefStruct.SPI_Mode = SPI_Mode_Master;
    SPI_InitTypeDefStruct.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitTypeDefStruct.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitTypeDefStruct.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitTypeDefStruct.SPI_NSS = SPI_NSS_Soft;
    SPI_InitTypeDefStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
    
    SPI_InitTypeDefStruct.SPI_FirstBit = SPI_FirstBit_MSB;
     
    SPI_Init(SPI1, &SPI_InitTypeDefStruct);
}

/*** SEND **************************************************************************/
char SPI_send(char data)
{    
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
    SPI_I2S_SendData(SPI1, data);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    
    return SPI_I2S_ReceiveData(SPI1);
}

/*** REC ***************************************************************************/
char SPI_rec(char address)
{      
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
    SPI_I2S_SendData(SPI1, address);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
    SPI_I2S_ReceiveData(SPI1);
     
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE)); 
    SPI_I2S_SendData(SPI1, 0x00);
    while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE));
     
    return  SPI_I2S_ReceiveData(SPI1);
}

/*** FT800 SPI select **************************************************************/
void FT_spi_select(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}

/*** FT800 SPI deselect ************************************************************/
void FT_spi_deselect(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_4);
}


/***********************************************************************************/
/*** RFM73P Radio Module ***********************************************************/
/***********************************************************************************/
void RF_spi_init(unsigned int prescaler)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	SPI_InitTypeDef SPI_InitStruct;
	
	/* configure pins used by SPI2
	 * PB10 = SCK		(AF)
	 * PB14 = MISO		(AF)
	 * PB15 = MOSI		(AF)
	 * PB8  = SS_RF		(OUT)
	 * PE2  = CE_RF		(OUT)
	 * PE3  = IRQ_RF  	(IN)
	 * PE4  = TREN_RF	(OUT)
	 * PE5  = PAEN_RF	(OUT)
	 */
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	
	// SPI2 GPIO Configuration
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
	// connect SPI2 pins to SPI alternate function
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
	
	// RF GPIO Configuration
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;	
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStruct);	
	
	// RF IRQ Pin Configuration
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &GPIO_InitStruct);	
	
	// EXTI Configuration
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource3);
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
	
	// Default pin states
	GPIO_SetBits(GPIOB, GPIO_Pin_8);		//SPI SS
	GPIO_ResetBits(GPIOE, GPIO_Pin_2);		//RF CE
	GPIO_ResetBits(GPIOE, GPIO_Pin_4);		//RF TREN
	GPIO_ResetBits(GPIOE, GPIO_Pin_5);		//RF PAEN
	
	// Enable SPI2 clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;     					// transmit in master mode, NSS pin has to be always high
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; 					// one packet of data is 8 bits wide
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;        					// clock is low when idle
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;      					// data sampled at first edge
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft; 							// set the NSS management to software
	SPI_InitStruct.SPI_BaudRatePrescaler = prescaler;				// SPI frequency is APB2 frequency / prescaler
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;					// data is transmitted MSB first
	SPI_Init(SPI2, &SPI_InitStruct); 
	
	SPI_Cmd(SPI2, ENABLE);
}

void RF_spi_select(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_8);
}

void RF_spi_deselect(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_8);
}

char RF_spi_send(char data)
{
	while(!SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE)); 
    SPI_I2S_SendData(SPI2, data);
	
    while(!SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE));
    return SPI_I2S_ReceiveData(SPI2);
}


/***********************************************************************************/
/*** SD Card ***********************************************************************/
/***********************************************************************************/
void SD_init(unsigned int prescaler)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	SPI_InitTypeDef SPI_InitStruct;
	
	// enable clock for used IO pins
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	
	/* configure pins used by SPI2
	 * PB10 = SCK
	 * PB14 = MISO
	 * PB15 = MOSI
	 * PD15 = SS_SD
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	// chip-select
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStruct);
	GPIO_SetBits(GPIOD, GPIO_Pin_15);
	
	// connect SPI2 pins to SPI alternate function
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
	
	// enable peripheral clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	
	SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // set to full duplex mode, seperate MOSI and MISO lines
	SPI_InitStruct.SPI_Mode = SPI_Mode_Master;     					// transmit in master mode, NSS pin has to be always high
	SPI_InitStruct.SPI_DataSize = SPI_DataSize_8b; 					// one packet of data is 8 bits wide
	SPI_InitStruct.SPI_CPOL = SPI_CPOL_Low;        					// clock is low when idle
	SPI_InitStruct.SPI_CPHA = SPI_CPHA_1Edge;      					// data sampled at first edge
	SPI_InitStruct.SPI_NSS = SPI_NSS_Soft; 							// set the NSS management to internal and pull internal NSS high
	SPI_InitStruct.SPI_BaudRatePrescaler = prescaler;				// SPI frequency: APB1
	SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;					// data is transmitted MSB first
	SPI_Init(SPI2, &SPI_InitStruct); 
	
	SPI_Cmd(SPI2, ENABLE);
}

unsigned char SD_send_single(unsigned char data)
{	
	SPI2->DR = data; 							// write data to be transmitted to the SPI data register
	while( !(SPI2->SR & SPI_I2S_FLAG_TXE) ); 	// wait until transmit complete
	while( !(SPI2->SR & SPI_I2S_FLAG_RXNE) ); 	// wait until receive complete
	while( SPI2->SR & SPI_I2S_FLAG_BSY ); 		// wait until SPI is not busy anymore
	return SPI2->DR; 							// return received data from SPI data register	
}

unsigned char SD_receive_single(void)
{
	SPI2->DR = 0xFF; 							// write data to be transmitted to the SPI data register
	while( !(SPI2->SR & SPI_I2S_FLAG_TXE) ); 	// wait until transmit complete
	while( !(SPI2->SR & SPI_I2S_FLAG_RXNE) ); 	// wait until receive complete
	while( SPI2->SR & SPI_I2S_FLAG_BSY ); 		// wait until SPI is not busy anymore
	return SPI2->DR; 							// return received data from SPI data register
}

void SD_send(unsigned char* data, unsigned int length)
{
	while (length--)
	{
		SD_send_single(*data);
		data++;
	}
}

void SD_receive(unsigned char* data, unsigned int length)
{
	while (length--)
	{
		*data = SD_receive_single();
		data++;
	}
}

void SD_transmit(unsigned char* txbuf, unsigned char* rxbuf, unsigned int len)
{
	while (len--)
	{
		SPI2->DR = *txbuf; 							// write data to be transmitted to the SPI data register
		while( !(SPI2->SR & SPI_I2S_FLAG_TXE) ); 	// wait until transmit complete
		while( !(SPI2->SR & SPI_I2S_FLAG_RXNE) ); 	// wait until receive complete
		while( SPI2->SR & SPI_I2S_FLAG_BSY ); 		// wait until SPI is not busy anymore
		*rxbuf = SPI2->DR; 							// return received data from SPI data register
		txbuf++;
		rxbuf++;	 
	}
}
