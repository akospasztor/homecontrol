#ifndef LCD_MAIN_H
#define LCD_MAIN_H

void store_main_default(void);
void draw_main_screen(const ButtonStruct* btnStartStop, const ButtonStruct* btnStatus, const ButtonStruct* btnNotifications, const ButtonStruct* btnSettings, const ButtonStruct* btnSetDate);
void draw_main_boxes(const ButtonStruct* btnSetDate);
void draw_main_buttons(const ButtonStruct* btnStartStop, const ButtonStruct* btnStatus, const ButtonStruct* btnNotifications, const ButtonStruct* btnSettings);

#endif