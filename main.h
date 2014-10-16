#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx.h"

#define LEDport		GPIOC
#define LEDS 		GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11
#define LEDhb		GPIO_Pin_8
#define LEDg1		GPIO_Pin_7
#define LEDg2		GPIO_Pin_6
#define LEDb		GPIO_Pin_10
#define LEDy		GPIO_Pin_11


/*** FUNCTIONS *********************************************************************/
int main(void);

void initSystem(void);			/* Init System */
void initSysTimer(void);		/* Init System Timer IT */
void initRTC(uint8_t reset, RTC_DateTypeDef* date, RTC_TimeTypeDef* time);	/* Init RTC */
void initLeds(void);			/* Init LEDs */
void initPWM(void);				/* Init Buzzer PWM */
void setPWM(uint8_t level);		/* Set Buzzer frequency (high or low) */
void initUSB(void);				/* Init USB */

void ledOn(uint16_t LED);		/* Turn On LED */
void ledOff(uint16_t LED);		/* Turn Off LED */ 

void delay(u32);				/* Breaking Delay (ms) (only for debug purposes!) */
void sysDus(uint32_t us);		/* SysTick Delay (us) */
void sysDms(uint32_t ms);		/* SysTick Delay (ms) */

#endif