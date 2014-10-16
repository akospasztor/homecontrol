#ifndef USART_H
#define USART_H

#include <stdint.h>

/* BTM511 */
void initBT(void);						/* init USART2 for Bluetooth (9600, 8N1) */
void BT_send_char(uint8_t c);			/* send char to USART2 */
void BT_send_str(unsigned char* str);	/* send str to USART2 */
void BT_send_cmd(unsigned char* str);	/* send str with terminating CR to Bluetooth */

/* KNX */
void initKNX(void);						/* init USART3 for KNX & RS-485 (19200, 8N1) */
void KNX_send_char(uint8_t c);			/* send char to USART3 */
void KNX_send_str(char* str);			/* send str to USART3 */

/* RS-485 */
void txEN485(void);						/* enable TX for RS-485 */
void rxEN485(void);						/* enable RX for RS-485 */

#endif