#ifndef LCD_DATE_H
#define LCD_DATE_H

void store_date_defaults(void);
void draw_date_screen(const ButtonStruct* btnBack, const ButtonStruct* keys, RTC_DateTypeDef* date, RTC_TimeTypeDef* time);

#endif