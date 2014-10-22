/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    lcd.c
  * @brief   LCD Menus 
  *          This file contains the functions for menus which includes:
  *           - processing tasks
  *           - handling user interactions
  *           - drawing screens
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "main.h"
#include "globals.h"
#include "lcd.h"
#include "ft800.h"

#include "lcd_start.h"
#include "lcd_sd.h"
#include "lcd_main.h"
#include "lcd_alert.h"
#include "lcd_status.h"
#include "lcd_notifications.h"
#include "lcd_date.h"
#include "lcd_settings.h"

#include "ff.h"
#include "diskio.h"

#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "usb_dcd_int.h"
extern __ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Structures */
extern PeriphConnectionStruct PeriphConnection;
extern ModeStruct Mode;
extern SensorStruct Sensor1;
extern SensorStruct Sensor2;
extern SensorStruct Sensor3;
extern SensorStruct* SensorFav;

/* Variables */
extern uint8_t EnableAlerts;
extern RTC_TimeTypeDef Rtime;
extern RTC_DateTypeDef Rdate;

/* Flags */
extern uint8_t FLAG_datetime;
extern uint8_t FLAG_getPeriphConnection;
extern uint8_t FLAG_dispAlert;
extern uint8_t FLAG_beepAlert;
extern uint8_t FLAG_inputIndicator;

/* Globals */
uint32_t tag = 0;			/* lcd touch TAG: 		global variable for live watch */
uint32_t tracker = 0;		/* lcd touch TRACKER: 	global variable for live watch */


/*** START SCREEN ******************************************************************/
void lcd_start(void)
{
	tag = 0;
	ButtonStruct btnContinue = TouchInit(1);
	
	/* startup animation */
	draw_startup_animation();

	/* startup screen */
	draw_start_screen(&btnContinue);
	
	while(1)
	{
		tag = HOST_MEM_RD32(REG_TOUCH_TAG);
		
		//button pressed
		if(tag == 1 && !btnContinue.pressed)
		{
			btnContinue.pressed = 1;
			draw_start_screen(&btnContinue);
		}
		
		//button released
		if(tag != btnContinue.tag && btnContinue.pressed)
		{
			if(buttonIsClicked(&btnContinue, tag)) { return; }
			btnContinue.pressed = 0;
			draw_start_screen(&btnContinue);
		}
	}
}

uint8_t lcd_start_sd(void)
{
	tag = 0;
	
	u8 valid = 1;		/* variable for checking whether the data is valid on the uSD */
	u8 finished = 0;	/* variable for checking whether every row is readed correctly */
	char error_msg[33];
	memset(error_msg, '\0', sizeof(error_msg));
	
	FATFS FatFs;
	FIL fil;
	
	//microSD	
	char row[NAME_MAX+3];
		
	if( !f_mount(&FatFs, "", 1) )
	{	
		if( !f_open(&fil, "config.txt", FA_OPEN_EXISTING | FA_READ) )
		{
			u8 cntr = 0;			
			
			/* read MODE */
			f_gets(row, sizeof(row), &fil);
			removeCRLF(row);
			if(strstr(row, "HEATING")) 		{ Mode.type = HEATING; }
			else if(strstr(row, "COOLING")) { Mode.type = COOLING; }
			else { valid = 0; }
			
			/* read EnableAlerts */
			f_gets(row, sizeof(row), &fil);
			removeCRLF(row);
			if(strstr(row, "ALERT_ENABLE")) 		{ EnableAlerts = 1; }
			else if(strstr(row, "ALERT_DISABLE"))	{ EnableAlerts = 0; }
			else { valid = 0; }
			
			/* read Sensors Data */
			for(cntr=0; cntr<3; ++cntr) 
			{
				SensorStruct* sensor;
				switch(cntr)
				{
					case 0: sensor = &Sensor1; break;
					case 1: sensor = &Sensor2; break;
					case 2: sensor = &Sensor3; break;
				}
				
				memset(row, '\0', sizeof(row));
					   
				/* name */
				f_gets(row, sizeof(row), &fil);
				removeCRLF(row);
				strcpy(sensor->name, row);
				
				/* communication */
				f_gets(row, sizeof(row), &fil);
				removeCRLF(row);
				if(strstr(row, "COM_RF")) 		{ sensor->com = COM_RF; }
				else if(strstr(row, "COM_TP"))	{ sensor->com = COM_TP; }
				else if(strstr(row, "COM_OFF"))	{ sensor->com = COM_OFF; }
				else { valid = 0; }
				
				/* target temp */
				f_gets(row, sizeof(row), &fil);
				removeCRLF(row);
				if(atof(row)) { sensor->target = atof(row); }
				else { valid = 0; }
				
				/* alerts: T_H, T_L, H_H, H_L, CO, CO2 (6) */
				f_gets(row, sizeof(row), &fil);
				removeCRLF(row);
				if(atof(row)) { sensor->aTmax = atof(row); }
				else { valid = 0; }
				
				f_gets(row, sizeof(row), &fil);
				removeCRLF(row);
				if(atof(row)) { sensor->aTmin = atof(row); }
				else { valid = 0; }
				
				f_gets(row, sizeof(row), &fil);
				removeCRLF(row);
				if(atoi(row)) { sensor->aHmax = atoi(row); }
				else { valid = 0; }
				
				f_gets(row, sizeof(row), &fil);
				removeCRLF(row);
				if(atoi(row)) { sensor->aHmin = atoi(row); }
				else { valid = 0; }
				
				f_gets(row, sizeof(row), &fil);
				removeCRLF(row);
				if(atoi(row)) { sensor->aCO = atoi(row); }
				else { valid = 0; }
				
				f_gets(row, sizeof(row), &fil);
				removeCRLF(row);
				if(atoi(row)) { sensor->aCO2 = atoi(row); }
				else { valid = 0; }
				
				/* check valid */
				if(!valid)
				{
					strcpy(error_msg, "Config file is corrupt!");
					break;
				}			
			} /* sensors */
			
			//close file
			f_close(&fil);
			if(valid) { finished = 1; }
			
		} /* f_open */
		else
		{
			strcpy(error_msg, "Cannot open the config file!");
		}
		
		//dismount
		f_mount(NULL, "", 1);
	} /* f_mount */
	else
	{
		strcpy(error_msg, "No microSD card is inserted!");
	}

	//check if reading from uSD was successful
	if(finished) { return 1; }
	
	// force release SD ChipSelect
	sdc_deassert();
	
	//display warning
	ButtonStruct btnOk = TouchInit(1);
	draw_sd_warning_screen(error_msg, &btnOk);	
	
	while(1)
	{
		tag = HOST_MEM_RD32(REG_TOUCH_TAG);
		
		//button pressed
		if(tag == 1 && !btnOk.pressed)
		{
			btnOk.pressed = 1;
			draw_sd_warning_screen(error_msg, &btnOk);
		}
		
		//button released
		if(tag != 1 && btnOk.pressed)
		{
			if(buttonIsClicked(&btnOk, tag)) { return 0; }
			btnOk.pressed = 0;
			draw_sd_warning_screen(error_msg, &btnOk);
		}
	}
}

/*** MAIN **************************************************************************/
uint8_t lcd_main(void)
{
	tag = 0;
	
	/* init buttons */
	ButtonStruct btnStartStop 		= TouchInit(1);
	ButtonStruct btnStatus 			= TouchInit(2);
	ButtonStruct btnNotifications 	= TouchInit(3);
	ButtonStruct btnSettings 		= TouchInit(4);
	ButtonStruct btnSetDate 		= TouchInit(5);
	
	/* write background into memory */
	store_main_default();
	
	/* first start: display buttons & others */
	draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
	
	while(1)
	{
		/* Process Tasks */
		processTasks();
		
		/* Display Alerts */
		if(EnableAlerts)
		{
			/* Check Sensor1 for Alerts to display */
			if(isAlert(&Sensor1) && !FLAG_dispAlert)
			{
				FLAG_beepAlert = 1;
				FLAG_dispAlert = 1;
				setAlertStateBit(&Sensor1);
				lcd_alert(&Sensor1);
				Sensor1.alertTimer = ALERT_TIME;
				FLAG_dispAlert = 0;
			}
			
			/* Check Sensor2 for Alerts to display */
			if(isAlert(&Sensor2) && !FLAG_dispAlert)
			{
				FLAG_beepAlert = 1;
				FLAG_dispAlert = 1;
				setAlertStateBit(&Sensor2);
				lcd_alert(&Sensor2);
				Sensor2.alertTimer = ALERT_TIME;
				FLAG_dispAlert = 0;
			}
			
			/* Check Sensor3 for Alerts to display */
			if(isAlert(&Sensor3) && !FLAG_dispAlert)
			{
				FLAG_beepAlert = 1;
				FLAG_dispAlert = 1;
				setAlertStateBit(&Sensor3);
				lcd_alert(&Sensor3);
				Sensor3.alertTimer = ALERT_TIME;
				FLAG_dispAlert = 0;
			}
		}
		
		/* Draw date & time if needed AND redraw screen in every sec */
		if(FLAG_datetime)
		{ 
			draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
			FLAG_datetime = 0;
		}
		
		/* Check Touch TAG */
		tag = HOST_MEM_RD32(REG_TOUCH_TAG);
		
		switch(tag)
		{
			case 1:		// start/stop button
			if(!btnStartStop.pressed)
				{
					btnStartStop.pressed = 1;
					draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
				}
			break;
			
			case 2:		// status button
				if(!btnStatus.pressed)
				{
					btnStatus.pressed = 1;
					draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
				}
			break;
			
			case 3:		// notifications button
				if(!btnNotifications.pressed)
				{
					btnNotifications.pressed = 1;
					draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
				}
			break;
			
			case 4:		// settings button
				if(!btnSettings.pressed)
				{
					btnSettings.pressed = 1;
					draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
				}
			break;
			
			case 5:		// set Date & Time
				if(!btnSetDate.pressed)
				{
					btnSetDate.pressed = 1;
					draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
				}
			break;
			
			case 11:	// Select Sensor1 as Favorite
				SensorFav = &Sensor1;
				draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
			break;	
			case 12:	// Select Sensor2 as Favorite
				SensorFav = &Sensor2;
				draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
			break;
			case 13:	// Select Sensor3 as Favorite
				SensorFav = &Sensor3;
				draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
			break;
			
			default:	//only if something is pressed
				if(btnStartStop.pressed || btnStatus.pressed || btnNotifications.pressed || btnSettings.pressed || btnSetDate.pressed)
				{
					if(buttonIsClicked(&btnStartStop, tag))		{ Mode.state ^= ON; }
					if(buttonIsClicked(&btnStatus, tag))		{ return btnStatus.tag; }
					if(buttonIsClicked(&btnNotifications, tag))	{ /*Mode.type ^= 1;*/ return btnNotifications.tag; }
					if(buttonIsClicked(&btnSettings, tag))		{ return btnSettings.tag; }
					if(buttonIsClicked(&btnSetDate, tag))		{ return btnSetDate.tag; }
					
					btnStartStop.pressed = 0;
					btnStatus.pressed = 0;
					btnNotifications.pressed = 0;
					btnSettings.pressed	= 0;
					btnSetDate.pressed = 0;
					
					draw_main_screen(&btnStartStop, &btnStatus, &btnNotifications, &btnSettings, &btnSetDate);
				}
			break;
		} /* switch */
		
	} /* while */
	
}

/*** ALERT *************************************************************************/
void lcd_alert(SensorStruct* sensor)
{
	tag = 0;
	ButtonStruct btnOk = TouchInit(1);
	draw_alert_screen(sensor, &btnOk);
	
	while(1)
	{
		/* Process Tasks */
		processTasks();
		
		/* Redraw if new alert comes during display */
		if(!(sensor->alert & 0x01))
		{
			draw_alert_screen(sensor, &btnOk);
			setAlertStateBit(sensor);
		}
		
		tag = HOST_MEM_RD32(REG_TOUCH_TAG);
		
		//button pressed
		if(tag == 1 && !btnOk.pressed)
		{
			btnOk.pressed = 1;
			draw_alert_screen(sensor, &btnOk);
		}
		
		//button released
		if(tag != 1 && btnOk.pressed)
		{
			if(buttonIsClicked(&btnOk, tag)) { return; }
			btnOk.pressed = 0;
			draw_alert_screen(sensor, &btnOk);
		}
	}
}

/*** STATUS ************************************************************************/
void lcd_status(void)
{
	tag = 0;
	ButtonStruct btnHome = TouchInit(5);
	
	draw_status_screen(&btnHome);
	
	while(1)
	{	
		/* Process Tasks */
		processTasks();
		
		/* Get PeriphConnection Status */
		if(FLAG_getPeriphConnection)
		{
			getPeriphConnection(&PeriphConnection);
			draw_status_screen(&btnHome);
			FLAG_getPeriphConnection = 0;
		}
		
		tag = HOST_MEM_RD32(REG_TOUCH_TAG);
		switch(tag)
		{
			case 5:
				if(!btnHome.pressed)
				{
					btnHome.pressed = 1;
					draw_status_screen(&btnHome);
				}
			break;
			
			default:
				if(btnHome.pressed)
				{
					if(buttonIsClicked(&btnHome, tag)) { return; }
					btnHome.pressed = 0;
					
					draw_status_screen(&btnHome);
				}
			break;
		}
		
	} /* while */
}

/*** NOTIFICATIONS *****************************************************************/
void lcd_notifications(void)
{
	tag = 0;
	
	ButtonStruct btnHome  = TouchInit(5);
	ButtonStruct btnClear = TouchInit(6);
	
	draw_notifications_screen(&btnHome, &btnClear);
	
	while(1)
	{	
		/* Process Tasks */
		processTasks();
		
		/* Redraw in every sec */
		if(FLAG_datetime)
		{ 
			draw_notifications_screen(&btnHome, &btnClear);
			FLAG_datetime = 0;
		}
		
		tag = HOST_MEM_RD32(REG_TOUCH_TAG);
		switch(tag)
		{
			case 5:
				if(!btnHome.pressed)
				{
					btnHome.pressed = 1;
					draw_notifications_screen(&btnHome, &btnClear);
				}
			break;
			
			case 6:
				if(!btnClear.pressed)
				{
					btnClear.pressed = 1;
					draw_notifications_screen(&btnHome, &btnClear);
				}
			break;
			
			default:
				if(btnHome.pressed || btnClear.pressed)
				{
					if(buttonIsClicked(&btnHome, tag))  { return; }
					if(buttonIsClicked(&btnClear, tag)) { clearNotif(); }
					
					btnHome.pressed = 0;
					btnClear.pressed = 0;
					
					draw_notifications_screen(&btnHome, &btnClear);
				}
			break;
		}
		
	} /* while */
}

/*** SET DATE & TIME ***************************************************************/
void lcd_date(void)
{
	tag = 0;
	RTC_DateTypeDef newDate = Rdate;
	RTC_TimeTypeDef newTime = Rtime;
	
	ButtonStruct btnBack = TouchInit(5);
	ButtonStruct keys = TouchInit(0);
	
	/* write background into memory */
	store_date_defaults();
	
	/* display */
	draw_date_screen(&btnBack, &keys, &newDate, &newTime);
	
	while(1)
	{		
		tag = HOST_MEM_RD32(REG_TOUCH_TAG);
		
		/* Buttons */
		switch(tag)
		{
			case 5:
				if(!btnBack.pressed)
				{
					btnBack.pressed = 1;
					draw_date_screen(&btnBack, &keys, &newDate, &newTime);
				}
				break;
			
			case 10:
			case 11:
			case 20:
			case 21:
			case 30:
			case 31:
			case 40:
			case 41:
			case 50:
			case 51:
				if(!keys.pressed)
				{
					keys.pressed = 1;
					keys.tag = tag;
					draw_date_screen(&btnBack, &keys, &newDate, &newTime);
				}
			break;
			
			default:
				if( btnBack.pressed ||
				    keys.pressed )
				{
					if(buttonIsClicked(&btnBack, tag))
					{
						initRTC(1, &newDate, &newTime);
						RTC_GetTime(RTC_Format_BIN, &Rtime);
						RTC_GetDate(RTC_Format_BIN, &Rdate);
						return;
					}
					
					if(buttonIsClicked(&keys, tag))
					{
						if(keys.tag == 10 && newDate.RTC_Year  > 0)  { newDate.RTC_Year--; }
						if(keys.tag == 11 && newDate.RTC_Year  < 99) { newDate.RTC_Year++; }
						if(keys.tag == 20 && newDate.RTC_Month > 0)  { newDate.RTC_Month--; }
						if(keys.tag == 21 && newDate.RTC_Month < 12) { newDate.RTC_Month++; }
						if(keys.tag == 30 && newDate.RTC_Date > 0)   { newDate.RTC_Date--; }
						if(keys.tag == 31 && newDate.RTC_Date < 31)  { newDate.RTC_Date++; }
						
						if(keys.tag == 40 && newTime.RTC_Hours > 0)    { newTime.RTC_Hours--; }
						if(keys.tag == 41 && newTime.RTC_Hours < 23)   { newTime.RTC_Hours++; }
						if(keys.tag == 50 && newTime.RTC_Minutes > 0)  { newTime.RTC_Minutes--; }
						if(keys.tag == 51 && newTime.RTC_Minutes < 59) { newTime.RTC_Minutes++; }
						
						keys.tag = 0;
					}
					
					btnBack.pressed = 0;
					keys.pressed = 0;
					
					draw_date_screen(&btnBack, &keys, &newDate, &newTime);
				}
			break;
		}
		
	} /* while */
}

/*** SETTINGS **********************************************************************/
void lcd_settings(void)
{	
	tag = 0;
	uint8_t redraw = 1; 	/* set to 1 on first start and also set in case of return from a submenu (this rewrites the ft800 ram) */
	
	ButtonStruct btnHome = TouchInit(5);
	ButtonStruct btnEnableAlerts = TouchInit(6);
	ButtonStruct btnS1Alert = TouchInit(10);
	ButtonStruct btnS1Conf 	= TouchInit(11);
	ButtonStruct btnS1Name 	= TouchInit(12);
	ButtonStruct btnS2Alert = TouchInit(20);
	ButtonStruct btnS2Conf 	= TouchInit(21);
	ButtonStruct btnS2Name	= TouchInit(22);
	ButtonStruct btnS3Alert = TouchInit(30);
	ButtonStruct btnS3Conf 	= TouchInit(31);
	ButtonStruct btnS3Name 	= TouchInit(32);
	
	/* write background into memory and draw screen */
	store_settings_default();
	draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
	
	while(1)
	{		
		tag = HOST_MEM_RD32(REG_TOUCH_TAG);
		
		/* Buttons */
		switch(tag)
		{
			case 5:
				if(!btnHome.pressed)
				{
					btnHome.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			
			case 6:
				if(!btnEnableAlerts.pressed)
				{
					btnEnableAlerts.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			
			/* Sensor 1 Buttons */
			case 10:
				if(!btnS1Alert.pressed)
				{
					btnS1Alert.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			case 11:
				if(!btnS1Conf.pressed)
				{
					btnS1Conf.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			case 12:
				if(!btnS1Name.pressed)
				{
					btnS1Name.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			
			/* Sensor 2 Buttons */
			case 20:
				if(!btnS2Alert.pressed)
				{
					btnS2Alert.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			case 21:
				if(!btnS2Conf.pressed)
				{
					btnS2Conf.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			case 22:
				if(!btnS2Name.pressed)
				{
					btnS2Name.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			
			/* Sensor 3 Buttons */
			case 30:
				if(!btnS3Alert.pressed)
				{
					btnS3Alert.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			case 31:
				if(!btnS3Conf.pressed)
				{
					btnS3Conf.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			case 32:
				if(!btnS3Name.pressed)
				{
					btnS3Name.pressed = 1;
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
			
			default:
				if(btnHome.pressed || btnEnableAlerts.pressed ||
				   btnS1Alert.pressed || btnS1Conf.pressed || btnS1Name.pressed ||
				   btnS2Alert.pressed || btnS2Conf.pressed || btnS2Name.pressed ||
				   btnS3Alert.pressed || btnS3Conf.pressed || btnS3Name.pressed)
				{
					if(buttonIsClicked(&btnHome, tag)) 			{ writeConfigToSD(); return; }
					if(buttonIsClicked(&btnEnableAlerts, tag)) 	{ EnableAlerts ^= 1; }
					if(buttonIsClicked(&btnS1Alert, tag)) 		{ lcd_settings_alert(&Sensor1); redraw = 1; }
					if(buttonIsClicked(&btnS1Conf, tag)) 		{ lcd_settings_conf(&Sensor1);  redraw = 1; }
					if(buttonIsClicked(&btnS1Name, tag)) 		{ lcd_settings_name(&Sensor1);  redraw = 1; }
					if(buttonIsClicked(&btnS2Alert, tag)) 		{ lcd_settings_alert(&Sensor2); redraw = 1; }
					if(buttonIsClicked(&btnS2Conf, tag)) 		{ lcd_settings_conf(&Sensor2);  redraw = 1; }
					if(buttonIsClicked(&btnS2Name, tag)) 		{ lcd_settings_name(&Sensor2);  redraw = 1; }
					if(buttonIsClicked(&btnS3Alert, tag)) 		{ lcd_settings_alert(&Sensor3); redraw = 1; }
					if(buttonIsClicked(&btnS3Conf, tag)) 		{ lcd_settings_conf(&Sensor3);  redraw = 1; }
					if(buttonIsClicked(&btnS3Name, tag)) 		{ lcd_settings_name(&Sensor3);  redraw = 1;}
					
					btnHome.pressed = 0;
					btnEnableAlerts.pressed = 0;
					btnS1Alert.pressed 	= 0;
					btnS1Conf.pressed 	= 0;
					btnS1Name.pressed 	= 0;
					btnS2Alert.pressed 	= 0;
					btnS2Conf.pressed 	= 0;
					btnS2Name.pressed 	= 0;
					btnS3Alert.pressed 	= 0;
					btnS3Conf.pressed 	= 0;
					btnS3Name.pressed 	= 0;
					
					if(redraw) { store_settings_default(); redraw = 0; }	/* we need to rewrite ft800 mem in case of return from submenu */
					draw_settings_screen(&btnHome, &btnEnableAlerts, &btnS1Alert, &btnS1Conf, &btnS1Name, &btnS2Alert, &btnS2Conf, &btnS2Name, &btnS3Alert, &btnS3Conf, &btnS3Name);
				}
			break;
		}
		
	} /* while */
	
}

/*** SETTINGS: ALERT ***************************************************************/
void lcd_settings_alert(SensorStruct* Sensor)
{
	tag = 0;
	tracker = 0;
	
	SliderStruct Slider1;
	ButtonStruct btnBack 		= TouchInit(6);
	ButtonStruct btnTempMaxM	= TouchInit(10);
	ButtonStruct btnTempMaxP	= TouchInit(11);
	ButtonStruct btnHumMaxM		= TouchInit(12);
	ButtonStruct btnHumMaxP		= TouchInit(13);
	ButtonStruct btnCOMaxM		= TouchInit(14);
	ButtonStruct btnCOMaxP		= TouchInit(15);
	ButtonStruct btnTempMinM	= TouchInit(20);
	ButtonStruct btnTempMinP	= TouchInit(21);
	ButtonStruct btnHumMinM		= TouchInit(22);
	ButtonStruct btnHumMinP		= TouchInit(23);
	ButtonStruct btnCO2MaxM		= TouchInit(24);
	ButtonStruct btnCO2MaxP		= TouchInit(25);
	
	/* get default value for slider */
	sliderInit( &Slider1, (65535 - (uint16_t)((Sensor->target-5)/25*65535)) );
	
	/* write background into memory */
	store_settings_alert_default();
	
	/* display */
	draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
	
	while(1)
	{
		tracker = HOST_MEM_RD32(REG_TRACKER);
		tag = HOST_MEM_RD32(REG_TOUCH_TAG);
		
		/* Slider */
		if((tracker & 0xFF) == 91)
		{	
			/*add to avg array & calculate avg*/			
			sliderPush(&Slider1, tracker>>16);
			Slider1.value = sliderAvg(&Slider1);
			
			/*set sensor & display*/
			Sensor->target = ((float)(65535-Slider1.value) / 65535 * 25.0) + 5.0;
			draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
		}
		
		/* Buttons */
		switch(tag)
		{
			case 6:
				if(!btnBack.pressed)
				{
					btnBack.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			
			case 10:
				if(!btnTempMaxM.pressed)
				{
					btnTempMaxM.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			case 11:
				if(!btnTempMaxP.pressed)
				{
					btnTempMaxP.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			
			case 12:
				if(!btnHumMaxM.pressed)
				{
					btnHumMaxM.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			case 13:
				if(!btnHumMaxP.pressed)
				{
					btnHumMaxP.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			
			case 14:
				if(!btnCOMaxM.pressed)
				{
					btnCOMaxM.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			case 15:
				if(!btnCOMaxP.pressed)
				{
					btnCOMaxP.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			
			case 20:
				if(!btnTempMinM.pressed)
				{
					btnTempMinM.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			case 21:
				if(!btnTempMinP.pressed)
				{
					btnTempMinP.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			
			case 22:
				if(!btnHumMinM.pressed)
				{
					btnHumMinM.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			case 23:
				if(!btnHumMinP.pressed)
				{
					btnHumMinP.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			
			case 24:
				if(!btnCO2MaxM.pressed)
				{
					btnCO2MaxM.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			case 25:
				if(!btnCO2MaxP.pressed)
				{
					btnCO2MaxP.pressed = 1;
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
			
			default:
				if( btnBack.pressed ||
				    btnTempMaxM.pressed ||
					btnTempMaxP.pressed ||
					btnHumMaxM.pressed ||
					btnHumMaxP.pressed ||
					btnCOMaxM.pressed ||
					btnCOMaxP.pressed ||
					btnTempMinM.pressed ||
					btnTempMinP.pressed ||
					btnHumMinM.pressed ||
					btnHumMinP.pressed ||
					btnCO2MaxM.pressed ||
					btnCO2MaxP.pressed )
				{
					if(buttonIsClicked(&btnBack, tag)) 		{ return; }
					if(buttonIsClicked(&btnTempMaxM, tag))	{ Sensor->aTmax -= 0.1; }
					if(buttonIsClicked(&btnTempMaxP, tag)) 	{ Sensor->aTmax += 0.1; }
					if(buttonIsClicked(&btnHumMaxM, tag)) 	{ Sensor->aHmax -= 1; }
					if(buttonIsClicked(&btnHumMaxP, tag)) 	{ Sensor->aHmax += 1; }
					if(buttonIsClicked(&btnCOMaxM, tag)) 	{ Sensor->aCO -= 1; }
					if(buttonIsClicked(&btnCOMaxP, tag)) 	{ Sensor->aCO += 1; }
					if(buttonIsClicked(&btnTempMinM, tag))	{ Sensor->aTmin -= 0.1; }
					if(buttonIsClicked(&btnTempMinP, tag)) 	{ Sensor->aTmin += 0.1; }
					if(buttonIsClicked(&btnHumMinM, tag)) 	{ Sensor->aHmin -= 1; }
					if(buttonIsClicked(&btnHumMinP, tag)) 	{ Sensor->aHmin += 1; }
					if(buttonIsClicked(&btnCO2MaxM, tag)) 	{ Sensor->aCO2 -= 1; }
					if(buttonIsClicked(&btnCO2MaxP, tag)) 	{ Sensor->aCO2 += 1; }
					
					btnBack.pressed = 0;
					btnTempMaxM.pressed = 0;
					btnTempMaxP.pressed = 0;
					btnHumMaxM.pressed	= 0;
					btnHumMaxP.pressed	= 0;
					btnCOMaxM.pressed	= 0;
					btnCOMaxP.pressed	= 0;
					btnTempMinM.pressed	= 0;
					btnTempMinP.pressed	= 0;
					btnHumMinM.pressed	= 0;
					btnHumMinP.pressed	= 0;
					btnCO2MaxM.pressed	= 0;
					btnCO2MaxP.pressed	= 0;
					
					draw_settings_alert_screen(Sensor, &btnBack, &Slider1, &btnTempMaxM, &btnTempMaxP, &btnHumMaxM, &btnHumMaxP, &btnCOMaxM, &btnCOMaxP, &btnTempMinM, &btnTempMinP, &btnHumMinM, &btnHumMinP, &btnCO2MaxM, &btnCO2MaxP);
				}
			break;
		}
		
	} /* while */
}

/*** SETTINGS: CONFIGURE ***********************************************************/
void lcd_settings_conf(SensorStruct* Sensor)
{
	tag = 0;
	
	ButtonStruct btnHome = TouchInit(6);
	
	/* write background into memory and draw screen */
	store_settings_conf_default();
	draw_settings_conf_screen(Sensor, &btnHome);
	
	while(1)
	{		
		tag = HOST_MEM_RD32(REG_TOUCH_TAG);
		
		/* Buttons */
		switch(tag)
		{
			case 6:
				if(!btnHome.pressed)
				{
					btnHome.pressed = 1;
					draw_settings_conf_screen(Sensor, &btnHome);
				}
			break;
			
			case 10:
				if(Sensor->com != COM_OFF)
				{
					Sensor->com = COM_OFF;
					Sensor->temp = 0.0;
					Sensor->humid = 0;
					Sensor->co = 0;
					Sensor->co2 = 0;
					Sensor->FLAG_poll = 0;
					Sensor->errcount = 0;
					draw_settings_conf_screen(Sensor, &btnHome);
				}
			break;
			case 11:
				if(Sensor->com != COM_TP)
				{
					Sensor->com = COM_TP;
					Sensor->FLAG_poll = 0;
					Sensor->errcount = 0;
					draw_settings_conf_screen(Sensor, &btnHome);
				}
			break;
			
			case 12:
				if(Sensor->com != COM_RF)
				{
					Sensor->com = COM_RF;
					Sensor->FLAG_poll = 0;
					Sensor->errcount = 0;
					draw_settings_conf_screen(Sensor, &btnHome);
				}
			break;
			
			default:
				if(btnHome.pressed)
				{
					if(buttonIsClicked(&btnHome, tag)) 		{ return; }
					btnHome.pressed = 0;
					
					draw_settings_conf_screen(Sensor, &btnHome);
				}
			break;
		}
		
	} /* while */
}

/*** SETTINGS: CHANGE NAME *********************************************************/
void lcd_settings_name(SensorStruct* Sensor)
{
	tag = 0;
	uint32_t tag_raw = 0;
		
	uint8_t shift = 1;
	uint8_t name_ptr = 0;
	char name[NAME_MAX+1];
	memset(name, '\0', sizeof(name));
	name_ptr = sprintf(name, Sensor->name);
	
	ButtonStruct btnHome = TouchInit(6);
	ButtonStruct keyboard = TouchInit(0);
	
	/* write background into memory and draw screen */
	store_settings_name_default();
	draw_settings_name_screen(Sensor, &btnHome, &keyboard, shift, name);
	
	while(1)
	{	
		tag_raw <<= 8;
		tag_raw |= HOST_MEM_RD32(REG_TOUCH_TAG) & 0xFF;
		
		if(tagIsValid(tag_raw))
		{
			tag = tag_raw & 0xFF;
			if(tag) { keyboard.tag = tag; }
		}
		
		/* Redraw every 500ms */
		if(FLAG_inputIndicator)
		{
			draw_settings_name_screen(Sensor, &btnHome, &keyboard, shift, name);
			FLAG_inputIndicator = 0;
		}
		
		/* Keyboard:  32: space, 94: shift, 127: delete, 128: clear */
		if((tag>=65 && tag<=90) || (tag>=97 && tag<=122) || tag == 32 || tag == 94 || tag == 127 || tag == 128)
		{
			if(!keyboard.pressed)
			{
				keyboard.pressed = 1;				
				draw_settings_name_screen(Sensor, &btnHome, &keyboard, shift, name);
			}
		}
		
		/* Buttons */
		switch(tag)
		{
			case 6:
				if(!btnHome.pressed)
				{
					btnHome.pressed = 1;
					draw_settings_name_screen(Sensor, &btnHome, &keyboard, shift, name);
				}
			break;
			
			default:
				if(btnHome.pressed || keyboard.pressed)
				{
					if(buttonIsClicked(&btnHome, tag))
					{
						if(strlen(name))
						{	
							strcpy(Sensor->name, name);
							return;
						}
					}
					if(buttonIsClicked(&keyboard, tag))
					{ 	
						if(keyboard.tag == 94) { shift ^= 1; }
						if( ((keyboard.tag>=65 && keyboard.tag<=90) || (keyboard.tag>=97 && keyboard.tag<=122) || keyboard.tag==32) && name_ptr<NAME_MAX ) { name[name_ptr++] = keyboard.tag; }
						if(keyboard.tag == 127 && name_ptr > 0) { name[--name_ptr] = '\0'; }
						if(keyboard.tag == 128 && name_ptr > 0) { memset(name, '\0', sizeof(name)); name_ptr = 0; }
						keyboard.pressed = 0;
						keyboard.tag = 0;
					}
					
					btnHome.pressed = 0;
					
					draw_settings_name_screen(Sensor, &btnHome, &keyboard, shift, name);
				}
			break;
		}
		
	} /* while */
}

/*** INIT **************************************************************************/
uint8_t initFT800(void)
{   
    uint8_t dev_id = 0;                 	// Variable for holding the read device id    
    GPIO_ResetBits(LCD_PORT_PD, LCD_PDN);   // Set the PDN pin low 
    
    sysDms(50);                          	// Delay 50 ms for stability
    GPIO_SetBits(LCD_PORT_PD, LCD_PDN); 	// Set the PDN pin high
    sysDms(50);                          	// Delay 50 ms for stability

    //WAKE
    HOST_CMD_ACTIVE();
	sysDms(500);
    
    //RESET
    //HOST_CMD_WRITE(CMD_CORERST);
    //sysDms(1000);

    //Ext Clock
    HOST_CMD_WRITE(CMD_CLKEXT);         // Send CLK_EXT Command (0x44)

    //PLL (48M) Clock
    HOST_CMD_WRITE(CMD_CLK48M);         // Send CLK_48M Command (0x62)

    //Read Dev ID
    dev_id = HOST_MEM_RD8(REG_ID);      // Read device id
    if(dev_id != 0x7C)                  // Device ID should always be 0x7C
    {   
        return 1;
    }

    HOST_MEM_WR8(REG_GPIO, 0x00);		// Set REG_GPIO to 0 to turn off the LCD DISP signal
    HOST_MEM_WR8(REG_PCLK, 0x00);      	// Pixel Clock Output disable
    
    HOST_MEM_WR16(REG_HCYCLE, 548);         // Set H_Cycle to 548
    HOST_MEM_WR16(REG_HOFFSET, 43);         // Set H_Offset to 43
    HOST_MEM_WR16(REG_HSYNC0, 0);           // Set H_SYNC_0 to 0
    HOST_MEM_WR16(REG_HSYNC1, 41);          // Set H_SYNC_1 to 41
    HOST_MEM_WR16(REG_VCYCLE, 292);         // Set V_Cycle to 292
    HOST_MEM_WR16(REG_VOFFSET, 12);         // Set V_OFFSET to 12
    HOST_MEM_WR16(REG_VSYNC0, 0);           // Set V_SYNC_0 to 0
    HOST_MEM_WR16(REG_VSYNC1, 10);          // Set V_SYNC_1 to 10
    HOST_MEM_WR8(REG_SWIZZLE, 0);           // Set SWIZZLE to 0
    HOST_MEM_WR8(REG_PCLK_POL, 1);          // Set PCLK_POL to 1
    HOST_MEM_WR8(REG_CSPREAD, 1);           // Set CSPREAD to 1
    HOST_MEM_WR16(REG_HSIZE, 480);          // Set H_SIZE to 480
    HOST_MEM_WR16(REG_VSIZE, 272);          // Set V_SIZE to 272
    
    /* configure touch & audio */
    HOST_MEM_WR8(REG_TOUCH_MODE, 0x03);     	//set touch on: continous
    HOST_MEM_WR8(REG_TOUCH_ADC_MODE, 0x01); 	//set touch mode: differential
	HOST_MEM_WR8(REG_TOUCH_OVERSAMPLE, 0x0F); 	//set touch oversampling to max
	HOST_MEM_WR16(REG_TOUCH_RZTHRESH, 5000);	//set touch resistance threshold
    HOST_MEM_WR8(REG_VOL_SOUND, 0xFF);      	//set the volume to maximum

    /* write first display list */
    HOST_MEM_WR32(RAM_DL+0, CLEAR_COLOR_RGB(0,0,0));  // Set Initial Color to BLACK
    HOST_MEM_WR32(RAM_DL+4, CLEAR(1,1,1));            // Clear to the Initial Color
    HOST_MEM_WR32(RAM_DL+8, DISPLAY());               // End Display List

    HOST_MEM_WR8(REG_DLSWAP, DLSWAP_FRAME);           // Make this display list active on the next frame

    HOST_MEM_WR8(REG_GPIO_DIR, 0x80);                 // Set Disp GPIO Direction 
    HOST_MEM_WR8(REG_GPIO, 0x80);                     // Enable Disp (if used)
    HOST_MEM_WR16(REG_PWM_HZ, 0x00FA);                // Backlight PWM frequency
    HOST_MEM_WR8(REG_PWM_DUTY, 0x80);                 // Backlight PWM duty    
    
    HOST_MEM_WR8(REG_PCLK, 0x05);                     // After this display is visible on the LCD

    return 0;
}

void touch_calibrate(void)
{    
    /*cmd(CMD_DLSTART);
    cmd(CLEAR_COLOR_RGB(0, 0, 0));
    cmd(CLEAR(1, 1, 1));
    cmd(CMD_CALIBRATE);
    cmd(0);*/
    
    HOST_MEM_WR32(REG_TOUCH_TRANSFORM_A, 0x00007DB5);
    HOST_MEM_WR32(REG_TOUCH_TRANSFORM_B, 0xFFFFFEF6);
    HOST_MEM_WR32(REG_TOUCH_TRANSFORM_C, 0xFFF61664);
    HOST_MEM_WR32(REG_TOUCH_TRANSFORM_D, 0x00000048);
    HOST_MEM_WR32(REG_TOUCH_TRANSFORM_E, 0xFFFFB387);
    HOST_MEM_WR32(REG_TOUCH_TRANSFORM_F, 0x0119F874);
}

void clrscr(void)
{
    cmd(CMD_DLSTART);
    cmd(CLEAR_COLOR_RGB(0,0,0));
    cmd(CLEAR(1,1,1));
    cmd(DISPLAY());
    cmd(CMD_SWAP);
}
