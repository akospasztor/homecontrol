/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    lcd_date.c
  * @brief   LCD Set Date&Time Menu
  *          This file contains the functions for creating the display-list
  *          to draw and show the date&time menu screen.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "main.h"
#include "globals.h"

#include "ft800.h"
#include "lcd_date.h"

#include <stdio.h>

/*** STORE DATE DEFAULTS INTO FT800 MEM ********************************************/
void store_date_defaults(void)
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

/*** DRAW DATE SCREEN **************************************************************/
void draw_date_screen(const ButtonStruct* btnBack, const ButtonStruct* keys, RTC_DateTypeDef* date, RTC_TimeTypeDef* time)
{
	u16 button_style = 0;
	char buf[5];
	
	cmd(CMD_DLSTART);
	cmd(CMD_APPEND); cmd(RAM_G); cmd(11*4);
	cmd_gradient(240,0,0xA0B0C0, 240,50, 0x506070);	
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,272));
	cmd(COLOR_RGB(255,255,255));
	cmd_text(240,25, 30,OPT_CENTER, "Set Date & Time");
	
	/* Year */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	sprintf(buf, "20%02u", date->RTC_Year);
	cmd_text(67,155, 30,OPT_CENTERX, buf);
	cmd_text(67,70,  26,OPT_CENTERX, "Year");
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG_MASK(1));
	if(keys->pressed && keys->tag == 11) { cmd_fgcolor(0x004000); 	button_style = OPT_FLAT; }
	else				 				 { cmd_fgcolor(0x008000);	button_style = 0; }
	cmd(TAG(11));
	cmd_button(47,105, 40,40, 27,button_style, "+");
	if(keys->pressed && keys->tag == 10) { cmd_fgcolor(0x004000); 	button_style = OPT_FLAT; }
	else				 				 { cmd_fgcolor(0x008000);	button_style = 0; }
	cmd(TAG(10));
	cmd_button(47,200, 40,40, 27,button_style, "-");
	cmd(TAG_MASK(0));
	
	/* Month */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	sprintf(buf, "%u", date->RTC_Month);
	cmd_text(144,155, 30,OPT_CENTERX, buf);
	cmd_text(144,70,  26,OPT_CENTERX, "Month");
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG_MASK(1));
	if(keys->pressed && keys->tag == 21) { cmd_fgcolor(0x004000); 	button_style = OPT_FLAT; }
	else				 				 { cmd_fgcolor(0x008000);	button_style = 0; }
	cmd(TAG(21));
	cmd_button(124,105, 40,40, 27,button_style, "+");
	if(keys->pressed && keys->tag == 20) { cmd_fgcolor(0x004000); 	button_style = OPT_FLAT; }
	else				 				 { cmd_fgcolor(0x008000);	button_style = 0; }
	cmd(TAG(20));
	cmd_button(124,200, 40,40, 27,button_style, "-");
	cmd(TAG_MASK(0));
	
	/* Day */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	sprintf(buf, "%u", date->RTC_Date);
	cmd_text(221,155, 30,OPT_CENTERX, buf);
	cmd_text(221,70,  26,OPT_CENTERX, "Day");
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG_MASK(1));
	if(keys->pressed && keys->tag == 31) { cmd_fgcolor(0x004000); 	button_style = OPT_FLAT; }
	else				 				 { cmd_fgcolor(0x008000);	button_style = 0; }
	cmd(TAG(31));
	cmd_button(201,105, 40,40, 27,button_style, "+");
	if(keys->pressed && keys->tag == 30) { cmd_fgcolor(0x004000); 	button_style = OPT_FLAT; }
	else				 				 { cmd_fgcolor(0x008000);	button_style = 0; }
	cmd(TAG(30));
	cmd_button(201,200, 40,40, 27,button_style, "-");
	cmd(TAG_MASK(0));
	
	/* Hour */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	sprintf(buf, "%02u", time->RTC_Hours);
	cmd_text(335,155, 30,OPT_CENTERX, buf);
	cmd_text(335,70,  26,OPT_CENTERX, "Hour");
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG_MASK(1));
	if(keys->pressed && keys->tag == 41) { cmd_fgcolor(0x004000); 	button_style = OPT_FLAT; }
	else				 				 { cmd_fgcolor(0x008000);	button_style = 0; }
	cmd(TAG(41));
	cmd_button(315,105, 40,40, 27,button_style, "+");
	if(keys->pressed && keys->tag == 40) { cmd_fgcolor(0x004000); 	button_style = OPT_FLAT; }
	else				 				 { cmd_fgcolor(0x008000);	button_style = 0; }
	cmd(TAG(40));
	cmd_button(315,200, 40,40, 27,button_style, "-");
	cmd(TAG_MASK(0));
	
	/* Min */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	sprintf(buf, "%02u", time->RTC_Minutes);
	cmd_text(412,155, 30,OPT_CENTERX, buf);
	cmd_text(412,70,  26,OPT_CENTERX, "Min");
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG_MASK(1));
	if(keys->pressed && keys->tag == 51) { cmd_fgcolor(0x004000); 	button_style = OPT_FLAT; }
	else				 				 { cmd_fgcolor(0x008000);	button_style = 0; }
	cmd(TAG(51));
	cmd_button(392,105, 40,40, 27,button_style, "+");
	if(keys->pressed && keys->tag == 50) { cmd_fgcolor(0x004000); 	button_style = OPT_FLAT; }
	else				 				 { cmd_fgcolor(0x008000);	button_style = 0; }
	cmd(TAG(50));
	cmd_button(392,200, 40,40, 27,button_style, "-");
	cmd(TAG_MASK(0));
	
	/* Home button */
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG_MASK(1));
	if(btnBack->pressed)	{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x000080);	button_style = 0; }	
	cmd(TAG(5));
	cmd_button(10,10, 80,30, 27,button_style, "Back");
	cmd(TAG_MASK(0));
	
	cmd(DISPLAY());
    cmd(CMD_SWAP);
	
	sysDms(20);
}
