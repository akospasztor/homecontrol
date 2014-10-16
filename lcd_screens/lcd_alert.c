/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    lcd_alert.c
  * @brief   LCD Alert Screen
  *          This file contains the functions for creating the display-list
  *          to draw and show the alerts.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "main.h"
#include "globals.h"

#include "ft800.h"
#include "lcd_alert.h"

#include <stdio.h>

/*** DRAW ALERT SCREEN *************************************************************/
void draw_alert_screen(SensorStruct* sensor, const ButtonStruct* btnOk)
{
	char buf[33];
	uint16_t posY = 100;
	
	cmd(CMD_DLSTART);
	cmd(CLEAR_COLOR_RGB(1,68,33));
	cmd(CLEAR(1,1,1));
	cmd(LINE_WIDTH(5*16));
	cmd(BEGIN(RECTS));
	cmd(COLOR_RGB(0x63,0xCD,0xFF));		
	cmd(VERTEX2F(16*40,  16*40));
	cmd(VERTEX2F(16*440, 16*232));
	cmd(END());
	cmd(COLOR_RGB(0xB9,0x00,0x07));
	cmd(LINE_WIDTH(1*16));
	cmd(BEGIN(LINES));
	cmd(VERTEX2F(16*80, 16*90));
	cmd(VERTEX2F(16*400, 16*90));
	cmd(END());
	cmd(COLOR_RGB(0xB9,0x00,0x07));
	cmd_text(90,40, 31,0, "Alert:");
	cmd_text(300,55, 28,OPT_CENTERX, sensor->name);
	cmd(COLOR_RGB(0x10,0x20,0x30));
	
	/* temperature alert */
	if(checkAlert(sensor, ALERT_TEMP_H))
	{
		cmd_text(80,posY, 27,0, "Temp. is too HIGH:");
		cmd(COLOR_RGB(0xB9,0x00,0x07));
		sprintf(buf, "%.1f C", sensor->temp);
		cmd_text(270,posY, 27,0, buf);
		cmd(COLOR_RGB(0x10,0x20,0x30));
		sprintf(buf, "(%.1fC)", sensor->aTmax);
		cmd_text(350,posY+2, 26,0, buf);
		posY += 20;
	}
	else if(checkAlert(sensor, ALERT_TEMP_L))
	{
		cmd_text(80,posY, 27,0, "Temp. is too LOW:");
		cmd(COLOR_RGB(0x00,0x00,0x80));
		sprintf(buf, "%.1f C", sensor->temp);
		cmd_text(270,posY, 27,0, buf);
		cmd(COLOR_RGB(0x10,0x20,0x30));
		sprintf(buf, "(%.1fC)", sensor->aTmin);
		cmd_text(350,posY+2, 26,0, buf);
		posY += 20;		
	}
	
	
	/* humidity alert */
	if(checkAlert(sensor, ALERT_HUM_H))
	{
		cmd_text(80,posY, 27,0, "Humidity is too HIGH:");
		cmd(COLOR_RGB(0xB9,0x00,0x07));
		sprintf(buf, "%u %%", sensor->humid);
		cmd_text(270,posY, 27,0, buf);
		cmd(COLOR_RGB(0x10,0x20,0x30));
		sprintf(buf, "(%u%%)", sensor->aHmax);
		cmd_text(350,posY+2, 26,0, buf);
		posY += 20;
	}
	else if(checkAlert(sensor, ALERT_HUM_L))
	{ 
		cmd_text(80,posY, 27,0, "Humidity is too LOW:");
		cmd(COLOR_RGB(0x00,0x00,0x80));
		sprintf(buf, "%u %%", sensor->humid);
		cmd_text(270,posY, 27,0, buf);
		cmd(COLOR_RGB(0x10,0x20,0x30));
		sprintf(buf, "(%u%%)", sensor->aHmin);
		cmd_text(350,posY+2, 26,0, buf);
		posY += 20;
	}
	
	/* co alert */
	if(checkAlert(sensor, ALERT_CO))
	{ 
		cmd_text(80,posY, 27,0, "CO level is too HIGH:");
		cmd(COLOR_RGB(0xB9,0x00,0x07));
		sprintf(buf, "%u ppm", sensor->co);
		cmd_text(270,posY, 27,0, buf);
		cmd(COLOR_RGB(0x10,0x20,0x30));
		sprintf(buf, "(%u ppm)", sensor->aCO);
		cmd_text(350,posY+2, 26,0, buf);
		posY += 20;
	}
	
	/* co2 alert */
	if(checkAlert(sensor, ALERT_CO2))
	{ 
		cmd_text(80,posY, 27,0, "CO2 level is too HIGH:");
		cmd(COLOR_RGB(0xB9,0x00,0x07));
		sprintf(buf, "%u ppm", sensor->co2);
		cmd_text(270,posY, 27,0, buf);
		cmd(COLOR_RGB(0x10,0x20,0x30));
		sprintf(buf, "(%u ppm)", sensor->aCO2);
		cmd_text(350,posY+2, 26,0, buf);
		posY += 20;
	}
	
	
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG(1));
	
	if(!btnOk->pressed)
	{
		cmd_fgcolor(0x0000C0);
		cmd_button(190,190, 100,36, 28,0, "OK");
	}
	else
	{
		cmd_fgcolor(0x000040);
		cmd_button(190,190, 100,36, 28,OPT_FLAT, "OK");
	}
	
	cmd(DISPLAY());
	cmd(CMD_SWAP);
}