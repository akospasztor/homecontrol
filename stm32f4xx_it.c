/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines
  *          This file contains the exception handlers and peripherial
  *          interrupt handler routines.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/ 


/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "stm32f4xx.h"
#include "main.h"
#include "globals.h"

#include "rfm73.h"

#include "usbd_cdc_core.h"
#include "usb_dcd_int.h"
#include "usbd_cdc_vcp.h"

/* Private variables --------------------------------------------------------------*/	  
uint16_t sysTimer = 0;			/* System Timer Counter (ms)  */
uint8_t  sysTimerSec = 0;		/* System Timer Counter (sec) */
uint8_t  sysTimerMin = 0;		/* System Timer Counter (min) */
	  
/* Global variables ---------------------------------------------------------------*/
extern __ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

extern uint32_t sys_delay;

extern RTC_TimeTypeDef Rtime;
extern RTC_DateTypeDef Rdate;

extern uint8_t rxBTchar;
extern uint8_t rxKNXchar;

extern ModuleRadioStruct ModuleRadio;

extern SensorStruct Sensor1;
extern SensorStruct Sensor2;
extern SensorStruct Sensor3;

/* Global flags -------------------------------------------------------------------*/
extern uint8_t  FLAG_rfm_rx;
extern uint8_t 	FLAG_datetime;
extern uint8_t  FLAG_settingDatetime;
extern uint8_t 	FLAG_getPeriphConnection;
extern uint8_t  FLAG_pluginType;
extern uint8_t  FLAG_inputIndicator;
extern uint16_t FLAG_usbConnection;
extern uint8_t	FLAG_dispAlert;
extern uint8_t  FLAG_beepAlert;

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	if(sys_delay!=0)
	{
		sys_delay--;
	}
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/*** System Timer Interrupt Handler ***/
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update))
	{
		/* Clear IT */
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		
		/* LED: hb */
		if(sysTimer < 50) { ledOn(LEDhb); }
		else { ledOff(LEDhb); }		
		
		/* Get date & time */
		if(sysTimer == 10)
		{
			RTC_GetDate(RTC_Format_BIN, &Rdate);
			RTC_GetTime(RTC_Format_BIN, &Rtime);
			FLAG_datetime = 1;
		}
		
		/* Get connected peripherials status */
		if(sysTimer == 20)
		{
			FLAG_getPeriphConnection = 1;
		}
		
		/* Get RS-485 | KNX Plugin Type */
		if(sysTimer == 30)
		{
			FLAG_pluginType = 1;
		}
		
		/* Polling Sensor1 */
		if(sysTimerSec == 0 &&  Sensor1.FLAG_poll == POLL_WAIT && !sysTimer)	{ if(Sensor1.errcount < 255) { Sensor1.errcount++; } }
		if(sysTimerSec  > 1 &&  Sensor1.FLAG_poll == POLL_RDY)					{ Sensor1.FLAG_poll = 0; }
		if(sysTimerSec == 1 &&  Sensor1.com && !sysTimer)
		{
			if( (!Sensor1.FLAG_poll) ||
			    ( Sensor1.FLAG_poll == POLL_WAIT && Sensor1.errcount > 1) )
			{ 
				Sensor1.FLAG_poll = 1;
			}
		}
		
		/* Polling Sensor2 */
		if(sysTimerSec == 4 &&  Sensor2.FLAG_poll == POLL_WAIT && !sysTimer)	{ if(Sensor2.errcount < 255) { Sensor2.errcount++; } }		
		if(sysTimerSec  > 5 &&  Sensor2.FLAG_poll == POLL_RDY)					{ Sensor2.FLAG_poll = 0; }
		if(sysTimerSec == 5 &&  Sensor2.com && !sysTimer)
		{
			if( (!Sensor2.FLAG_poll) ||
			    ( Sensor2.FLAG_poll == POLL_WAIT && Sensor2.errcount > 1) )
			{ 
				Sensor2.FLAG_poll = 1;
			}
		}
		
		/* Polling Sensor3 */
		if(sysTimerSec == 8 &&  Sensor3.FLAG_poll == POLL_WAIT && !sysTimer)	{ if(Sensor3.errcount < 255) { Sensor3.errcount++; } }
		if(sysTimerSec  > 9 &&  Sensor3.FLAG_poll == POLL_RDY)					{ Sensor3.FLAG_poll = 0; }
		if(sysTimerSec == 9 &&  Sensor3.com && !sysTimer)
		{
			if( (!Sensor3.FLAG_poll) ||
			    ( Sensor3.FLAG_poll == POLL_WAIT && Sensor3.errcount > 1) )
			{ 
				Sensor3.FLAG_poll = 1;
			}
		}
		
		/* Clear Sensor:alert Bit0 when alertTimer reaches 0 (only when there is no need to display alert) */
		if(!Sensor1.alertTimer && !isAlert(&Sensor1) && !FLAG_dispAlert) { clrAlertStateBit(&Sensor1); }
		if(!Sensor2.alertTimer && !isAlert(&Sensor2) && !FLAG_dispAlert) { clrAlertStateBit(&Sensor2); }
		if(!Sensor3.alertTimer && !isAlert(&Sensor3) && !FLAG_dispAlert) { clrAlertStateBit(&Sensor3); }
		
		/* Beep alert */
		if(FLAG_beepAlert && sysTimer == 100) 	 { setPWM(1); FLAG_beepAlert = 2; }
		if(FLAG_beepAlert == 2)
		{
			if(sysTimer > 100 && sysTimer < 200) { TIM_Cmd(TIM4, ENABLE); }
			if(sysTimer == 200) 				 { setPWM(0); }
			if(sysTimer > 200 && sysTimer < 300) { TIM_Cmd(TIM4, ENABLE); }
			if(sysTimer > 330) 					 { TIM_Cmd(TIM4, DISABLE); FLAG_beepAlert = 3; }
		}
		if(FLAG_beepAlert == 3)	{ FLAG_beepAlert = 0; }
		
		/* Flash indicator for inputs */
		if(sysTimer % 500 == 0) { FLAG_inputIndicator = 1; }
		
		/* Usb connection watchdog */
		if(FLAG_usbConnection) { FLAG_usbConnection--; }
		
		/* Counters */		
		if(sysTimer >= 999)
		{
			sysTimer = 0;
			sysTimerSec++;
			if(Sensor1.alertTimer) { Sensor1.alertTimer--; }
			if(Sensor2.alertTimer) { Sensor2.alertTimer--; }
			if(Sensor3.alertTimer) { Sensor3.alertTimer--; }
		}
		else { sysTimer++; }
		
		if(sysTimerSec >= 12) { sysTimerSec = 0; sysTimerMin++; }
		sysTimerMin = (sysTimerMin >= 5) ? 0 : sysTimerMin+1;
		
	}
}

/*** RFM73P Interrupt Handlers ***/
void EXTI3_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line3) != RESET)
  	{
    	FLAG_rfm_rx = 1;
    	EXTI_ClearITPendingBit(EXTI_Line3);
  	}
}

/*** USART Bluetooth Interrupt Handlers ***/
void USART2_IRQHandler(void)
{
	if( USART_GetITStatus(USART2, USART_IT_RXNE) )
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		rxBTchar = USART_ReceiveData(USART2);
		bufBTPush(rxBTchar);
	}
}

/*** USART KNX Interrupt Handlers ***/
void USART3_IRQHandler(void)
{
	if( USART_GetITStatus(USART3, USART_IT_RXNE) )
	{
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		rxKNXchar = USART_ReceiveData(USART3);
		bufKNXPush(rxKNXchar);
	}
}


/*** USB Interrupt Handlers ***/
void OTG_FS_IRQHandler(void)
{
	USBD_OTG_ISR_Handler (&USB_OTG_dev);
	FLAG_usbConnection = 1000;
}

void OTG_FS_WKUP_IRQHandler(void)
{
	if(USB_OTG_dev.cfg.low_power)
	{
		*(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
		SystemInit();
		USB_OTG_UngateClock(&USB_OTG_dev);
	}
	EXTI_ClearITPendingBit(EXTI_Line18);
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics ***** END OF FILE ****/
