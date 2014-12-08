/**
  ******************************************************************************
  * Home Automation System - Central Unit 
  ******************************************************************************
  * @author  Akos Pasztor    (http://akospasztor.com)
  * @file    globals.c
  * @brief   Global Resources & Implementations
  *          This file contains the global structures, variables, flags
  *          and functions for the whole software.
  ******************************************************************************
  * Copyright (c) 2014 Akos Pasztor. All rights reserved.
  ******************************************************************************
**/

#include "stm32f4xx.h"
#include "globals.h"
#include "main.h"
#include "linked_list.h"

#include "ff.h"
#include "diskio.h"
#include "rfm73.h"
#include "usart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*** VARIABLES *********************************************************************/
RTC_TimeTypeDef Rtime;
RTC_DateTypeDef Rdate;

uint8_t EnableAlerts = 1;			/* 1: (default) enable displaying alerts | 0: disable displaying alerts */
ListStruct* notif_head = NULL;		/* head pointer of notifications list */

char message[MSG_MAX+1];	/* output buffer for communication */

char rxRF[BUF_MAX+1];		/* dedicated RFM73P rx buffer */
uint8_t rxRFlength;			/* data length of rxBT */

char rxKNX[BUF_MAX+1];		/* dedicated KNX/RS-485 buffer */
uint8_t rxKNXptr = 0;

char rxBT[BUF_MAX+1];		/* dedicated BT rx buffer */
char rBT[BUF_MAX+1];		/* dedicated BT response buffer: if rxBT contains valid response, then the useful response is copied here */
uint8_t rxBTptr = 0;

PeriphConnectionStruct 	PeriphConnection;
ModuleBTStruct 			ModuleBT;
ModuleTcpServerStruct	ModuleTcpServer;
ModuleRadioStruct 		ModuleRadio;
ModuleKNXStruct			ModuleKNX;

ModeStruct Mode;
SensorStruct Sensor1;
SensorStruct Sensor2;
SensorStruct Sensor3;
SensorStruct* SensorFav;

/*** GENERIC FLAGS *****************************************************************/
uint8_t  FLAG_dispAlert = 0;				/* set to 1: when alert screen is being displayed */

/*** INTERRUPT FLAGS ***************************************************************/
uint8_t  FLAG_rfm_rx = 0;					/* set to 1: when something is in rfm73p rx buffer */
uint8_t  FLAG_datetime = 0;					/* set to 1: when need to obtain date & time */
uint8_t  FLAG_getPeriphConnection = 0;		/* set to 1: when need to get the connection status of the peripherials */
uint8_t  FLAG_pluginType = 0;				/* set to 1: when need to get the RS-485/KNX plugin type */
uint8_t  FLAG_inputIndicator = 0;			/* set to 1: when need to flash indicatior in input fields */
uint8_t  FLAG_beepAlert = 0;				/* set to 1: when need to make beep for alerts | 2: sound is being played */
uint16_t FLAG_usbConnection = 0;			/* set to 1000: when usb is connected (USB OTG IT handler periodically called during connection). In every System Timer IT the value decreases. When reach 0, there is no USB connection. */

/*** INTERRUPT VARIABLES ***********************************************************/
uint8_t rxBTchar 	= 0x00;
uint8_t rxKNXchar 	= 0x00;


/*** MAIN TASKS ********************************************************************/
void processTasks(void)
{	
	/* Get Plugin Type */
	if(FLAG_pluginType)
	{	
		getPluginType(&ModuleKNX);
		FLAG_pluginType = 0;
	}	
	
	/* Radio: MAX_RT clear */
	uint8_t rfstatus = rfm_reg_read(RFM73_REG_STATUS);
	if(rfstatus & (1<<4)) { rfm_reg_write(RFM73_REG_STATUS, rfstatus); } 
	
	/* POLL MODULES - Receive Answer (KNX | RS-485) */
	if(Sensor1.FLAG_poll == POLL_WAIT ||
	   Sensor2.FLAG_poll == POLL_WAIT ||
	   Sensor3.FLAG_poll == POLL_WAIT )
	{
		if(isReadyKNXResponse())
		{
			if(isValidSensorData(rxKNX, rxKNXptr)) { fillSensorData(rxKNX); }
		}
	}
	
	/* POLL MODULES - Receive Answer (Radio) */
	if(FLAG_rfm_rx)								
	{
		memset(rxRF, '\0', sizeof(rxRF));
		rfm_rec(rxRF, &rxRFlength);				/* receive all data from radio not to block communication */
		
		if(Sensor1.FLAG_poll == POLL_WAIT ||
		   Sensor2.FLAG_poll == POLL_WAIT ||
		   Sensor3.FLAG_poll == POLL_WAIT )
		{	
			if(isValidSensorData(rxRF, rxRFlength)) { fillSensorData(rxRF); }
		}
		
		FLAG_rfm_rx = 0;
	}
	
	/* POLL MODULES - Send Request */
	if(Sensor1.FLAG_poll == POLL_REQ)		/* Module 1 */
	{
		fillMsgPoll(1);
		switch(Sensor1.com)
		{
			case COM_RF:
				ledOn(LEDy);
				rfm_set_addr(Sensor1.addr);
				rfm_mode_tx();
				rfm_send(message,MSG_MAX);
				rfm_mode_rx();	
				ledOff(LEDy);
			break;
			
			case COM_TP:
				txEN485();
				KNX_send_str(message);
				sysDms(5);
				rxEN485();
			break;
		}
		Sensor1.FLAG_poll = POLL_WAIT;
	}
	
	if(Sensor2.FLAG_poll == POLL_REQ)		/* Module 2 */
	{
		fillMsgPoll(2);		
		switch(Sensor2.com)
		{
			case COM_RF:
				ledOn(LEDy);
				rfm_set_addr(Sensor2.addr);
				rfm_mode_tx();
				rfm_send(message,MSG_MAX);
				rfm_mode_rx();
				ledOff(LEDy);
			break;
			
			case COM_TP:
				txEN485();
				KNX_send_str(message);
				sysDms(5);
				rxEN485();
			break;
		}
		Sensor2.FLAG_poll = POLL_WAIT;
	}
	
	if(Sensor3.FLAG_poll == POLL_REQ)		/* Module 3 */
	{
		fillMsgPoll(3);		
		switch(Sensor3.com)
		{
			case COM_RF:
				ledOn(LEDy);
				rfm_set_addr(Sensor3.addr);
				rfm_mode_tx();
				rfm_send(message,MSG_MAX);
				rfm_mode_rx();
				ledOff(LEDy);
			break;
			
			case COM_TP:
				txEN485();
				KNX_send_str(message);
				sysDms(5);
				rxEN485();
			break;
		}
		Sensor3.FLAG_poll = POLL_WAIT;
	}
	
	/* Bluetooth */
	if(ModuleBT.state == BT_OFF && !ModuleBT.errcount)
	{ 
		ModuleBT.state = 1;
		ModuleBT.cmdsent = 0;
	}
	else
	{
		/* check error number */
		if(ModuleBT.errcount > 2) { ModuleBT.state = BT_OFF; /*error*/ }
		
		/* BT state machine */
		switch(ModuleBT.state)
		{
			case 1:
				if(!ModuleBT.cmdsent)
				{
					ModuleBTSend("AT");
				}
				else
				{
					uint8_t response = ModuleBTResponse();
					if(response && response == 0xFF)
					{
						ModuleBT.state++;
						ModuleBT.cmdsent = 0;
					}
					else if(response)
					{
						ModuleBT.cmdsent = 0;
					}
					   
				}
			break;
			
			case 2:
				if(!ModuleBT.cmdsent)
				{
					ModuleBTSend("AT&F*");
				}
				else
				{
					uint8_t response = ModuleBTResponse();
					if(response && response == 0xFF)
					{
						ModuleBT.state++;
						ModuleBT.cmdsent = 0;
					}
					else if(response)
					{
						ModuleBT.cmdsent = 0;
					}
					   
				}
			break;
			
			case 3:
				if(!ModuleBT.cmdsent)
				{
					ModuleBTSend("ATS102=$1");
				}
				else
				{
					uint8_t response = ModuleBTResponse();
					if(response && response == 0xFF)
					{
						ModuleBT.state++;
						ModuleBT.cmdsent = 0;
					}
					else if(response)
					{
						ModuleBT.cmdsent = 0;
					}
					   
				}
			break;
			
			case 4:
				if(!ModuleBT.cmdsent)
				{
					ModuleBTSend("ATS512=4");
				}
				else
				{
					uint8_t response = ModuleBTResponse();
					if(response && response == 0xFF)
					{
						ModuleBT.state++;
						ModuleBT.cmdsent = 0;
					}
					else if(response)
					{
						ModuleBT.cmdsent = 0;
					}
					   
				}
			break;
			
			case 5:
				if(!ModuleBT.cmdsent)
				{
					ModuleBTSend("AT&W");
				}
				else
				{
					uint8_t response = ModuleBTResponse();
					if(response && response == 0xFF)
					{
						ModuleBT.state++;
						ModuleBT.cmdsent = 0;
					}
					else if(response)
					{
						ModuleBT.cmdsent = 0;
					}
					   
				}
			break;
			
			case 6:
				if(!ModuleBT.cmdsent)
				{
					ModuleBTSend("ATZ");					
				}
				else
				{
					uint8_t response = ModuleBTResponse();
					if(response && response == 0xFF)
					{
						ModuleBT.state++;
						ModuleBT.cmdsent = 0;
					}
					else if(response)
					{
						ModuleBT.cmdsent = 0;
					}
					   
				}
			break;
			
			case 7:
				if(!ModuleBT.cmdsent)
				{
					ModuleBTSend("ATE0");
				}
				else
				{
					uint8_t response = ModuleBTResponse();
					if(response && response == 0xFF)
					{
						ModuleBT.state++;
						ModuleBT.cmdsent = 0;
					}
					else if(response)
					{
						ModuleBT.cmdsent = 0;
					}
					   
				}
			break;
			
			case 8:
				if(!ModuleBT.cmdsent)
				{
					ModuleBTSend("AT+BTP");
				}
				else
				{
					uint8_t response = ModuleBTResponse();
					if(response && response == 0xFF)
					{
						ModuleBT.state++;
						ModuleBT.cmdsent = 0;
					}
					else if(response)
					{
						ModuleBT.cmdsent = 0;
					}
					   
				}
			break;
			
			case 9:
				if(!ModuleBT.cmdsent)
				{
					ModuleBTSend("ATI4");
				}
				else
				{
					uint8_t response = ModuleBTResponse();
					if(response && response == 0xFF)
					{
						strcpy(ModuleBT.address, rBT);		/* get BT MAC addr. */
						ModuleBT.state = BT_READY;
						ModuleBT.cmdsent = 0;
					}
					else if(response)
					{
						ModuleBT.cmdsent = 0;
					}
					   
				}
			break;
			
			case BT_READY:
			{
				uint8_t pairresponse = ModuleBTPair();
				if(pairresponse)
				{
					ModuleBT.state = BT_PAIRED;
				}
			}
			break;
		}	/*switch*/
		
	} /*ModuleBT*/
		
}

/*** FUNCTIONS *********************************************************************/
/* POLLING: fill message buffer */
void fillMsgPoll(uint8_t addr)
{
	uint8_t i;
	
	memset(message, '\0', sizeof(message));
	message[0]  = '#';			/* Lead-in */
	message[1]  = (addr+48);	/* Destination address */
	message[2]  = '0';			/* Sender address */
	message[3]  = '0';			/* Relay 1 */
	message[4]  = '0';			/* Relay 2 */
	message[5]  = '0';			/* Controllable LED */
	message[6]  = '0';			/* Temp-msb (dummy) */
	message[7]  = '0';			/* Temp-lsb (dummy) */
	message[8]  = '0';			/* Sen1-msb (dummy) */
	message[9]  = '0';			/* Sen1-lsb (dummy) */
	message[10] = '0';			/* Sen2-msb (dummy) */
	message[11] = '0';			/* Sen2-lsb (dummy) */
	message[12] = '0';			/* Humidity (dummy) */
	message[13] = 0;			/* CHKS */
	for(i=0; i<MSG_MAX-2; ++i)
	{
		message[13] += message[i];
	}
	message[14] = '*';			/* Termination */
}

/* POLLING: fill Sensor struct with received data AND check data for alerts */
void fillSensorData(const char* data)
{
	SensorStruct* SensorSelected = SensorFav;	/* default: favorite sensor */
	switch(data[2])
	{
		case '1': SensorSelected = &Sensor1; break;
		case '2': SensorSelected = &Sensor2; break;
		case '3': SensorSelected = &Sensor3; break;
	}
	/* update sensor data */
	setSensor(SensorSelected, 	(float)data[6] + (float)data[7]/10,		/* temp */
								(data[12]),								/* humid */
								(data[8] << 8) + data[9],				/* co */
								(data[10] << 8) + data[11] );			/* co2 */
	/* reset error counter */
	SensorSelected->errcount = 0;
	
	/* reset flag */
	SensorSelected->FLAG_poll = POLL_RDY;
	
	/* check Sensor Data for Alerts */
	checkSensorForAlert(SensorSelected);
}

/* POLLING: check if received data is valid */
uint8_t isValidSensorData(const char* data, const uint8_t length)
{
	/* Check Length */
	if(!length || length > MSG_MAX) { return 0; }
	
	/* Check CHKS */
	uint8_t i, chks = 0;
	for(i=0; i<(length-2); ++i)
	{ 
		chks += data[i];
	}
	if(chks != data[length-2] || data[length-1] != '*') { return 0; }
	
	/* Check Address */
	if(data[1] != '0') { return 0; }
	
	/* Check Sender Address */
	if(data[2] < '1' || data[2] > '3') { return 0; }
	
	return 1;
}

/* Sensor */
void initSensor(SensorStruct* sensor, char* name, float target)
{
	strcpy(sensor->name, name);
	sensor->target = target;
	
	sensor->alert = 0;
	sensor->alertTimer = 0;
	
	sensor->aTmax = 28.0;
	sensor->aTmin = 18.0;
	sensor->aHmax = 70;
	sensor->aHmin = 30;
	sensor->aCO = 100;
	sensor->aCO2 = 400;
	
	sensor->com = COM_OFF;
	sensor->FLAG_poll = 0;
	sensor->addr[0] = 0xE7;
	sensor->addr[1] = 0xE7;
	sensor->addr[2] = 0xE7;
	sensor->addr[3] = 0xE7;
	sensor->addr[4] = 0xE7;
}

void nameSensor(SensorStruct* sensor, char* name)
{
	strcpy(sensor->name, name);
}

void setSensor(SensorStruct* sensor, float temp, uint8_t humid, uint16_t co, uint16_t co2)
{
	sensor->temp = temp;
	sensor->humid = humid;
	sensor->co = co;
	sensor->co2 = co2;
}

void setSensorAlert(SensorStruct* sensor, float target, float aTmax, float aTmin, uint8_t aHmax, uint8_t aHmin, uint16_t aCO, uint16_t aCO2)
{
	sensor->target = target;
	sensor->aTmax = aTmax;
	sensor->aTmin = aTmin;
	sensor->aHmax = aHmax;
	sensor->aHmin = aHmin;
	sensor->aCO = aCO;
	sensor->aCO2 = aCO2;
}

void setSensorCom(SensorStruct* sensor, uint8_t com, char addr_lsb)
{
	sensor->com = com;
	sensor->FLAG_poll = 0;
	sensor->errcount = 0;
	sensor->addr[0] = addr_lsb;
}

/* Alert functions */
void checkSensorForAlert(SensorStruct* sensor)
{
	char buf[LIST_MSG_MAX+1];
	
	/* TEMP High */
	if((sensor->temp > sensor->aTmax) && (sensor->temp > 0.0))
	{ 
		setAlert(sensor, ALERT_TEMP_H);
		sprintf(buf, "Temp. is too high (%.1f C) in: %s", sensor->temp, sensor->name);
		addNotif(buf);
	} 
	else { clrAlert(sensor, ALERT_TEMP_H); }
	
	/* TEMP Low */
	if((sensor->temp < sensor->aTmin) && (sensor->temp > 0.0))
	{ 
		setAlert(sensor, ALERT_TEMP_L);
		sprintf(buf, "Temp. is too low (%.1f C) in: %s", sensor->temp, sensor->name);
		addNotif(buf);
	} 
	else { clrAlert(sensor, ALERT_TEMP_L); }
	
	/* HUMID High */
	if((sensor->humid > sensor->aHmax) && (sensor->humid > 0))
	{
		setAlert(sensor, ALERT_HUM_H);
		sprintf(buf, "Humidity is too high (%u %%) in: %s", sensor->humid, sensor->name);
		addNotif(buf);
	} 
	else { clrAlert(sensor, ALERT_HUM_H); }
	
	/* HUMID Low */
	if((sensor->humid < sensor->aHmin) && (sensor->humid > 0))
	{
		setAlert(sensor, ALERT_HUM_L);
		sprintf(buf, "Humidity is too low (%u %%) in: %s", sensor->humid, sensor->name);
		addNotif(buf);
	} 
	else { clrAlert(sensor, ALERT_HUM_L); }
	
	/* CO High */
	if((sensor->co  > sensor->aCO)  && (sensor->aCO  > 0))
	{
		setAlert(sensor, ALERT_CO);	
		sprintf(buf, "CO level is too high (%u ppm) in: %s", sensor->co, sensor->name);
		addNotif(buf);
	} 
	else { clrAlert(sensor, ALERT_CO);  }
	
	/* CO2 High */
	if((sensor->co2 > sensor->aCO2) && (sensor->aCO2 > 0))
	{ 
		setAlert(sensor, ALERT_CO2);
		sprintf(buf, "CO2 level is too high (%u ppm) in: %s", sensor->co2, sensor->name);
		addNotif(buf);
	}
	else { clrAlert(sensor, ALERT_CO2); }
}

void setAlert(SensorStruct* sensor, uint8_t alert)
{
	/* if there is a new alert, then clear Bit0 and alertTimer variable to force-show new alert */
	if(!checkAlert(sensor, alert))
	{
		clrAlertStateBit(sensor);
		sensor->alertTimer = 0;
	}
	
	/* set proper alert bit */
	sensor->alert |= (1<<alert);
}
		
uint8_t checkAlert(SensorStruct* sensor, uint8_t alert)
{
	return (sensor->alert & (1<<alert));
}

void clrAlert(SensorStruct* sensor, uint8_t alert)
{
	sensor->alert &= ~(1<<alert);
}

void clrAlertAll(SensorStruct* sensor)
{
	sensor->alert &= 0x01;
}

uint8_t isAlert(SensorStruct* sensor)
{
	/* Do not display when Bit0 is 1 OR alertTimer is counting OR Sensor is off*/
	if(sensor->alert & 0x01 || sensor->alertTimer || !sensor->com) { return 0; }
	
	/* If there is alert, return 1 */
	if(sensor->alert > 0) { return 1; }
	
	return 0;
}

void setAlertStateBit(SensorStruct* sensor)
{
	sensor->alert |= 0x01;
}

void clrAlertStateBit(SensorStruct* sensor)
{
	sensor->alert &= 0xFE;
}

/* Notification Messages */
void addNotif(char* msg)
{	
	char date_buf[LIST_DATE_MAX+1];
	
	if(list_size(notif_head) >= NOTIF_MAX)
	{
		list_popback(notif_head);
	}
	sprintf(date_buf, "20%u/%u/%u %02u:%02u:%02u", Rdate.RTC_Year, Rdate.RTC_Month, Rdate.RTC_Date, Rtime.RTC_Hours, Rtime.RTC_Minutes, Rtime.RTC_Seconds);
	list_pushfront(&notif_head, date_buf, msg);	
}

void clearNotif(void)
{
	list_clear(&notif_head);
}

/* Buffer KNX */
void bufKNXClr(void)
{
	memset(rxKNX, '\0', BUF_MAX+1);
	rxKNXptr = 0;
}

uint8_t bufKNXPush(const char c)
{
	uint8_t ov = 0;
	if(c == '#') { rxKNXptr = 0; }
	if(rxKNXptr >= BUF_MAX) { ov = 1; rxKNXptr = 0; }
	rxKNX[rxKNXptr++] = c;
	return ov;
}

uint8_t isReadyKNXResponse(void)
{
	return (rxKNX[rxKNXptr-1] == '*') ? 1 : 0;
}


/* Buffer BT */
void bufBTClr(void)
{
	memset(rxBT, '\0', BUF_MAX+1);
	rxBTptr = 0;
}

void rBTClr(void)
{
	memset(rBT, '\0', BUF_MAX+1);
}

char bufBTPop(void)
{
	if(rxBTptr < 1) { return 0; }
	char pop = rxBT[--rxBTptr];
	rxBT[rxBTptr] = '\0';
	return pop;
}

uint8_t bufBTPush(const char c)
{
	uint8_t ov = 0; 
	if(rxBTptr >= BUF_MAX) { ov = 1; rxBTptr = 0; }
	rxBT[rxBTptr++] = c;
	return ov;
}

uint8_t bufBTCRLF(void)
{
	if(rxBTptr < 3) { return 0; }
	return ( rxBT[rxBTptr-2] == 0x0D && rxBT[rxBTptr-1] == 0x0A ) ? 1 : 0;
}

/* PeriphConnection: get connection status */
void getPeriphConnection(PeriphConnectionStruct* PeriphConnection)
{
	/* USB Connection */
	if(FLAG_usbConnection) { PeriphConnection->usb = 1; }
	else { PeriphConnection->usb = 0; }
	
	/* microSD card */
	FATFS FatFs;
	if( !f_mount(&FatFs, "", 1) )
	{
		PeriphConnection->microsd = 1;		/* mount success */
		f_mount(NULL, "", 1);
	}
	else { PeriphConnection->microsd = 0; }	/* mount fail */
	sdc_deassert();							/* force release SD ChipSelect */
}

/* PeriphConnection Struct init */
void PeriphConnectionStructInit(PeriphConnectionStruct* PeriphConnection)
{
	PeriphConnection->ethernet = 0;
	PeriphConnection->microsd = 0;
	PeriphConnection->usb = 0;
}

/* PeriphModules: Bluetooth Struct Init */
void ModuleBTStructInit(ModuleBTStruct* ModuleBT)
{
	ModuleBT->state = BT_OFF;
	ModuleBT->cmdsent = 0;
	ModuleBT->errcount = 0;
	ModuleBT->address[12] = '\0';
	strcpy(ModuleBT->address, "000000000000");
	ModuleBT->pairaddr[12] = '\0';
	strcpy(ModuleBT->pairaddr, "000000000000");
}

/* PeriphModules: Bluetooth Send Command */
void ModuleBTSend(unsigned char* cmd)
{
	rBTClr();
	BT_send_cmd(cmd);
	ModuleBT.cmdsent = 1;
}

/* PeriphModules: Bluetooth Response */
uint8_t ModuleBTResponse(void)
{
	if(!bufBTCRLF()) { return 0; }
	
	/* check if response is correct */
	const char msgOK[6] = {0x0D, 0x0A, 'O', 'K', 0x0D, 0x0A};
	char* pos = strstr(rxBT, msgOK);
	if(pos)
	{
		uint8_t i,q = 0;						/* copy useful response into rBT */
		memset(rBT, '\0', BUF_MAX+1);
		for(i=0, q=0; &rxBT[i] != pos; ++i)
		{
			if(rxBT[i] != 0x0D && rxBT[i] != 0x0A) 	{ rBT[q++] = rxBT[i]; }
		}
		ModuleBT.errcount = 0;				/* clear error numbers on OK */
		bufBTClr();							/* clear rxBT buffer */
		return 0xFF;						/* return 0xFF to show there is OK response and there is new data in rBT */
	}
	
	/* check if response error */
	if(strstr(rxBT, "ERROR "))
	{
		pos = strrchr(rxBT, ' ');			/* search for last ' ' space. after space there is the error number */
		if(pos)
		{
			uint8_t i = 0;
			char errorNum[] = {'\0', '\0', '\0', '\0'};
			for(i=0; i<3; ++i)
			{
				errorNum[i] = *(pos+1+i);
			}
			ModuleBT.errcount++;			/* increase error numbers on ERROR */
			bufBTClr();						/* clear rxBT buffer */
			return strtoul(errorNum, NULL, 0);
		}
	}
	
	return 0;
}

/* PeriphModules: Bluetooth Pair */
uint8_t ModuleBTPair(void)
{
	if(!bufBTCRLF()) { return 0; }
	
	/* check if pair */
	char* pos = strstr(rxBT, "PAIR ");
	if(pos)
	{
		uint8_t i = 0;
		pos = strchr(rxBT, ' ');					/* search for first ' ' space. after space there is the pair status number */
		uint8_t pairstatus = ( *(pos+1) - '0' );	/* convert ascii char digit to number */
		if(!pairstatus)
		{
			for(i=0; i<12; ++i)						/* get mac address of paired device */
			{
				ModuleBT.pairaddr[i] = *(pos+3+i);
			}
			ModuleBT.pairaddr[12] = '\0';
			bufBTClr();								/* clear rxBT buffer */
			return 0xFF;							/* return 0xFF on successful pairing */
		}
		else
		{
			bufBTClr();				/* clear rxBT buffer */
			return pairstatus;		/* on pair error return pair error number */
		}
	}
	return 0;
}

/* PeriphModules: TcpServer */
void ModuleTcpServerStructInit(ModuleTcpServerStruct* ModuleTcpServer)
{
	ModuleTcpServer->state = 0;
	ModuleTcpServer->ip[0] = 192;
	ModuleTcpServer->ip[1] = 168;
	ModuleTcpServer->ip[2] = 1;
	ModuleTcpServer->ip[3] = 110;
	ModuleTcpServer->port = 10001;
}

/* PeriphModules: Radio Struct Init*/
void ModuleRadioStructInit(ModuleRadioStruct* ModuleRadio)
{
	ModuleRadio->state = 0;
	ModuleRadio->address[0] = 0xE7;
	ModuleRadio->address[1] = 0xE7;
	ModuleRadio->address[2] = 0xE7;
	ModuleRadio->address[3] = 0xE7;
	ModuleRadio->address[4] = 0xE7;
}

/* PeriphModules: Plugin */
void ModuleKNXStructInit(ModuleKNXStruct* ModuleKNX)
{
	ModuleKNX->state = 0;
	ModuleKNX->plugin = PLUGIN_OFF;
}

/* PeriphModules: Plugin: get state */
void getPluginType(ModuleKNXStruct* ModuleKNX)
{
	uint8_t isOff = 0;
	GPIO_InitTypeDef gpio;
	
	gpio.GPIO_Pin = GPIO_Pin_12;
	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &gpio);
	sysDms(1);
	if( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12) ) { isOff++; }
	
	gpio.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOD, &gpio);
	sysDms(1);
	if( !GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12) ) { isOff++; }
	if(isOff == 2) { ModuleKNX->plugin = PLUGIN_OFF; return; }
	
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &gpio);
	ModuleKNX->plugin = (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12)) ? PLUGIN_RS485 : PLUGIN_KNX;	
}

/* microSD */
void writeConfigToSD(void)
{
	FATFS FatFs;
	FIL fil;
	if( !f_mount(&FatFs, "", 1) )
	{	
		if( !f_open(&fil, "config.txt", FA_CREATE_ALWAYS | FA_WRITE) )
		{
			uint8_t cntr;
			char buf[5];
			
			/* write MODE */
			if(Mode.type == COOLING) { f_puts("COOLING\n", &fil); }
			else 					 { f_puts("HEATING\n", &fil); }
			
			/* write EnableAlerts */
			if(EnableAlerts) { f_puts("ALERT_ENABLE\n", &fil); }
			else 			 { f_puts("ALERT_DISABLE\n", &fil); }
			
			/* write Sensors Data */
			for(cntr=0; cntr<3; ++cntr) 
			{
				SensorStruct* sensor;
				switch(cntr)
				{
					case 0: sensor = &Sensor1; break;
					case 1: sensor = &Sensor2; break;
					case 2: sensor = &Sensor3; break;
				}
				
					   
				/* name */
				f_puts(sensor->name, &fil); f_puts("\n", &fil);
				
				/* communication */
				if(sensor->com == COM_RF) 	   { f_puts("COM_RF\n", &fil); }
				else if(sensor->com == COM_TP) { f_puts("COM_TP\n", &fil); }
				else 						   { f_puts("COM_OFF\n", &fil); }
				
				/* target temp */
				sprintf(buf, "%.1f\n", sensor->target);
				f_puts(buf, &fil);
				
				/* alerts: T_H, T_L, H_H, H_L, CO, CO2 (6) */
				sprintf(buf, "%.1f\n", sensor->aTmax);	f_puts(buf, &fil);
				sprintf(buf, "%.1f\n", sensor->aTmin);	f_puts(buf, &fil);
				sprintf(buf, "%u\n", sensor->aHmax);	f_puts(buf, &fil);
				sprintf(buf, "%u\n", sensor->aHmin);	f_puts(buf, &fil);
				sprintf(buf, "%u\n", sensor->aCO);		f_puts(buf, &fil);
				sprintf(buf, "%u\n", sensor->aCO2);		f_puts(buf, &fil);
			}
			
			/* close file */
			f_close(&fil);
		}
	}
}

void removeCRLF(char* str)
{
	uint8_t i;
	for(i=0; str[i] != '\0'; ++i)
	{
		if(str[i] == 0x0D || str[i] == 0x0A) { str[i] = '\0'; }
	}
}

/* Touch Tag */
uint8_t tagIsValid(uint32_t tag)
{
	uint8_t tag_last = (uint8_t)(tag & 0xFF);
	if( (tag_last == ((tag>>8) &0xFF)) &&
	    (tag_last == ((tag>>16)&0xFF)) &&
		(tag_last == ((tag>>24)&0xFF)) )
	{
		return 1;
	}
	return 0;
}

/* Button */
uint8_t buttonIsClicked(const ButtonStruct* btn, const uint32_t tag)
{
	return (btn->pressed && !tag) ? 1 : 0;
}

/* Slider */
void sliderInit(SliderStruct* slider, uint16_t value)
{
	uint8_t i;
	slider->index = 0;
	slider->value = value;
	for(i=0; i<SLIDER_AVG; ++i) { slider->avg[i] = value; }
}

void sliderPush(SliderStruct* slider, uint16_t value)
{
	slider->avg[slider->index] = value;
	slider->index = (slider->index >= SLIDER_AVG-1) ? 0 : (slider->index + 1);
}

uint16_t sliderAvg(SliderStruct* slider)
{
	uint8_t i;
	uint32_t sum = 0;
	for(i=0; i<SLIDER_AVG; ++i) { sum += slider->avg[i]; }
	return (uint16_t)(sum/SLIDER_AVG);
}