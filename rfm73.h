#ifndef RFM73_H
#define RFM73_H

#define RFM73_CMD_W_REGISTER			0x20
#define RFM73_CMD_R_RX_PAYLOAD			0x61
#define RFM73_CMD_W_TX_PAYLOAD			0xA0
#define RFM73_CMD_FLUSH_TX              0xE1
#define RFM73_CMD_FLUSH_RX              0xE2
#define RFM73_CMD_REUSE_TX_PL           0xE3
#define RFM73_CMD_W_TX_PAYLOAD_NOACK    0xB0
#define RFM73_CMD_W_ACK_PAYLOAD         0xA8
#define RFM73_CMD_ACTIVATE              0x50
#define RFM73_CMD_R_RX_PL_WID           0x60
#define RFM73_CMD_NOP                   0xFF

#define RFM73_ADDR_LENGTH				5
#define RFM73_REG_STATUS				0x07
#define RFM73_REG_RX_ADDR_P0			0x0A
#define RFM73_REG_RX_ADDR_P1            0x0B
#define RFM73_REG_RX_ADDR_P2            0x0C
#define RFM73_REG_RX_ADDR_P3            0x0D
#define RFM73_REG_RX_ADDR_P4            0x0E
#define RFM73_REG_RX_ADDR_P5            0x0F
#define RFM73_REG_TX_ADDR               0x10
#define RFM73_REG_DYNPD                 0x1C

/*** Low-level functions ***********************************************************/
void rfm_CE(uint8_t val);		/* set Chip Enable (activates RX or TX mode) */
void rfm_TREN(uint8_t val);		/* set TREN (1: TX, 0: RX) */
void rfm_PAEN(uint8_t val);		/* set PAEN (1: chip is active, 0: chip is shutdown) */


/*** Application functions *********************************************************/
uint8_t rfm_init(void);				/* returns 1 on successful init */
void rfm_bank_select(uint8_t bank);	/* select register bank */
void rfm_mode_rx(void);				/* set receiver mode */
void rfm_mode_tx(void);				/* set transmitter mode */

uint8_t rfm_check_rx(void);			/* check if new data in rx (used when rfm73 module is configured without rx interrupt) */
void rfm_set_addr(char addr[]);		/* function for setting RX_P0 and TX addresses */
void rfm_send(const char data[], const uint8_t length);		/* function for sending data. when MAX_RT is asserted (maximum retransmits) then it auto-clears the register to enable communication */ 
void rfm_rec(char data[], uint8_t* length);					/* function for receiving data */

uint8_t rfm_txfifo_empty(void);		/* check if tx fifo is empty */
uint8_t rfm_rxfifo_empty(void); 	/* check if rx fifo is empty */

uint8_t rfm_buffer_write(const char addr, const char data[], const uint8_t length);	/* write custom length(max. 32byte) data to address. LSB byte to MSB byte! Can be used for writing into fifo/buffers and also for writing custom length data into register bank (eg. setting tx/rx address) */
uint8_t rfm_buffer_read(const char addr, char data[], uint8_t length);				/* read custom length data from address */
uint8_t rfm_rxfifo_read(const char addr, char data[], uint8_t* length);				/* read custom length(max. 32byte) data from rx fifo. */

void rfm_cmd_write(char cmd, char data);					/* write 1byte command */
void rfm_reg_write(char address, char data);				/* write 1byte data into register */
char rfm_reg_read(char address);							/* read 1byte data from register */

void rfm_bank1_write_0to8(uint8_t address, uint32_t val);	/* write 4byte data into bank1 (address 0-to-8:  MSB byte first) */
void rfm_bank1_write_9to13(uint8_t address, uint32_t val);	/* write 4byte data into bank1 (address 9-to-13: LSB byte first) */
void rfm_bank1_write_14(void);								/* write special 11byte data into bank1 address 14 (LSB byte first) */

#endif