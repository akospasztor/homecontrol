/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    lcd_notifications.c
  * @brief   LCD Notifications Menu
  *          This file contains the functions for creating the display-list
  *          to draw and show the notifications menu screen.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "main.h"
#include "globals.h"
#include "linked_list.h"

#include "ft800.h"
#include "lcd_notifications.h"

#include <stdio.h>
#include <string.h>

extern ListStruct* notif_head;

/*** DRAW NOTIFICATIONS SCREEN *****************************************************/
void draw_notifications_screen(const ButtonStruct* btnHome, const ButtonStruct* btnClear)
{
	u16 button_style = 0;	
	uint8_t i;
	
	cmd(CMD_DLSTART);
	cmd(CLEAR_TAG(255));
	cmd(CLEAR_COLOR_RGB(1,68,33));
	cmd(CLEAR(1,1,1));
	cmd(BEGIN(RECTS));
	cmd(COLOR_RGB(1,68,33));
	cmd(COLOR_RGB(0x9E,0xD6,0xF0));
	cmd(LINE_WIDTH(5*16));
	cmd(VERTEX2F(16*13,  16*64));
	cmd(VERTEX2F(16*467, 16*258));
	cmd(END());
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,50));
	cmd_gradient(240,0,0xA0B0C0, 240,50, 0x506070);	
	cmd(SCISSOR_XY(0,0));
	cmd(SCISSOR_SIZE(480,272));
	cmd(COLOR_RGB(255,255,255));
	cmd_text(240,25, 30,OPT_CENTER, "Notifications");
	
	/* Notifications */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	
	ListStruct* iterator = notif_head;
	if(list_size(notif_head))
	{
		while(iterator)
		{
			cmd(COLOR_RGB(0x10,0x20,0x30));
			cmd_text(30, 75+(i*20),  26,0, iterator->date);
			cmd(COLOR_RGB(0xB9,0x00,0x07));
			cmd_text(180,75+(i*20),  26,0, iterator->msg);
			iterator = iterator->next;
			i++;
		}
	}
	else
	{
		cmd_text(240, 150, 28,OPT_CENTERX, "Yay, there aren't any notifications!");
	}
	
	/* Home button */
	cmd(COLOR_RGB(255,255,255));
	if(btnHome->pressed)	{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x000080);	button_style = 0; }
	cmd(TAG_MASK(1));
	cmd(TAG(5));
	cmd_button(10,10, 80,30, 27,button_style, "Back");
	cmd(TAG_MASK(0));
	
	/* Clear button */
	if(btnClear->pressed)	{ cmd_fgcolor(0x203040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x506070);	button_style = 0; }
	cmd(TAG_MASK(1));
	cmd(TAG(6));
	cmd_button(390,10, 80,30, 27,button_style, "Clear");
	cmd(TAG_MASK(0));
	
	cmd(DISPLAY());
    cmd(CMD_SWAP);
	
	sysDms(20);
}