/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    lcd_start.c
  * @brief   LCD Startup Screen
  *          This file contains the functions for creating the display-list
  *          to draw and show the startup screen.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "main.h"
#include "globals.h"

#include "ft800.h"
#include "lcd_start.h"

/*** DRAW STARTUP ANIMATION ********************************************************/
void draw_startup_animation(void)
{
	u16 i;
	for(i=0; i<250; ++i)
	{	
		cmd(CMD_DLSTART);
		cmd(CLEAR_COLOR_RGB(0,0,0));
		cmd(CLEAR(1,1,1));
		cmd_gradient(0,0,0xA1E1FF, 0,i,0x000080);
		cmd(DISPLAY());
		cmd(CMD_SWAP);
		
		sysDms(10);
	}
}

/*** DRAW START DEFAULT SCREEN *****************************************************/
void draw_start_screen(const ButtonStruct* btnContinue)
{
	cmd(CMD_DLSTART);
	cmd(CLEAR_COLOR_RGB(0,0,0));
	cmd(CLEAR(1,1,1));
	cmd_gradient(0,0,0xA1E1FF, 0,250,0x000080);
	cmd_text(10,245, 27,0, "Designed by: Akos Pasztor");
	cmd_text(470,250, 26,OPT_RIGHTX, "http://akospasztor.com");
	cmd(COLOR_RGB(0xDE,0x00,0x08));
	cmd_text(240,40, 31,OPT_CENTERX, "Home Control System");
	cmd(COLOR_RGB(255,255,255));
	cmd(TAG(1));
	
	if(!btnContinue->pressed)
	{
		cmd_fgcolor(0x228B22);
		cmd_button(130,150, 220,48, 28,0, "Tap to Continue");
	}
	else
	{
		cmd_fgcolor(0x0A520A);
		cmd_button(130,150, 220,48, 28,OPT_FLAT, "Tap to Continue");
	}
	
	cmd(DISPLAY());
	cmd(CMD_SWAP);	
}
