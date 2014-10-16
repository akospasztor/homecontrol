/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    lcd_sd.c
  * @brief   LCD microSD Messages
  *          This file contains the functions for creating the display-list
  *          to draw and show the microSD messages.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "main.h"
#include "globals.h"

#include "ft800.h"
#include "lcd_sd.h"

/*** DRAW SD WARNING SCREEN ********************************************************/
void draw_sd_warning_screen(char* msg, const ButtonStruct* btnOk)
{
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
	cmd(VERTEX2F(16*130, 16*90));
	cmd(VERTEX2F(16*350, 16*90));
	cmd(END());
	cmd(COLOR_RGB(0xB9,0x00,0x07));
	cmd_text(240,40, 31,OPT_CENTERX, "Warning!");	
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd_text(120,110, 27,0, msg);
	cmd_text(120,130, 27,0, "Note: Default values will be loaded.");
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG(1));
	
	if(!btnOk->pressed)
	{
		cmd_fgcolor(0x0000C0);
		cmd_button(190,170, 100,48, 28,0, "OK");
	}
	else
	{
		cmd_fgcolor(0x000040);
		cmd_button(190,170, 100,48, 28,OPT_FLAT, "OK");
	}
	
	cmd(DISPLAY());
	cmd(CMD_SWAP);
}