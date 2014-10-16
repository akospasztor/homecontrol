#ifndef GLOBALS_H
#define GLOBALS_H

#define TouchInit(tag)	{tag,0,0}

#define NAME_MAX		16
#define BUF_MAX			32
#define MSG_MAX			15
#define NOTIF_MAX		9
#define SLIDER_AVG		8

/* Defines for alert */
#define ALERT_TEMP_L	1
#define ALERT_TEMP_H	2
#define ALERT_HUM_L		3
#define ALERT_HUM_H		4
#define ALERT_CO		5
#define ALERT_CO2		6
#define ALERT_TIME		30

/* Defines for Polling */
#define POLL_REQ	1
#define POLL_WAIT	2
#define POLL_RDY	3

/* Defines for Plugin */
#define PLUGIN_RS485	1
#define PLUGIN_KNX		2
#define PLUGIN_OFF		0

/* Communication Defines for Sensors */
#define COM_OFF			0
#define COM_TP			1
#define COM_RF			2

/* Communication Defines for BT */
#define BT_OFF			0
#define BT_READY		20
#define BT_PAIRED		30
#define BT_CONNECTED	40

/*** ENUMERATIONS ******************************************************************/
enum modeType 		{ COOLING = 0, HEATING = 1 };
enum modeState 		{ OFF = 0, ON = 1 };

/*** STRUCTURES ********************************************************************/
typedef struct 		/* Sensor Struct */
{
	char name[NAME_MAX+1];	/* Sensor Name */
	float target;			/* Target temperature: control the sensor module to reach this temp */
	float temp;				/* Current temperature */
	uint8_t humid;			/* Current humidity */
	uint16_t co;			/* Current CO level */
	uint16_t co2;			/* Current CO2 level */
	
	float aTmax;			/* Max limit for temperature */
	float aTmin;			/* Min limit for temperature */
	uint8_t aHmax;			/* Max limit for humidity */
	uint8_t aHmin;			/* Min limit for humidity */
	uint16_t aCO;			/* Max limit for CO */
	uint16_t aCO2;			/* Max limit for CO2 */
	
	/*  Alert Variable: if something generates an alert, the proper bit is set to 1
	 *	Bit0: alert state (0: alert screen wasn't displayed yet in current round)
	 *	Bit1 - Bit6: see ALERT_ defines
	 */
	uint8_t alert;
	uint8_t alertTimer;	/* If an alert is displayed, it is set to a value (in sec, see defines) and counts to 0. When reaches 0, sysTimerIT clears Sensor:alert Bit0 */
	
	uint8_t com;		/* 0: off | 1: twisted pair | 2: radio */
	uint8_t state;		/* 0: offline | 1: online */
	uint8_t FLAG_poll;	/* see POLL_ defines */
	uint8_t errcount;	/* counts the number of failed polls. if higher than the limit, the sensor is displayed as offline */
	char addr[5];		/* address: radio */
} SensorStruct;

typedef struct		/* Mode Struct */
{
	enum modeType type;		/* COOLING or HEATING */
	enum modeState state;	/* ON or OFF */
} ModeStruct;

typedef struct		/* Peripherial Connection Struct */
{
	uint8_t ethernet;	/* 1: ethernet cable is connected | 0: not connected */	
	uint8_t usb;		/* 1: usb is connected | 0: not connected */
	uint8_t microsd;	/* 1: microsd card is inserted | 0: no microsd card */
} PeriphConnectionStruct;

typedef struct		/* Bluetooth module Struct */
{
	uint8_t state;		/* bluetooth module current state (see defines for states) */
	uint8_t cmdsent;	/* 1: cmd sent to BT and waiting for response | 0: cmd isn't sent to BT yet */
	uint8_t errcount;	/* error counter: inc when ERROR received. resets when OK received. */
	char address[13];	/* bluetooth module mac address string, last char is for termination */
	char pairaddr[13];	/* paired bluetooth module mac address */
} ModuleBTStruct;

typedef struct		/* TCP Server Struct */
{
	uint8_t state;		/* 1: server is on & listening | 0: server is off */
	uint8_t ip[4];		/* ip address */
	uint16_t port;		/* server listening port */
} ModuleTcpServerStruct;

typedef struct		/* Radio module Struct */
{
	uint8_t state;		/* 1: radio module is on | 0: radio module is not present */
	char address[5];	/* radio rx address */
} ModuleRadioStruct;

typedef struct		/* KNX module Struct */
{
	uint8_t plugin;		/* 1: RS-485 | 2: KNX | 0: No plugin */
	uint8_t state;		/* 1: module is on | 0: module is off */
} ModuleKNXStruct;

/*** LCD DISPLAY STRUCTURES ********************************************************/
typedef struct		/* Button Struct */
{
	uint32_t tag;			/* Touch tag */
	uint8_t  prevstate;		/* Previous state */
	uint8_t  pressed;		/* Is pressed flag */
} ButtonStruct;

typedef struct		/* Slider Struct */
{
	uint16_t value;				/* current value */
	uint16_t avg[SLIDER_AVG];	/* value array for calculating average */
	uint8_t	 index;				/* index pointer for position in avg */
} SliderStruct;


/*** FUNCTIONS *********************************************************************/
void processTasks(void);				/* main fuction for processing tasks */
void fillMsgPoll(uint8_t addr);			/* fill the message buffer with data for polling */
void fillSensorData(const char* data);	/* fill the Sensor struct with received data AND check Sensor data for alerts */
uint8_t isValidSensorData(const char* rx, const uint8_t length);	/* check incoming data. return 1: data is valid & usable, 0: corrupted data */

/* Sensors */
void initSensor(SensorStruct* sensor, char* name, float target);							/* Init Sensor Struct with default values */
void nameSensor(SensorStruct* sensor, char* name);											/* Set Sensor name */
void setSensor(SensorStruct* sensor, float temp, uint8_t humid, uint16_t co, uint16_t co2);	/* Set Sensor values */
void setSensorAlert(SensorStruct* sensor, float target, float aTmax, float aTmin, uint8_t aHmax, uint8_t aHmin, uint16_t aCO, uint16_t aCO2);	/* Set Sensor alert values */
void setSensorCom(SensorStruct* sensor, uint8_t com, char addr_lsb);						/* Set Sensor communication type */

/* Sensor Alert Functions */
void checkSensorForAlert(SensorStruct* sensor);			/* check sensor data for alert (it compares values with the previously set limits) */
void setAlert(SensorStruct* sensor, uint8_t alert);		/* set the proper bit in Sensor:alert variable */
uint8_t checkAlert(SensorStruct* sensor, uint8_t alert);/* check the proper bit in Sensor:alert variable */
void clrAlert(SensorStruct* sensor, uint8_t alert);		/* clear the proper bit in Sensor:alert variable */
void clrAlertAll(SensorStruct* sensor);					/* clear all alert bit in Sensor:alert variable */
uint8_t isAlert(SensorStruct* sensor);					/* return: 0: no need to display alert screen (either because it was already done or there is no alert | 1: need to display alert */
void setAlertStateBit(SensorStruct* sensor);			/* set Bit0 in Sensor:alert to 1 */
void clrAlertStateBit(SensorStruct* sensor);			/* clear Bit0 in Sensor:alert */

/* Notification Messages */
void addNotif(char* msg);				/* Add new notification to linked list */
void clearNotif(void);					/* Clear notifications linked list */

/* Dedicated KNX RX buffer */
void bufKNXClr(void);					/* clear buffer with '\0' */
uint8_t bufKNXPush(const char c);		/* push back char into buffer. return 1 in case of buffer overflow */
uint8_t isReadyKNXResponse(void);		/* check if response from KNX/RS-485 is completely received */

/* Dedicated BT RX buffer */
void bufBTClr(void);					/* clear rx buffer with '\0' */
void rBTClr(void);						/* clear response rBT buffer with '\0' */
char bufBTPop(void);					/* pop char from back. if no more char to pop, it does nothing */
uint8_t bufBTPush(const char c);		/* push back char into buffer. return 1 in case of buffer overflow */
uint8_t bufBTCRLF(void);				/* check if the last two char was CR LF <enter> and there is more character(s) in buffer before <enter> */

/* Connections */
void PeriphConnectionStructInit(PeriphConnectionStruct* PeriphConnection);	/* init PeriphConnection Struct with default data */
void getPeriphConnection(PeriphConnectionStruct* PeriphConnection);			/* get the connection status of the peripherials (ethernet, usb, microsd) */

/* Bluetooth */
void ModuleBTStructInit(ModuleBTStruct* ModuleBT);		/* init ModuleBT Struct with default data */
void ModuleBTSend(unsigned char* cmd);					/* clear rBT buffer, send command, set ModuleBT.cmdsent to 1 */
uint8_t ModuleBTResponse(void);							/* check if there is valid response from BT. Returns 0 when there is no response.
														 * Returns 0xFF when OK and stores useful response in rBT response buffer. Clear ModuleBT.errcount
														 * Returns error code on ERROR. Increases ModuleBT.errcount */
uint8_t ModuleBTPair(void);								/* in READY mode returns 0xFF when pairing with another device was successful.
														 * Returns error number on pairing error.
														 * Returns 0 when nothing happened */

/* TCP Server */
void ModuleTcpServerStructInit(ModuleTcpServerStruct* ModuleTcpServer);		/* init ModuleTcpServer Struct with default data */

/* Radio */
void ModuleRadioStructInit(ModuleRadioStruct* ModuleRadio);					/* init ModuleRadio Struct with default data */

/* KNX | RS-485 */
void ModuleKNXStructInit(ModuleKNXStruct* ModuleKNX);						/* init ModuleKNX Struct with default data */
void getPluginType(ModuleKNXStruct* ModuleKNX);								/* determines which plugin is installed */

/* uSD */
void writeConfigToSD(void);				/* Write current configuration to SD */
void removeCRLF(char* str);				/* Remove CRLF from string (used when reading from SD */

/* Touch */
uint8_t tagIsValid(uint32_t tag);						/* check if tag is valid (all 4 bytes are equal) */
uint8_t buttonIsClicked(const ButtonStruct* btn, const uint32_t tag);	/* check if button was pressed (when released (tag==0), previous tag value was the tag value of the button) */

void sliderInit(SliderStruct* slider, uint16_t value);	/* init slider circular buffer with default value */
void sliderPush(SliderStruct* slider, uint16_t value);	/* insert new value into slider circular buffer */
uint16_t sliderAvg(SliderStruct* slider);				/* calculate slider circular buffer average value */

#endif