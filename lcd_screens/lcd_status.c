/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    lcd_status.c
  * @brief   LCD Status Menu
  *          This file contains the functions for creating the display-list
  *          to draw and show the status menu.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "main.h"
#include "globals.h"

#include "ft800.h"
#include "lcd_status.h"

#include <stdio.h>

extern PeriphConnectionStruct 	PeriphConnection;
extern ModuleBTStruct 			ModuleBT;
extern ModuleTcpServerStruct	ModuleTcpServer;
extern ModuleRadioStruct 		ModuleRadio;
extern ModuleKNXStruct			ModuleKNX;

/*** DRAW STATUS SCREEN ********************************************************/
void draw_status_screen(const ButtonStruct* btnHome)
{
	u16 button_style = 0;
	char buf[18];
	
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
	cmd_text(240,25, 30,OPT_CENTER, "Status");
	
	/* Home button */
	if(btnHome->pressed)	{ cmd_fgcolor(0x000040); 	button_style = OPT_FLAT; }
	else				 	{ cmd_fgcolor(0x000080);	button_style = 0; }
	cmd(TAG_MASK(1));
	cmd(TAG(5));
	cmd_button(10,10, 80,30, 27,button_style, "Back");
	cmd(TAG_MASK(0));
	
	/* PeriphConnection Status informations */
	cmd(COLOR_RGB(0xb9,0x00,0x07));
	cmd(LINE_WIDTH(8));
	cmd(BEGIN(LINES));
	cmd(VERTEX2F(16*20, 16*95));
	cmd(VERTEX2F(16*220, 16*95));
	cmd(END());
	cmd_text(20,70, 28,0,  "Connections:");
	
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd_text(20,100, 26,0, "Ethernet Cable:");
	cmd_text(20,120, 26,0, "USB Cable:");
	cmd_text(20,150, 26,0, "MicroSD Card:");
	
	if(PeriphConnection.ethernet) 	{ cmd_text(125,100, 26,0, "Connected"); }
	else 							{ cmd_text(125,100, 26,0, "Not Connected"); }
	if(PeriphConnection.usb) 		{ cmd_text(125,120, 26,0, "Connected"); }
	else 							{ cmd_text(125,120, 26,0, "Not Connected"); }
	if(PeriphConnection.microsd) 	{ cmd_text(125,150, 26,0, "Inserted"); }
	else 							{ cmd_text(125,150, 26,0, "Not Present"); }
	
	/* Module information Texts */
	cmd(COLOR_RGB(0xb9,0x00,0x07));
	cmd(LINE_WIDTH(8));
	cmd(BEGIN(LINES));
	cmd(VERTEX2F(16*250, 16*95));
	cmd(VERTEX2F(16*450, 16*95));
	cmd(END());
	cmd_text(250,70, 28,0,  "Modules:");
	
	cmd(COLOR_RGB(0x10,0x20,0x30));
	cmd_text(250,100, 26,0, "Plugin:");
	cmd_text(250,120, 26,0, "Radio:");
	cmd_text(250,150, 26,0, "Bluetooth:");
	cmd_text(250,200, 26,0, "TCP Server:");
	
	cmd(COLOR_RGB(0x40,0x50,0x60));
	cmd_text(270,170, 26,0, "MAC Addr:");
	cmd_text(270,220, 26,0, "IP:");
	cmd_text(270,240, 26,0, "Port:");
	
	/* Module information: Plugin */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	switch(ModuleKNX.plugin)
	{
		case PLUGIN_RS485: 	cmd_text(410,100, 26,OPT_CENTERX, "RS-485");	break;
		case PLUGIN_KNX: 	cmd_text(410,100, 26,OPT_CENTERX, "KNX"); 		break;
		default:			cmd_text(410,100, 26,OPT_CENTERX, "OFF"); 		break;
	}
	
	/* Module information: Bluetooth */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	if(ModuleBT.state)
	{ 
		if(ModuleBT.state < BT_READY) 			{ cmd_text(410,150, 26,OPT_CENTERX, "Initializing..."); }
		else if(ModuleBT.state == BT_PAIRED)	{ cmd_text(410,150, 26,OPT_CENTERX, "Paired"); }
		else if(ModuleBT.state == BT_CONNECTED)	{ cmd_text(410,150, 26,OPT_CENTERX, "Connected"); }
		else 									{ cmd_text(410,150, 26,OPT_CENTERX, "Discoverable"); }
		   
		sprintf(buf, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c", 	ModuleBT.address[0],
														ModuleBT.address[1],
														ModuleBT.address[2],
														ModuleBT.address[3],
														ModuleBT.address[4],
														ModuleBT.address[5],
														ModuleBT.address[6],
														ModuleBT.address[7],
														ModuleBT.address[8],
														ModuleBT.address[9],
														ModuleBT.address[10],
														ModuleBT.address[11]);
		cmd(COLOR_RGB(0x40,0x50,0x60));
		cmd_text(345,170, 26,0, buf);
	}
	else { cmd_text(410,120, 26,OPT_CENTERX, "OFF"); }
	
	/* Module information: Radio */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	if(ModuleRadio.state)	{ cmd_text(410,120, 26,OPT_CENTERX, "ON"); }
	else { cmd_text(410,120, 26,OPT_CENTERX, "OFF"); }
	
	/* Module information: TCP Server */
	cmd(COLOR_RGB(0x10,0x20,0x30));
	if(ModuleTcpServer.state)	{ cmd_text(410,200, 26,OPT_CENTERX, "Listening"); }
	else						{ cmd_text(410,200, 26,OPT_CENTERX, "Not Listening"); }
	cmd(COLOR_RGB(0x40,0x50,0x60));
	sprintf(buf, "%u.%u.%u.%u", ModuleTcpServer.ip[0],
								ModuleTcpServer.ip[1],
								ModuleTcpServer.ip[2],
								ModuleTcpServer.ip[3]);
	cmd_text(345,220, 26,0, buf);
	sprintf(buf, "%u", ModuleTcpServer.port);
	cmd_text(345,240, 26,0, buf);
	
	cmd(DISPLAY());
    cmd(CMD_SWAP);
	
	sysDms(20);
}