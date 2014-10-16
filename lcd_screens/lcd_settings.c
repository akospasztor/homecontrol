/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    lcd_settings.c
  * @brief   LCD Settings Menu
  *          This file contains the functions for creating the display-list
  *          to draw and show the settings menu and submenus.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "main.h"
#include "globals.h"

#include "ft800.h"
#include "lcd_settings.h"

#include <stdio.h>
#include <string.h>

extern uint16_t sysTimer;

extern ModeStruct Mode;
extern SensorStruct Sensor1;
extern SensorStruct Sensor2;
extern SensorStruct Sensor3;
extern SensorStruct* SensorFav;

extern uint8_t EnableAlerts;
extern uint8_t FLAG_inputIndicator;

/*** STORE SETTINGS DEFAULTS INTO FT800 MEM ****************************************/
void store_settings_default(void)
{
	cmd(CMD_MEMWRITE); cmd(RAM_G); cmd(15*4);
	cmd(CLEAR_TAG(255));
	cmd(CLEAR_COLOR_RGB(1,68,33));
	cmd(CLEAR(1,1,1));
	cmd(BEGIN(RECTS));
	cmd(COLOR_RGB(0x9E,0xD6,0xF0));
	cmd(LINE_WIDTH(5*16));
	cmd(VERTEX2F(16*13,  16*64));	//box1
	cmd(VERTEX2F(16*151, 16*258));
	cmd(VERTEX2F(16*171, 16*64));	//box2
	cmd(VERTEX2F(16*309, 16*258));
	cmd(VERTEX2F(16*329, 16*64));	//box3
	cmd(VERTEX2F(16*467, 16*258));
	cmd(END());
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,50));
}

/*** DRAW SETTINGS SCREEN **********************************************************/
void draw_settings_screen(const ButtonStruct* btnHome, const ButtonStruct* btnEnableAlerts,
						  const ButtonStruct* btnS1Alert, const ButtonStruct* btnS1Conf, const ButtonStruct* btnS1Name,
						  const ButtonStruct* btnS2Alert, const ButtonStruct* btnS2Conf, const ButtonStruct* btnS2Name,
						  const ButtonStruct* btnS3Alert, const ButtonStruct* btnS3Conf, const ButtonStruct* btnS3Name)
{
	u16 button_style = 0;
	
	cmd(CMD_DLSTART);
	cmd(CMD_APPEND); cmd(RAM_G); cmd(15*4);
	cmd_gradient(240,0,0xA0B0C0, 240,50, 0x506070);	
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,272));
	cmd(COLOR_RGB(255,255,255));
	cmd_text(240,25, 30,OPT_CENTER, "Settings");
	
	/* Sensor 1 text*/
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd(LINE_WIDTH(1*16));
	cmd(BEGIN(LINES));
	cmd(VERTEX2F(16*20, 16*88));
	cmd(VERTEX2F(16*120, 16*88));
	cmd(END());
	cmd_text(20,65, 27,0, Sensor1.name);
	
	/* Sensor 2 text */
	cmd(BEGIN(LINES));
	cmd(VERTEX2F(16*178, 16*88));
	cmd(VERTEX2F(16*278, 16*88));
	cmd(END());
	cmd_text(178,65, 27,0, Sensor2.name);
	
	/* Sensor 3 text */
	cmd(BEGIN(LINES));
	cmd(VERTEX2F(16*336, 16*88));
	cmd(VERTEX2F(16*436, 16*88));
	cmd(END());
	cmd_text(336,65, 27,0, Sensor3.name);
	
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG_MASK(1));
	
	/* Sensor 1 Buttons */
	if(btnS1Alert->pressed)	{ cmd_fgcolor(0x7f4600); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0xff8c00);	button_style = 0; }
	cmd(TAG(10));
	cmd_button(18,113, 128,40, 27,button_style, "Temp & Alerts");
	
	if(btnS1Conf->pressed)	{ cmd_fgcolor(0x696900); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0xb0b000);	button_style = 0; }
	cmd(TAG(11));
	cmd_button(18,163, 128,40, 27,button_style, "Configure");
	
	if(btnS1Name->pressed)	{ cmd_fgcolor(0x203040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x506070);	button_style = 0; }
	cmd(TAG(12));
	cmd_button(18,213, 128,40, 27,button_style, "Change Name");
	
	/* Sensor 2 Buttons */
	if(btnS2Alert->pressed)	{ cmd_fgcolor(0x7f4600); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0xff8c00);	button_style = 0; }
	cmd(TAG(20));
	cmd_button(176,113, 128,40, 27,button_style, "Temp & Alerts");
	
	if(btnS2Conf->pressed)	{ cmd_fgcolor(0x696900); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0xb0b000);	button_style = 0; }
	cmd(TAG(21));
	cmd_button(176,163, 128,40, 27,button_style, "Configure");
	
	if(btnS2Name->pressed)	{ cmd_fgcolor(0x203040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x506070);	button_style = 0; }
	cmd(TAG(22));
	cmd_button(176,213, 128,40, 27,button_style, "Change Name");
	
	/* Sensor 3 Buttons */
	if(btnS3Alert->pressed)	{ cmd_fgcolor(0x7f4600); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0xff8c00);	button_style = 0; }
	cmd(TAG(30));
	cmd_button(334,113, 128,40, 27,button_style, "Temp & Alerts");
	
	if(btnS3Conf->pressed)	{ cmd_fgcolor(0x696900); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0xb0b000);	button_style = 0; }
	cmd(TAG(31));
	cmd_button(334,163, 128,40, 27,button_style, "Configure");
	
	if(btnS3Name->pressed)	{ cmd_fgcolor(0x203040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x506070);	button_style = 0; }
	cmd(TAG(32));
	cmd_button(334,213, 128,40, 27,button_style, "Change Name");
	
	
	
	/* Home button */
	if(btnHome->pressed)	{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x000080);	button_style = 0; }	
	cmd(TAG(5));
	cmd_button(10,10, 80,30, 27,button_style, "Back");
	
	/* Enable Alerts button */
	if(!EnableAlerts)
	{
		if(btnEnableAlerts->pressed) { cmd_fgcolor(0x004000);	button_style = OPT_FLAT; }
		else 						 { cmd_fgcolor(0x008000);	button_style = 0; }
		
		cmd(TAG(6));
		cmd_button(350,10, 120,30, 27,button_style, "Enable Alerts");
	}
	else
	{
		if(btnEnableAlerts->pressed) { cmd_fgcolor(0x610004);	button_style = OPT_FLAT; }
		else 						 { cmd_fgcolor(0xb90007);	button_style = 0; }
		
		cmd(TAG(6));
		cmd_button(350,10, 120,30, 27,button_style, "Disable Alerts");
	}
	
	cmd(TAG_MASK(0));
	cmd(DISPLAY());
    cmd(CMD_SWAP);
	
	sysDms(20);
}

/*** STORE SETTINGS: SENSOR ALERT DEFAULTS INTO FT800 MEM **************************/
void store_settings_alert_default(void)
{
	cmd(CMD_MEMWRITE); cmd(RAM_G); cmd(30*4);
	cmd(CLEAR_TAG(255));
	cmd(CLEAR_COLOR_RGB(1,68,33));
	cmd(CLEAR(1,1,1));
	cmd(BEGIN(RECTS));
	cmd(COLOR_RGB(1,68,33));
	cmd(LINE_WIDTH(1*16));
	cmd(TAG_MASK(1));
	cmd(TAG(91));					//slider assist
	cmd(VERTEX2F(16*110, 16*80));
	cmd(VERTEX2F(16*160, 16*240));
	cmd(TAG_MASK(0));
	cmd(COLOR_RGB(0x9E,0xD6,0xF0));
	cmd(LINE_WIDTH(5*16));
	cmd(VERTEX2F(16*13,  16*64));	//slider box
	cmd(VERTEX2F(16*150, 16*258));
	cmd(VERTEX2F(16*168, 16*64));	//row1
	cmd(VERTEX2F(16*259, 16*154));
	cmd(VERTEX2F(16*272, 16*64));
	cmd(VERTEX2F(16*363, 16*154));
	cmd(VERTEX2F(16*376, 16*64));
	cmd(VERTEX2F(16*467, 16*154));
	cmd(VERTEX2F(16*168, 16*168));	//row2
	cmd(VERTEX2F(16*259, 16*258));
	cmd(VERTEX2F(16*272, 16*168));
	cmd(VERTEX2F(16*363, 16*258));
	cmd(VERTEX2F(16*376, 16*168));
	cmd(VERTEX2F(16*467, 16*258));
	cmd(END());
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,50));
}

/*** DRAW SETTINGS: SENSOR ALERT SCREEN ********************************************/
void draw_settings_alert_screen(SensorStruct* Sensor, const ButtonStruct* btnBack, const SliderStruct* Slider,
								const ButtonStruct* btnTempMaxM,	const ButtonStruct* btnTempMaxP,
								const ButtonStruct* btnHumMaxM, 	const ButtonStruct* btnHumMaxP,
								const ButtonStruct* btnCOMaxM, 		const ButtonStruct* btnCOMaxP,
								const ButtonStruct* btnTempMinM, 	const ButtonStruct* btnTempMinP,
								const ButtonStruct* btnHumMinM, 	const ButtonStruct* btnHumMinP,
								const ButtonStruct* btnCO2MaxM, 	const ButtonStruct* btnCO2MaxP)
{
	u16 button_style = 0;
	char buf[5];
	
	/* Tracking */
	cmd_track(110,80, 50,160, 91);
	
	cmd(CMD_DLSTART);
	cmd(CMD_APPEND); cmd(RAM_G); cmd(30*4);
	cmd_gradient(240,0,0xA0B0C0, 240,50, 0x506070);	
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,272));
	cmd(COLOR_RGB(255,255,255));
	cmd_text(240,25, 29,OPT_CENTER, "Settings: Alert");
	
	/* Texts & Data: Slider box */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd(LINE_WIDTH(1*16));
	cmd(BEGIN(LINES));
	cmd(VERTEX2F(16*20, 16*88));
	cmd(VERTEX2F(16*120, 16*88));
	cmd(END());
	cmd_text(20,65, 27,0, Sensor->name);
	cmd_text(20,230, 26,0, "Range: 5C - 30C");
	cmd_text(20,120, 26,0, "Target Temp:");
	sprintf(buf, "%.1f", Sensor->target); 
	cmd_text(25,145, 31,0, buf);
	
	
	/* Alert Temp Max */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd_text(170,64, 26,0, "Temp Max:");
	sprintf(buf, "%.1f C", Sensor->aTmax);
	cmd_text(213,99, 29,OPT_CENTER, buf);
	cmd(COLOR_RGB(255,255,255));
	if(btnTempMaxM->pressed)	{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0x000080);	button_style = 0; }
	cmd(TAG_MASK(1)); 
	cmd(TAG(10));
	cmd_button(170,119, 40,33, 27,button_style, "-");
	if(btnTempMaxP->pressed)	{ cmd_fgcolor(0x610004); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0xb90007);	button_style = 0; }
	cmd(TAG(11));
	cmd_button(217,119, 40,33, 27,button_style, "+");
	cmd(TAG_MASK(0));
	
	/* Alert Hum Max */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd_text(274,64, 26,0, "Hum. Max:");
	sprintf(buf, "%u %%", Sensor->aHmax);
	cmd_text(317,99, 29,OPT_CENTER, buf);
	cmd(COLOR_RGB(255,255,255));
	if(btnHumMaxM->pressed)		{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0x000080);	button_style = 0; }
	cmd(TAG_MASK(1));
	cmd(TAG(12));
	cmd_button(274,119, 40,33, 27,button_style, "-");
	if(btnHumMaxP->pressed)		{ cmd_fgcolor(0x610004); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0xb90007);	button_style = 0; }
	cmd(TAG(13));
	cmd_button(321,119, 40,33, 27,button_style, "+");
	cmd(TAG_MASK(0));
	
	/* Alert CO Max */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd_text(378,64, 26,0, "CO Max:");
	sprintf(buf, "%u ppm", Sensor->aCO);
	cmd_text(421,99, 28,OPT_CENTER, buf);
	cmd(COLOR_RGB(255,255,255));
	if(btnCOMaxM->pressed)		{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0x000080);	button_style = 0; }
	cmd(TAG_MASK(1));
	cmd(TAG(14));
	cmd_button(378,119, 40,33, 27,button_style, "-");
	if(btnCOMaxP->pressed)		{ cmd_fgcolor(0x610004); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0xb90007);	button_style = 0; }
	cmd(TAG(15));
	cmd_button(425,119, 40,33, 27,button_style, "+");
	cmd(TAG_MASK(0));
	
	/* Alert Temp Min */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd_text(170,168, 26,0, "Temp Min:");
	sprintf(buf, "%.1f C", Sensor->aTmin);
	cmd_text(213,203, 29,OPT_CENTER, buf);
	cmd(COLOR_RGB(255,255,255));
	if(btnTempMinM->pressed)	{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0x000080);	button_style = 0; }
	cmd(TAG_MASK(1));
	cmd(TAG(20));
	cmd_button(170,223, 40,33, 27,button_style, "-");
	if(btnTempMinP->pressed)	{ cmd_fgcolor(0x610004); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0xb90007);	button_style = 0; }
	cmd(TAG(21));
	cmd_button(217,223, 40,33, 27,button_style, "+");
	cmd(TAG_MASK(0));
	
	/* Alert Hum Min */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd_text(274,168, 26,0, "Hum. Min:");
	sprintf(buf, "%u %%", Sensor->aHmin);
	cmd_text(317,203, 29,OPT_CENTER, buf);
	cmd(COLOR_RGB(255,255,255));
	if(btnHumMinM->pressed)		{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0x000080);	button_style = 0; }
	cmd(TAG_MASK(1));
	cmd(TAG(22));
	cmd_button(274,223, 40,33, 27,button_style, "-");
	if(btnHumMinP->pressed)		{ cmd_fgcolor(0x610004); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0xb90007);	button_style = 0; }
	cmd(TAG(23));
	cmd_button(321,223, 40,33, 27,button_style, "+");
	cmd(TAG_MASK(0));
	
	/* Alert CO2 Max */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd_text(378,168, 26,0, "CO2 Max:");
	sprintf(buf, "%u ppm", Sensor->aCO2);
	cmd_text(421,203, 28,OPT_CENTER, buf);
	cmd(COLOR_RGB(255,255,255));
	if(btnCO2MaxM->pressed)		{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0x000080);	button_style = 0; }
	cmd(TAG_MASK(1));
	cmd(TAG(24));
	cmd_button(378,223, 40,33, 27,button_style, "-");
	if(btnCO2MaxP->pressed)		{ cmd_fgcolor(0x610004); 	button_style = OPT_FLAT; }
	else				 		{ cmd_fgcolor(0xb90007);	button_style = 0; }
	cmd(TAG(25));
	cmd_button(425,223, 40,33, 27,button_style, "+");
	cmd(TAG_MASK(0));
	

	/* Slider */
	cmd(COLOR_RGB(255,255,255));
	cmd_fgcolor(0xb90007);
	cmd_bgcolor(0x610004);
	cmd(TAG_MASK(1));
	cmd(TAG(91));
	cmd_slider(130,80, 10,160, 0, Slider->value,0xFFFF);
	
	/* Back button */
	if(btnBack->pressed)	{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x000080);	button_style = 0; }
	
	cmd(TAG(6));
	cmd_button(10,10, 80,30, 27,button_style, "Back");
	cmd(TAG_MASK(0));
	
	cmd(DISPLAY());
    cmd(CMD_SWAP);
	
	sysDms(20);
}


/*** STORE SETTINGS: SENSOR CONF DEFAULTS INTO FT800 MEM ***************************/
void store_settings_conf_default(void)
{
	cmd(CMD_MEMWRITE); cmd(RAM_G); cmd(11*4);
	cmd(CLEAR_TAG(255));
	cmd(CLEAR_COLOR_RGB(1,68,33));
	cmd(CLEAR(1,1,1));
	cmd(BEGIN(RECTS));
	cmd(COLOR_RGB(0x9E,0xD6,0xF0));
	cmd(LINE_WIDTH(5*16));
	cmd(VERTEX2F(16*13,  16*64));
	cmd(VERTEX2F(16*467, 16*258));
	cmd(END());
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,50));
}

/*** DRAW SETTINGS: SENSOR CONF SCREEN *********************************************/
void draw_settings_conf_screen(SensorStruct* Sensor, const ButtonStruct* btnBack)
{
	u16 button_style = 0;
	char buf[18];
	
	cmd(CMD_DLSTART);
	cmd(CMD_APPEND); cmd(RAM_G); cmd(11*4);
	cmd_gradient(240,0,0xA0B0C0, 240,50, 0x506070);	
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,272));
	cmd(COLOR_RGB(255,255,255));
	cmd_text(240,25, 29,OPT_CENTER, "Settings: Configure");
	
	/* Sensor info */
	cmd(COLOR_RGB(0xb9,0x00,0x07));
	cmd(LINE_WIDTH(1*16));
	cmd(BEGIN(LINES));
	cmd(VERTEX2F(16*20, 16*88));
	cmd(VERTEX2F(16*320, 16*88));
	cmd(END());
	cmd_text(20,65, 27,0, Sensor->name);
	cmd(COLOR_RGB(0x10,0x20,0x30));
	
	cmd_text(30,100, 27,0, "Connection type:");
	switch(Sensor->com)
	{
		case COM_TP: sprintf(buf, "RS-485/KNX");	break;
		case COM_RF: sprintf(buf, "Radio");			break;
		default: sprintf(buf, "Off");				break;
	}
	cmd_text(220,103, 26,0, buf);
	
	if(Sensor->com == COM_RF)
	{ 
		sprintf(buf, "%X:%X:%X:%X:%X",	Sensor->addr[0],
										Sensor->addr[1],
										Sensor->addr[2],
										Sensor->addr[3],
										Sensor->addr[4]);
		cmd_text(30,130, 27,0, "Connection address:");
		cmd_text(220,133, 26,0, buf);
	}
		   
	cmd(COLOR_RGB(255,255,255));
	
	/* Buttons */
	cmd(TAG_MASK(1));
	if(Sensor->com == COM_TP)	{ cmd_fgcolor(0x004000); button_style = OPT_FLAT; }
	else 						{ cmd_fgcolor(0x008000); button_style = 0; }
	cmd(TAG(11));
	cmd_button(40,200, 120,40, 27,button_style, "RS-485/KNX");
	
	if(Sensor->com == COM_RF)	{ cmd_fgcolor(0x004000); button_style = OPT_FLAT; }
	else 						{ cmd_fgcolor(0x008000); button_style = 0; }
	cmd(TAG(12));
	cmd_button(180,200, 120,40, 27,button_style, "Radio");
	
	if(Sensor->com == OFF)		{ cmd_fgcolor(0x203040); button_style = OPT_FLAT; }
	else 						{ cmd_fgcolor(0x506070); button_style = 0; }
	cmd(TAG(10));
	cmd_button(320,200, 120,40, 27,button_style, "Off");
	
	
	/* Back button */
	if(btnBack->pressed)	{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x000080);	button_style = 0; }
	
	cmd(TAG(6));
	cmd_button(10,10, 80,30, 27,button_style, "Back");
	cmd(TAG_MASK(0));
	
	cmd(DISPLAY());
    cmd(CMD_SWAP);
	
	sysDms(20);
}


/*** STORE SETTINGS: SENSOR NAME DEFAULTS INTO FT800 MEM ***************************/
void store_settings_name_default(void)
{
	cmd(CMD_MEMWRITE); cmd(RAM_G); cmd(15*4);
	cmd(CLEAR_TAG(255));
	cmd(CLEAR_COLOR_RGB(1,68,33));
	cmd(CLEAR(1,1,1));
	cmd(BEGIN(RECTS));
	cmd(COLOR_RGB(0x9E,0xD6,0xF0));
	cmd(LINE_WIDTH(5*16));
	cmd(VERTEX2F(16*13,  16*64));
	cmd(VERTEX2F(16*467, 16*258));
	cmd(COLOR_RGB(0xA0,0xB0,0xC0));		/* input field */
	cmd(LINE_WIDTH(3*16));
	cmd(VERTEX2F(16*43, 16*83));
	cmd(VERTEX2F(16*357, 16*107));
	cmd(END());
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,50));
}

/*** DRAW SETTINGS: SENSOR NAME SCREEN *********************************************/
void draw_settings_name_screen(SensorStruct* Sensor, const ButtonStruct* btnBack, const ButtonStruct* keyboard, uint8_t shift, char* name)
{
	u8 length = 0;
	u16 button_style = 0;
	char buf[NAME_MAX+2];
	
	cmd(CMD_DLSTART);
	cmd(CMD_APPEND); cmd(RAM_G); cmd(15*4);
	cmd_gradient(240,0,0xA0B0C0, 240,50, 0x506070);	
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,272));
	cmd(COLOR_RGB(255,255,255));
	cmd_text(240,25, 29,OPT_CENTER, "Settings: Change Name");
	
	/* Backspace */
	cmd(TAG_MASK(1));
	cmd(TAG(127));
	if(keyboard->tag == 127 && keyboard->pressed) { cmd_fgcolor(0x000040); button_style = OPT_FLAT; }
	else 										  { cmd_fgcolor(0x000080); button_style = 0; } 
	cmd_button(380, 80, 60, 30, 27, button_style, "<-");
	cmd(TAG_MASK(0));
	
	/* Shift */
	cmd(TAG_MASK(1));
	cmd(TAG(94));
	if(keyboard->tag == 94 && keyboard->pressed)  { cmd_fgcolor(0x000040); button_style = OPT_FLAT; }
	else 										  { cmd_fgcolor(0x000080); button_style = 0; } 
	cmd_button(40, 230, 57, 30, 26, button_style, "Shift");
	cmd(TAG_MASK(0));
	
	/* Draw keyboard */
	cmd_fgcolor(0x000080);
	cmd_bgcolor(0x000040);
	cmd(TAG_MASK(1));
	if(shift)
	{
		cmd_keys(40, 125, 400, 30, 28,  keyboard->tag, "QWERTYUIOP");
		cmd_keys(60, 160, 360, 30, 28,  keyboard->tag, "ASDFGHJKL");
		cmd_keys(100, 195, 280, 30, 28, keyboard->tag, "ZXCVBNM");
	}
	else
	{
		cmd_keys(40, 125, 400, 30, 28,  keyboard->tag, "qwertyuiop");
		cmd_keys(60, 160, 360, 30, 28,  keyboard->tag, "asdfghjkl");
		cmd_keys(100, 195, 280, 30, 28, keyboard->tag, "zxcvbnm");
	}
	cmd_keys(140, 230, 200, 30, 28, keyboard->tag, " ");
	cmd(TAG_MASK(0));
	
	/* Sensor name */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	if(sysTimer < 500) { length = sprintf(buf, "%s|", name) -1; }
	else { length = sprintf(buf, "%s", name); }
	cmd_text(50, 85, 27, 0, buf);
	
	if(length == NAME_MAX)
	{
		cmd(COLOR_RGB(0x77,0x77,0x77));
		cmd_text(290, 85, 26, 0, "(max)");
	}
	else if(!length)
	{
		cmd(COLOR_RGB(0x77,0x77,0x77));
		cmd_text(200, 85, 26, OPT_CENTERX, "Please enter a name");
	}
	
	/* Clear button */
	if(length)
	{
		cmd(COLOR_RGB(255,255,255));
		cmd(TAG_MASK(1));
		cmd(TAG(128));
		if(keyboard->tag == 128 && keyboard->pressed) { cmd_fgcolor(0x203040); button_style = OPT_FLAT; }
		else 										  { cmd_fgcolor(0x506070); button_style = 0; } 
		cmd_button(333, 84, 20, 20, 26, button_style, "x");
		cmd(TAG_MASK(0));
	}
	
	/* Back button */
	cmd(COLOR_RGB(255,255,255));
	if(btnBack->pressed)	{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x000080);	button_style = 0; }
	
	cmd(TAG_MASK(1));
	cmd(TAG(6));
	cmd_button(10,10, 80,30, 27,button_style, "Back");
	cmd(TAG_MASK(0));
	
	cmd(DISPLAY());
    cmd(CMD_SWAP);
	
	sysDms(20);
}
