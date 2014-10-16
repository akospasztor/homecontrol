/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    main.c
  * @brief   Main program
  *          This file contains the initializations and starting functions
  *          for the central unit software.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/ 

#include "main.h"
#include "globals.h"

#include "spi.h"
#include "ft800.h"
#include "lcd.h"
#include "usart.h"
#include "rfm73.h"

#include "ff.h"
#include "diskio.h"

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "usb_dcd_int.h"
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

uint32_t sys_delay = 0;

extern uint8_t rxBTchar;
extern uint8_t rxKNXchar;

extern RTC_TimeTypeDef Rtime;
extern RTC_DateTypeDef Rdate;

extern PeriphConnectionStruct 	PeriphConnection;
extern ModuleBTStruct 			ModuleBT;
extern ModuleTcpServerStruct	ModuleTcpServer;
extern ModuleRadioStruct 		ModuleRadio;
extern ModuleKNXStruct			ModuleKNX;

extern ModeStruct Mode;
extern SensorStruct Sensor1;
extern SensorStruct Sensor2;
extern SensorStruct Sensor3;
extern SensorStruct* SensorFav;

	
/*** MAIN **************************************************************************/
int main(void)
{	
	/* Initialization */
	__disable_irq();						/* Disable Interrupts for init */
	initSystem();							/* Init System */
	initSysTimer();							/* Init System Timer IT */
	initRTC(1, &Rdate, &Rtime);				/* Init RTC */
    initLeds();								/* Init LEDs */
	initPWM();								/* Init Buzzer PWM */
	initUSB();								/* Init USB VCP (Baud: 115200, 8N1 */
	SPI_init();								/* Init SPI1 high-speed bus (only for FT800) */
	initBT();								/* Init Bluetooth Uart */
	initKNX();								/* Init KNX & RS-485 Uart */
	RF_spi_init(SPI_BaudRatePrescaler_8);	/* Init Radio SPI */
	__enable_irq();							/* Enable Interrupts after init */
    
	
	/* FT800 init & calibrate */
    while(initFT800());
    sysDms(500);
    SPI_speedup();
    touch_calibrate();
    while(!cmd_ready());
    
	/* Clear & Start screen */
    clrscr();
    //lcd_start();
	
	/* Set defaults: Mode */
	Mode.type = HEATING;
	Mode.state = OFF;
	
	/* Set defaults: Sensors */
	SensorFav = &Sensor1;
	initSensor(&Sensor1, "Living Room",	24.0);
	initSensor(&Sensor2, "Bedroom", 23.0);
	initSensor(&Sensor3, "Kitchen", 22.0);
	
	/* Set defaults: Connection & Modules */
	PeriphConnectionStructInit(&PeriphConnection);
	ModuleKNXStructInit(&ModuleKNX);
	ModuleBTStructInit(&ModuleBT);
	ModuleRadioStructInit(&ModuleRadio);
	ModuleTcpServerStructInit(&ModuleTcpServer);
	
	/* Set defaults: Connection Address */
	setSensorCom(&Sensor1, COM_RF,  0xE1);
	setSensorCom(&Sensor2, COM_RF,  0xE2);
	setSensorCom(&Sensor3, COM_OFF, 0xE3);
	
	/* Check SD and load data from it */
	if(!lcd_start_sd())	
	{	
		/* Mode */
		Mode.type = HEATING;
	}
	
	/* Additional Initialization (which require IT-s) */
	if(rfm_init()) { ModuleRadio.state = 1; }	
	
	/* Main loop */
    while(1)
    {
		switch( lcd_main() )
		{
			case 1:	break;
			
			case 2:	lcd_status();
					break;
					
			case 3:	lcd_notifications();
					break;
			
			case 4: lcd_settings();
					break;
					
			case 5: lcd_date();
					break;
		}		
             
    } /* loop */
}


/*** INIT SYSTEM *******************************************************************/
void initSystem(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	
	SysTick_Config(SystemCoreClock/1000000);
	NVIC_SetPriority(SysTick_IRQn, 0);
	
	/* Default Date */
	Rtime.RTC_H12     = RTC_H12_AM;
	Rtime.RTC_Hours   = 19;
	Rtime.RTC_Minutes = 0;
	Rtime.RTC_Seconds = 45;
	RTC_SetTime(RTC_Format_BIN, &Rtime);
	
	/* Default Time */
	Rdate.RTC_Year = 14;
	Rdate.RTC_Month = RTC_Month_October;
	Rdate.RTC_Date = 6;
	Rdate.RTC_WeekDay = RTC_Weekday_Monday;
	RTC_SetDate(RTC_Format_BIN, &Rdate);
}

/*** SYSTEM TIMER ******************************************************************/
void initSysTimer(void)
{
	TIM_TimeBaseInitTypeDef timer;
	NVIC_InitTypeDef  nvic;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	/* 84MHz -> 1kHz (1ms) */
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	timer.TIM_Prescaler = 8399;
	timer.TIM_Period = 9;
	timer.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInit(TIM2, &timer);
	
	nvic.NVIC_IRQChannel = TIM2_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 0;
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
}

/*** INIT USB **********************************************************************/
void initUSB(void)
{
	USBD_Init(&USB_OTG_dev,
	           USB_OTG_FS_CORE_ID,
	           &USR_desc,
	           &USBD_CDC_cb,
	           &USR_cb );
}

/*** LEDS **************************************************************************/
void initLeds(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef gpio;
    GPIO_StructInit(&gpio);
    gpio.GPIO_Mode = GPIO_Mode_OUT;
    gpio.GPIO_Pin = LEDS;
    GPIO_Init(LEDport, &gpio);
}

void ledOn(uint16_t LED)
{
	GPIO_SetBits(LEDport, LED);
}

void ledOff(uint16_t LED)
{
	GPIO_ResetBits(LEDport, LED);
}

/*** BUZZER PWM ********************************************************************/
void initPWM(void)
{
	GPIO_InitTypeDef gpio;
	TIM_TimeBaseInitTypeDef  timer;
	TIM_OCInitTypeDef  timer_oc;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	/* PWM pin: PD13 (TIM4 CH2) */
	gpio.GPIO_Pin = GPIO_Pin_13;
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &gpio);
	
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
	
	/* Timer config: TIM4 CH2 */
	timer.TIM_Prescaler = 999;
	timer.TIM_Period = 55;
	timer.TIM_ClockDivision = 0;
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &timer);

	timer_oc.TIM_OCMode = TIM_OCMode_PWM1;
	timer_oc.TIM_OCPolarity = TIM_OCPolarity_High;
	timer_oc.TIM_Pulse = 50;
	timer_oc.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OC2Init(TIM4, &timer_oc);

	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(TIM4, ENABLE);
}

void setPWM(uint8_t level)
{
	TIM_TimeBaseInitTypeDef  timer;
	if(level)
	{	
		timer.TIM_Prescaler = 999;
		timer.TIM_Period = 55;
	}
	else
	{
		timer.TIM_Prescaler = 999;
		timer.TIM_Period = 65;
	}
	timer.TIM_ClockDivision = 0;
	timer.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &timer);
}

/*** REAL-TIME CLOCK ***************************************************************/
void initRTC(uint8_t reset, RTC_DateTypeDef* date, RTC_TimeTypeDef* time)
{
    RTC_InitTypeDef RTC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC */
    PWR_BackupAccessCmd(ENABLE);
	
	if(reset)
	{		
		/* Reset RTC Domain */
		RCC_BackupResetCmd(ENABLE);
		RCC_BackupResetCmd(DISABLE);

		/* Enable the LSE OSC */
		RCC_LSEConfig(RCC_LSE_ON);

		/* Wait until LSE is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

		/* Select the RTC Clock Source */
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);	

		/* Configure the RTC data register and RTC prescaler */
		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
		RTC_InitStructure.RTC_SynchPrediv  = 0xFF;
		RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;
		RTC_Init(&RTC_InitStructure);

		/* Set time */
		RTC_SetTime(RTC_Format_BIN, time);
		
		/* Set date */
		RTC_SetDate(RTC_Format_BIN, date);
	}
}

/*** DELAY *************************************************************************/
void delay(u32 ms)
{
    ms *= 24000;
    while(ms--) {
        __NOP();
    }
}

void sysDus(uint32_t us)
{
	sys_delay = us;
	while(sys_delay);
}

void sysDms(uint32_t ms)
{
	sys_delay = ms*1000;
	while(sys_delay);
}