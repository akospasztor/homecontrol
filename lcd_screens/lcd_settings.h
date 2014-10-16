#ifndef LCD_SETTINGS_H
#define LCD_SETTINGS_H

void store_settings_default(void);
void draw_settings_screen(const ButtonStruct* btnHome, const ButtonStruct* btnEnableAlerts,
						  const ButtonStruct* btnS1Alert, const ButtonStruct* btnS1Conf, const ButtonStruct* btnS1Name,
						  const ButtonStruct* btnS2Alert, const ButtonStruct* btnS2Conf, const ButtonStruct* btnS2Name,
						  const ButtonStruct* btnS3Alert, const ButtonStruct* btnS3Conf, const ButtonStruct* btnS3Name);

void store_settings_alert_default(void);
void draw_settings_alert_screen(SensorStruct* Sensor, const ButtonStruct* btnBack, const SliderStruct* Slider,
								const ButtonStruct* btnTempMaxM,	const ButtonStruct* btnTempMaxP,
								const ButtonStruct* btnHumMaxM, 	const ButtonStruct* btnHumMaxP,
								const ButtonStruct* btnCOMaxM, 		const ButtonStruct* btnCOMaxP,
								const ButtonStruct* btnTempMinM, 	const ButtonStruct* btnTempMinP,
								const ButtonStruct* btnHumMinM, 	const ButtonStruct* btnHumMinP,
								const ButtonStruct* btnCO2MaxM, 	const ButtonStruct* btnCO2MaxP);

void store_settings_conf_default(void);
void draw_settings_conf_screen(SensorStruct* Sensor, const ButtonStruct* btnBack);

void store_settings_name_default(void);
void draw_settings_name_screen(SensorStruct* Sensor, const ButtonStruct* btnBack, const ButtonStruct* keyboard, uint8_t shift, char* name);

#endif