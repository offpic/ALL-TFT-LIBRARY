/********************************************************************************/
/*!
	@file			st7787.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -S95417				(ST7787)	8/16bit mode.			@n
					 -AR240320A7NFWUG2		(ST7785)	8/16bit mode.			

    @section HISTORY
		2012.08.27	V1.00	Stable Release.
		2013.04.01  V2.00   Added ST7785 Support.
		2014.10.15	V3.00	Fixed 8-bit access bug.
		2023.05.01	V4.00	Removed unused delay function.
		2023.08.01	V5.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "st7787.h"
/* check header file version for fool proof */
#if ST7787_H != 0x0500
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Display Module Reset Routine.
*/
/**************************************************************************/
inline void ST7787_reset(void)
{
	ST7787_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ST7787_RD_SET();
	ST7787_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	ST7787_RES_CLR();							/* RES=L, CS=L   			*/
	ST7787_CS_CLR();

	_delay_ms(10);								/* wait 10ms     			*/
	ST7787_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait over 120ms     		*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ST7787_wr_cmd(uint8_t cmd)
{
	ST7787_DC_CLR();							/* DC=L						*/

	ST7787_CMD = cmd;							/* cmd(8bit)				*/
	ST7787_WR();								/* WR=L->H					*/

	ST7787_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ST7787_wr_dat(uint8_t dat)
{
	ST7787_DATA = dat;							/* data(8bit_Low or 16bit)	*/
	ST7787_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ST7787_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ST7787_DATA = (uint8_t)(gram>>8);			/* upper 8bit data		*/
	ST7787_WR();								/* WR=L->H				*/
	ST7787_DATA = (uint8_t)gram;				/* lower 8bit data		*/
#else
	ST7787_DATA = gram;							/* 16bit data			*/
#endif
	ST7787_WR();								/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ST7787_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ST7787_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		ST7787_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ST7787_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	ST7787_wr_cmd(cmd);
	ST7787_WR_SET();

    ReadLCDData(temp);							/* Dummy Read(Invalid Data) */
    ReadLCDData(temp);							/* Dummy Read				*/
    ReadLCDData(temp);							/* Upper Read				*/
    ReadLCDData(val);							/* Lower Read				*/

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ST7787_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ST7787_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	ST7787_wr_dat((OFS_COL + x)>>8);
	ST7787_wr_dat(OFS_COL + x);
	ST7787_wr_dat((OFS_COL + width)>>8);
	ST7787_wr_dat(OFS_COL + width);

	ST7787_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	ST7787_wr_dat((OFS_RAW + y)>>8);
	ST7787_wr_dat(OFS_RAW + y);
	ST7787_wr_dat((OFS_RAW + height)>>8);
	ST7787_wr_dat(OFS_RAW + height);

	ST7787_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ST7787_clear(void)
{
	volatile uint32_t n;

	ST7787_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ST7787_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ST7787_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	ST7787_reset();

	/* Check Device Code */
	/* ST7787/7785 DeviceID all 0xFF! */
	/* But we can read "Display Read Status(09h)" as "0x6100" */
	devicetype = ST7787_rd_cmd(0x09);  		/* Confirm Vaild LCD Controller */

	if(devicetype == 0x6100)
	{
		/* Initialize ST7787 */
		ST7787_wr_cmd(0x11);
		_delay_ms(125);   
		
		ST7787_wr_cmd(0xB1);
		ST7787_wr_dat(0x3C);
		ST7787_wr_dat(0x02);
		ST7787_wr_dat(0x02);
		
		ST7787_wr_cmd(0xBC);
		
		ST7787_wr_cmd(0xC2);
		ST7787_wr_dat(0x04);
		ST7787_wr_dat(0xE6);
		ST7787_wr_dat(0x86);
		ST7787_wr_dat(0x33);
		ST7787_wr_dat(0x03);
		
		ST7787_wr_cmd(0xC3);
		ST7787_wr_dat(0x03);
		ST7787_wr_dat(0x33);
		ST7787_wr_dat(0x03);
		ST7787_wr_dat(0x00);
		ST7787_wr_dat(0x00);
		
		ST7787_wr_cmd(0xF4);
		ST7787_wr_dat(0xFF);
		ST7787_wr_dat(0x3F);
		
		ST7787_wr_cmd(0xF5);
		ST7787_wr_dat(0x10);
		
		ST7787_wr_cmd(0xFB);
		ST7787_wr_dat(0x7F);
		
		ST7787_wr_cmd(0xC5);
		ST7787_wr_dat(0xC9);
		ST7787_wr_dat(0x1A);
		
		ST7787_wr_cmd(0xC6);
		ST7787_wr_dat(0x24);
		ST7787_wr_dat(0x00);
		
		ST7787_wr_cmd(0xC0);
		ST7787_wr_dat(0x00);
		
		ST7787_wr_cmd(0xB6);
		ST7787_wr_dat(0x02);
		ST7787_wr_dat(0x04);
		
		ST7787_wr_cmd(0x36);
		ST7787_wr_dat((0<<7)|(0<<6)|(0<<5)|(0<<4)|(0<<3)|(0<<2));   
		
		ST7787_wr_cmd(0x3A);   
		ST7787_wr_dat(0x05); 
		
		ST7787_wr_cmd(0x29);
		_delay_ms(50);
	}

	else { for(;;);} /* Invalid Device Code!! */

	ST7787_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ST7787_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ST7787_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
