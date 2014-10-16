/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    lcd_main.c
  * @brief   LCD Main Menu
  *          This file contains the functions for creating the display-list
  *          to draw and show the main menu screen.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "globals.h"
#include "linked_list.h"

#include "ft800.h"
#include "lcd_main.h"

extern ModeStruct Mode;
extern SensorStruct Sensor1;
extern SensorStruct Sensor2;
extern SensorStruct Sensor3;
extern SensorStruct* SensorFav;

extern RTC_TimeTypeDef Rtime;
extern RTC_DateTypeDef Rdate;

extern ListStruct* notif_head;

/*** STORE MAIN DEFAULTS INTO FT800 MEM ********************************************/
void store_main_default(void)
{
	cmd(CMD_MEMWRITE); cmd(RAM_G); cmd(11*4);
	cmd(CLEAR_TAG(255));
	cmd(CLEAR_COLOR_RGB(1,68,33));		//bg
	cmd(CLEAR(1,1,1));
	cmd(LINE_WIDTH(5*16));
	cmd(BEGIN(RECTS));
	cmd(COLOR_RGB(0x63,0xCD,0xFF));		//left boxes (1st is not displayed here)
	cmd(VERTEX2F(16*13,  16*102));
	cmd(VERTEX2F(16*151, 16*170));
	cmd(VERTEX2F(16*13,  16*190));
	cmd(VERTEX2F(16*151, 16*258));
	cmd(END());
}

/*** DRAW MAIN SCREEN **************************************************************/
void draw_main_screen(const ButtonStruct* btnStartStop, const ButtonStruct* btnStatus, const ButtonStruct* btnNotifications, const ButtonStruct* btnSettings, const ButtonStruct* btnSetDate)
{
	cmd(CMD_DLSTART);
	cmd(CMD_APPEND); cmd(RAM_G); cmd(11*4);
	
	draw_main_boxes(btnSetDate);
	draw_main_buttons(btnStartStop, btnStatus, btnNotifications, btnSettings);
	
	cmd(DISPLAY());
    cmd(CMD_SWAP);
	
	sysDms(10);
}

/*** DRAW MAIN SCREEN BOXES ********************************************************/
void draw_main_boxes(const ButtonStruct* btnSetDate)
{
	char buf[10];
	
	/* middle boxes outer dependig on which sensor is selected */
	cmd(COLOR_RGB(0xF0,0x00,0x07));
	cmd(LINE_WIDTH(5*16));
	cmd(BEGIN(RECTS));
	if(SensorFav == &Sensor1)
	{
		cmd(VERTEX2F(16*169, 16*12));
		cmd(VERTEX2F(16*311, 16*84));
	}
	if(SensorFav == &Sensor2)
	{		
		cmd(VERTEX2F(16*169, 16*100));
		cmd(VERTEX2F(16*311, 16*172));
	}
	if(SensorFav == &Sensor3)
	{
		cmd(VERTEX2F(16*169, 16*188));
		cmd(VERTEX2F(16*311, 16*260));
	}
	
	/* middle boxes */
	cmd(COLOR_RGB(0x63,0xCD,0xFF));
	cmd(LINE_WIDTH(5*16));
	cmd(TAG_MASK(1));
	cmd(TAG(11));
	cmd(VERTEX2F(16*171, 16*14));
	cmd(VERTEX2F(16*309, 16*82));
	cmd(TAG(12));
	cmd(VERTEX2F(16*171, 16*102));
	cmd(VERTEX2F(16*309, 16*170));
	cmd(TAG(13));
	cmd(VERTEX2F(16*171, 16*190));
	cmd(VERTEX2F(16*309, 16*258));
	cmd(TAG_MASK(0));
	
	/* heating/cooling box */
	if(Mode.type == HEATING) { cmd(COLOR_RGB(0xB9,0x00,0x09)); }
	else { cmd(COLOR_RGB(0x00,0x10,0xC0)); }
	
	cmd(VERTEX2F(16*13,  16*14));
	cmd(VERTEX2F(16*151, 16*82));
	cmd(END());
	
	cmd(COLOR_RGB(255,255,255));
	if(Mode.type == HEATING) {	cmd_text(80,15, 29,OPT_CENTERX, "Heating"); }
	else { cmd_text(80,15, 29,OPT_CENTERX, "Cooling"); }
	
	if(Mode.state == ON) { cmd_text(80,50, 30,OPT_CENTERX, "ON"); }
	else { cmd_text(80,50, 30,OPT_CENTERX, "OFF"); }
	
	/* boxes: sensor name underlines */
	cmd(COLOR_RGB(0xB9,0x00,0x07));
	cmd(BEGIN(LINES));
	cmd(LINE_WIDTH(8));
	cmd(VERTEX2F(16*170, 16*24));
	cmd(VERTEX2F(16*290, 16*24));
	cmd(VERTEX2F(16*170, 16*112));
	cmd(VERTEX2F(16*290, 16*112));
	cmd(VERTEX2F(16*170, 16*200));
	cmd(VERTEX2F(16*290, 16*200));
	cmd(END());
	
	/* boxes */
	cmd(COLOR_RGB(0xB9,0x00,0x07));
	cmd_text(170,10,  26,0, Sensor1.name);
	cmd_text(170,97,  26,0, Sensor2.name);
	cmd_text(170,186, 26,0, Sensor3.name);
	
	cmd(COLOR_RGB(0x10,0x20,0x30));
	
	if(Sensor1.errcount > 1) { cmd_text(260,10,  26,0, "(Old)"); }
	if(Sensor2.errcount > 1) { cmd_text(260,97,  26,0, "(Old)"); }
	if(Sensor3.errcount > 1) { cmd_text(260,186, 26,0, "(Old)"); }
	
	if(Sensor1.com)
	{
		cmd_text(170,25, 26,0, "Temp:"); 	sprintf(buf, "%.1f C", Sensor1.temp);	cmd_text(240,25, 26,0, buf);
		cmd_text(170,40, 26,0, "Humid:");	sprintf(buf, "%u%%", Sensor1.humid);	cmd_text(240,40, 26,0, buf);
		cmd_text(170,55, 26,0, "CO:");		sprintf(buf, "%u ppm", Sensor1.co);		cmd_text(240,55, 26,0, buf);
		cmd_text(170,70, 26,0, "CO2:");		sprintf(buf, "%u ppm", Sensor1.co2);	cmd_text(240,70, 26,0, buf);
	}
	else
	{	cmd_text(240,55, 27,OPT_CENTER, "Switched Off"); }
	
	if(Sensor2.com)
	{
		cmd_text(170,113, 26,0, "Temp:"); 	sprintf(buf, "%.1f C", Sensor2.temp);	cmd_text(240,113, 26,0, buf);
		cmd_text(170,128, 26,0, "Humid:");	sprintf(buf, "%u%%", Sensor2.humid);	cmd_text(240,128, 26,0, buf);
		cmd_text(170,143, 26,0, "CO:");		sprintf(buf, "%u ppm", Sensor2.co);		cmd_text(240,143, 26,0, buf);
		cmd_text(170,158, 26,0, "CO2:");	sprintf(buf, "%u ppm", Sensor2.co2);	cmd_text(240,158, 26,0, buf);
	}
	else 
	{	cmd_text(240,143, 27,OPT_CENTER, "Switched Off"); }
	
	if(Sensor3.com)
	{
		cmd_text(170,201, 26,0, "Temp:"); 	sprintf(buf, "%.1f C", Sensor3.temp);	cmd_text(240,201, 26,0, buf);
		cmd_text(170,216, 26,0, "Humid:");	sprintf(buf, "%u%%", Sensor3.humid);	cmd_text(240,216, 26,0, buf);
		cmd_text(170,231, 26,0, "CO:");		sprintf(buf, "%u ppm", Sensor3.co);		cmd_text(240,231, 26,0, buf);
		cmd_text(170,246, 26,0, "CO2:");	sprintf(buf, "%u ppm", Sensor3.co2);	cmd_text(240,246, 26,0, buf);
	}
	else 
	{	cmd_text(240,231, 27,OPT_CENTER, "Switched Off"); }
	
	/* favorite box */
	cmd_text(10,99, 26,0, "Target Temp:");
	sprintf(buf, "%.1f", SensorFav->target);
	cmd_text(82,145, 31,OPT_CENTER, buf);
	
	cmd_text(10,188, 26,0, "Current Temp:");
	sprintf(buf, "%.1f", SensorFav->temp);
	cmd_text(82,234, 31,OPT_CENTER, buf);

	/* date & time */
	cmd(LINE_WIDTH(5*16));
	cmd(BEGIN(RECTS));
	if(btnSetDate->pressed) { cmd(COLOR_RGB(0x23,0x8D,0xBF)); }
	else 					{ cmd(COLOR_RGB(0x63,0xCD,0xFF)); }
	cmd(TAG_MASK(1));
	cmd(TAG(5));
	cmd(VERTEX2F(16*329, 16*14));
	cmd(VERTEX2F(16*467, 16*82));
	cmd(TAG_MASK(0));
	cmd(END());
	
	cmd(COLOR_RGB(0x10,0x20,0x30));
	sprintf(buf, "%02u:%02u", Rtime.RTC_Hours, Rtime.RTC_Minutes);
	cmd_text(398,40, 31,OPT_CENTER, buf);
	sprintf(buf, "20%02u/%u/%u", Rdate.RTC_Year, Rdate.RTC_Month, Rdate.RTC_Date);
	cmd_text(398,75, 26,OPT_CENTER, buf);
}

/*** DRAW MAIN SCREEN BUTTONS ******************************************************/
void draw_main_buttons(const ButtonStruct* btnStartStop, const ButtonStruct* btnStatus, const ButtonStruct* btnNotifications, const ButtonStruct* btnSettings)
{
	u8 notif_num = list_size(notif_head);
	u16 button_style = 0;
	char buf[20];
	
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG_MASK(1));
	
	/* Start/Stop Button */
	if(Mode.state == OFF)
	{
		if(btnStartStop->pressed)	{ cmd_fgcolor(0x004000);	button_style = OPT_FLAT; }
		else 						{ cmd_fgcolor(0x008000);	button_style = 0; }
		
		cmd(TAG(1));
		cmd_button(324,226, 148,37, 27,button_style, "Start");
	}
	else
	{
		if(btnStartStop->pressed)	{ cmd_fgcolor(0x610004);	button_style = OPT_FLAT; }
		else 						{ cmd_fgcolor(0xb90007);	button_style = 0; }
		
		cmd(TAG(1));
		cmd_button(324,226, 148,37, 27,button_style, "Stop");
	}	
	
	/* Status Button */
	if(btnStatus->pressed) 		{ cmd_fgcolor(0x004080);	button_style = OPT_FLAT; }
	else 						{ cmd_fgcolor(0x0060B0);	button_style = 0; }
	cmd(TAG(2));
	cmd_button(324,97, 148,37, 27,button_style, "Status");
	
	/* Notifications Button */
	if(notif_num) { sprintf(buf, "Notifications (%u)", list_size(notif_head)); }
	else 		  { sprintf(buf, "Notifications"); }
	if(btnNotifications->pressed) 	{ cmd_fgcolor(0x696900);	button_style = OPT_FLAT; }
	else 							{ cmd_fgcolor(0xb0b000);	button_style = 0; }
	cmd(TAG(3));
	
	cmd_button(324,185, 148,37, 27,button_style, buf);	
	
	/* Settings Button */
	if(btnSettings->pressed)  	{ cmd_fgcolor(0x203040);	button_style = OPT_FLAT; }
	else 						{ cmd_fgcolor(0x506070); 	button_style = 0; }
	cmd(TAG(4));
	cmd_button(324,138, 148,37, 27,button_style, "Settings");
	
	cmd(TAG_MASK(0));
}