#ifndef LCD_H
#define LCD_H

uint8_t initFT800(void);				/* Init FT800 */
void touch_calibrate(void);				/* Calibrate FT800 Touch Engine */
void clrscr(void);						/* Clear FT800 screen */

void lcd_start(void);					/* Startup screen */
uint8_t lcd_start_sd(void);				/* SD card information screen */

uint8_t lcd_main(void);					/* Main menu */
void lcd_alert(SensorStruct* sensor);	/* Alert menu */
void lcd_status(void);					/* Status menu */
void lcd_notifications(void);			/* Notifications menu */
void lcd_date(void);					/* Set date&time menu */

void lcd_settings(void);						/* Settings menu */
void lcd_settings_alert(SensorStruct* Sensor);	/* Settings: set Sensor alert values submenu */
void lcd_settings_conf(SensorStruct* Sensor);	/* Settings: set Sensor configuration values submenu */
void lcd_settings_name(SensorStruct* Sensor);	/* Settings: change Sensor name submenu */

#endif