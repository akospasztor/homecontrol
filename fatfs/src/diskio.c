/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "main.h"

unsigned char sdcCommand[6];

void sdc_assert(void)
{
	GPIOD->ODR &= ~GPIO_Pin_15;
}

void sdc_deassert(void)
{
	GPIOD->ODR |= GPIO_Pin_15;
	SD_send_single(0xFF); // send 8 clocks for SDC to set SDO tristate
}

uint8_t sdc_isConn(void)
{
	return 1;
	
//	if (GPIOC->IDR & GPIO_Pin_5)
//	{
//		return 0;
//	}
//	else
//	{
//		return 1;
//	}
}

void sdc_sendCommand(uint8_t command, uint8_t par1, uint8_t par2, uint8_t par3, uint8_t par4)
{
	sdcCommand[0] = 0x40 | command;
	sdcCommand[1] = par1;
	sdcCommand[2] = par2;
	sdcCommand[3] = par3;
	sdcCommand[4] = par4;
	
	if (command == SDC_GO_IDLE_STATE)
	{
		sdcCommand[5] = 0x95; // precalculated CRC
	}
	else 
	{
		sdcCommand[5] = 0xFF;
	}
	SD_send(sdcCommand, 6);
}

uint8_t sdc_getResponse(uint8_t response)
{
	for(uint8_t n = 0; n < 8; n++)
	{
		unsigned char tmp = SD_receive_single();
		if (tmp == response)
		{
			return 0;
		}
	}
	return 1;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber (0..) */
)
{		
	GPIO_InitTypeDef GPIO_InitStruct;
	
	// initialize chip select line (PD15)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	GPIOD->ODR |= GPIO_Pin_15;

	// initialize card detect line (PC5)
	/*GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);*/

	//init SPI2 for uSD low speed
	SD_init(SPI_BaudRatePrescaler_128);

	// check wether SDC is inserted into socket
	if (!sdc_isConn())
	{
		return STA_NODISK;
	}
		
	// send 10 dummy bytes to wake up SDC
	sdc_assert();
	for (uint8_t i = 0; i < 10; i++)
	{
		SD_send_single(0xFF);
	}
	sdc_deassert();
	
	// assert SDC
	sdc_assert();

	// software reset the SDC
	sdc_sendCommand(SDC_GO_IDLE_STATE, 0, 0, 0, 0);

	if(sdc_getResponse(0x01))
	{
		return STA_NOINIT;
	}
	
	// wait for SDC to come out of idle state
	uint8_t resp = 1;
	while(resp)
	{
		sdc_sendCommand(SDC_SEND_OP_COND, 0, 0, 0, 0);
		if (!sdc_getResponse(0x00))
		{
			resp = 0;
		}		
	}
	
	// deassert the SDC
	sdc_deassert();

	//init SPI2 for uSD full speed
	SD_init(SPI_BaudRatePrescaler_8);

	return 0;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number (0..) */
)
{
	if (!sdc_isConn())
	{
		return STA_NODISK;
	}
	
	/*
	unsigned char result[2];
	sdc_assert();
	sdc_sendCommand(SDC_SEND_STATUS, 0, 0, 0, 0);
	SPI_receive(SPI1, result, 2);
	sdc_deassert();
	
	if (result[0] & 0x01)
	{
		return STA_NOINIT; 
	}
	else if (result[1] & 0x01)
	{
		return STA_PROTECT;
	}
	*/
	
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	unsigned char buf[2];
	
	// check if parameters are in valid range
	if( count > 128 )
	{
		return RES_PARERR;
	}
	
	sector *= 512; 	// convert LBA to physical address
	sdc_assert(); 	// assert SDC
	
	// if multiple sectors are to be read
	if(count > 1)
	{
		// start multiple sector read
		sdc_sendCommand(SDC_READ_MULTIPLE_BLOCK, ((sector>>24)&0xFF), ((sector>>16)&0xFF), ((sector>>8)&0xFF), (sector&0xFF));
		while(sdc_getResponse(0x00)); // wait for command acknowledgement
		
		while(count)
		{
			while(sdc_getResponse(0xFE)); // wait for data token 0xFE
			SD_receive(buff, 512); // read 512 bytes
			SD_receive(buf, 2); // receive two byte CRC
			count--;
			buff += 512;
		}
		// stop multiple sector read
		sdc_sendCommand(SDC_STOP_TRANSMISSION, 0, 0, 0, 0);
		while(sdc_getResponse(0x00)); // wait for R1 response
	}
	else // if single sector is to be read
	{
		sdc_sendCommand(SDC_READ_SINGLE_BLOCK, ((sector>>24)&0xFF), ((sector>>16)&0xFF), ((sector>>8)&0xFF), (sector&0xFF));
		while(sdc_getResponse(0x00)); // wait for command acknowledgement
		while(sdc_getResponse(0xFE)); // wait for data token 0xFE
		SD_receive(buff, 512); // receive data
		SD_receive(buf, 2); // receive two byte CRC
	}
	
	while(!SD_receive_single()); // wait until card is not busy anymore
	
	sdc_deassert(); // deassert SDC 
	
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE* buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	unsigned char result;
	unsigned char buf[2];
	buf[0] = 0xFF;
	buf[1] = 0xFF;
	
	// check if parameters are in valid range
	if( count > 128 )
	{
		return RES_PARERR;
	}
	
	sector *= 512; // convert LBA to physical address
	sdc_assert(); // assert SDC
	
	// if multiple sectors are to be written
	if(count > 1)
	{
		// start multiple sector write
		sdc_sendCommand(SDC_WRITE_MULTIPLE_BLOCK, ((sector>>24)&0xFF), ((sector>>16)&0xFF), ((sector>>8)&0xFF), (sector&0xFF));
		while(sdc_getResponse(0x00)); // wait for R1 response
		SD_send_single(0xFF);  // send one byte gap
		
		while(count)
		{
			SD_send_single(0xFC); // send multi byte data token 0xFC
			SD_send((unsigned char*)buff, 512); // send 512 bytes
			SD_send(buf, 2); // send two byte CRC
			
			// check if card has accepted data
			result = SD_receive_single();
			if( (result & 0x1F) != 0x05)
			{
				return RES_ERROR;
			}
			count--;
			buff += 512;
			while(!SD_receive_single()); // wait until SD card is ready
		}
		
		SD_send_single(0xFD); // send stop transmission data token 0xFD		
		SD_send_single(0xFF);  // send one byte gap
	}
	else // if single sector is to be written
	{
		sdc_sendCommand(SDC_WRITE_BLOCK, ((sector>>24)&0xFF), ((sector>>16)&0xFF), ((sector>>8)&0xFF), (sector&0xFF));
		while(sdc_getResponse(0x00)); // wait for R1 response
		SD_send_single(0xFF);  // send one byte gap
		SD_send_single(0xFE); // send data token 0xFE
		SD_send((unsigned char*)buff, 512); // send data
		SD_send(buf, 2); // send two byte CRC
		// check if card has accepted data
		result = SD_receive_single();
		if( (result & 0x1F) != 0x05)
		{
			return RES_ERROR;
		}
	}
	
	while(!SD_receive_single()); // wait until card is not busy anymore
	
	sdc_deassert(); // deassert SDC 
	
	return RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	return RES_OK;
}
#endif

DWORD get_fattime (void)
{
	return 0;
}
