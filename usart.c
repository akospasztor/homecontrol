/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    usart.c
  * @brief   USART functions
  *          This file contains the GPIO initializations and functions
  *          for USART peripherials.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "usart.h"

/*** BLUETOOTH USART INIT **********************************************************/
void initBT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;	
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	/* USART2 GPIO Config (uC side)
	 * PD3 - CTS
	 * PD4 - RTS
	 * PD5 - TX
	 * PD6 - RX
	 */
	
    /* TX, RX: PullUp */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/* CTS, RTS: NoPull */
	//GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3 | GPIO_Pin_4;
	//GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	//GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	//GPIO_PinAFConfig(GPIOD, GPIO_PinSource3, GPIO_AF_USART2);
	//GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);
	
	/* USART2 Config */
	USART_InitStructure.USART_BaudRate   = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;
	USART_InitStructure.USART_Parity     = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	//USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
	USART_InitStructure.USART_Mode       =  USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);
	
	/* USART2 RX Interrupt Config */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);		
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	 	
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART2, ENABLE);
}

/*** BLUETOOTH USART SEND **********************************************************/
void BT_send_char(uint8_t c)
{
   while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
   USART_SendData(USART2, c);
}

void BT_send_str(unsigned char* str)
{
	uint8_t i = 0;
	for(i=0; str[i] != '\0'; ++i)
	{
		BT_send_char(str[i]);
	}
}

void BT_send_cmd(unsigned char* str)
{
	BT_send_str(str);
	BT_send_char(0x0D);
}


/*** KNX USART INIT ****************************************************************/
void initKNX(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;	
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* USART3 GPIO Config (uC side)
	 * PD8  - TX
	 * PD9  - RX
	   PD10 - RXEN
	   PD11 - TXEN
	   PD12 - SelectPlugin (0: KNX | 1: RS-485)
	 */
	
    /* TX, RX: PullUp */
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/* TXEN, RXEN pins*/
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOD, GPIO_Pin_10);				/* /RXEN */
	GPIO_ResetBits(GPIOD, GPIO_Pin_11);				/* TXEN */
	
	/* SelectPlugin pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	/* RX, TX as AF */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);
	
	/* USART3 Config */
	USART_InitStructure.USART_BaudRate   = 19200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits   = USART_StopBits_1;
	USART_InitStructure.USART_Parity     = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode       =  USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3, &USART_InitStructure);
	
	/* USART3 RX Interrupt Config */
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);		
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	 	
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART3, ENABLE);
}

/*** KNX USART SEND ****************************************************************/
void KNX_send_char(uint8_t c)
{
	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
	USART_SendData(USART3, c);
}

void KNX_send_str(char* str)
{
	uint8_t i = 0;
	for(i=0; str[i] != '\0'; ++i)
	{
		KNX_send_char(str[i]);
	}
}

/*** RS-485 ************************************************************************/
void txEN485(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_11 | GPIO_Pin_10);
}

void rxEN485(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_10 | GPIO_Pin_11);
}
