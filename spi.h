#ifndef SPI_H
#define SPI_H

/* FT800 GPIOs */
#define LCD_PORT        GPIOA
#define LCD_PORT_PD     GPIOE

#define LCD_PDN         GPIO_Pin_8
#define LCD_CS          GPIO_Pin_4

/* FT800 functions */
void SPI_init(void);			/* SPI1 init for FT800 */
void SPI_speedup(void);			/* Speed Up SPI1 */
char SPI_send(char data);		/* Send char to SPI1 */
char SPI_rec(char address);		/* Receive char from SPI1 */

void FT_spi_select(void);		/* Select FT800 */
void FT_spi_deselect(void);		/* Deselect FT800 */

/* RFM73P */
void RF_spi_init(unsigned int prescaler);		/* SPI2 init & RFM73P GPIOs	 	*/
void RF_spi_select(void);						/* SPI2 RFM73P chip select 		*/
void RF_spi_deselect(void);						/* SPI2 RFM73P chip deselect 	*/
char RF_spi_send(char data);					/* SPI2 char send & receive (no chip select management!) */


/* microSD */
void SD_init(unsigned int prescaler);					/* Init SD card: SPI2 & GPIOs */
unsigned char SD_send_single(unsigned char data);		/* Send single byte to SPI2 */
unsigned char SD_receive_single(void);					/* Receive single byte from SPI2 */
void SD_send(unsigned char* data, unsigned int length);		/* Send length bytes of data to SPI2 */
void SD_receive(unsigned char* data, unsigned int length);	/* Receive length bytes of data from SPI2 */
void SD_transmit(unsigned char* txbuf, unsigned char* rxbuf, unsigned int len);	/* Transmit len bytes from tx buffer to SPI2 and receive response into rx buffer */

#endif