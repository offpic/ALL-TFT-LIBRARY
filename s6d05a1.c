/********************************************************************************/
/*!
	@file			s6d05a1.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TFT1P3925-E				(S6D05A1)	16bit mode.

    @section HISTORY
		2013.12.30	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s6d05a1.h"
/* check header file version for fool proof */
#if S6D05A1_H != 0x0300
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
inline void S6D05A1_reset(void)
{
	S6D05A1_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S6D05A1_RD_SET();
	S6D05A1_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	S6D05A1_RES_CLR();							/* RES=L, CS=L   			*/
	S6D05A1_CS_CLR();

	_delay_ms(20);								/* wait 10ms     			*/
	S6D05A1_RES_SET();						  	/* RES=H					*/
	_delay_ms(100);				    			/* wait 100ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D05A1_wr_cmd(uint8_t cmd)
{
	S6D05A1_DC_CLR();							/* DC=L						*/

	S6D05A1_CMD = cmd;							/* cmd(8bit)				*/
	S6D05A1_WR();								/* WR=L->H					*/

	S6D05A1_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void S6D05A1_wr_dat(uint8_t dat)
{
	S6D05A1_DATA = dat;							/* data						*/
	S6D05A1_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void S6D05A1_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6D05A1_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	S6D05A1_WR();								/* WR=L->H					*/
	S6D05A1_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	S6D05A1_DATA = gram;						/* 16bit data 				*/
#endif
	S6D05A1_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6D05A1_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		S6D05A1_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		S6D05A1_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}



/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t S6D05A1_rd_cmd(uint8_t cmd)
{
	uint16_t val,temp;

	S6D05A1_wr_cmd(cmd);
	S6D05A1_WR_SET();

	ReadLCDData(temp);
    ReadLCDData(val);
	ReadLCDData(temp);
	ReadLCDData(temp);
	ReadLCDData(temp);
	val &= 0x00FF;

	return val;
}


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void S6D05A1_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	S6D05A1_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	S6D05A1_wr_dat((OFS_COL + x)>>8);
	S6D05A1_wr_dat(OFS_COL + x);
	S6D05A1_wr_dat((OFS_COL + width)>>8);
	S6D05A1_wr_dat(OFS_COL + width);

	S6D05A1_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	S6D05A1_wr_dat((OFS_RAW + y)>>8);
	S6D05A1_wr_dat(OFS_RAW + y);
	S6D05A1_wr_dat((OFS_RAW + height)>>8);
	S6D05A1_wr_dat(OFS_RAW + height);

	S6D05A1_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S6D05A1_clear(void)
{
	volatile uint32_t n;

	S6D05A1_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6D05A1_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S6D05A1_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	S6D05A1_reset();
	
	/* Check Device Code */
	devicetype = S6D05A1_rd_cmd(0xA1);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0xF0)
	{
		/* Initialize S6D05A1 */
		S6D05A1_wr_cmd(0xF0);		/* PASSWD1 */
		S6D05A1_wr_dat(0x5A);
		S6D05A1_wr_dat(0x5A);
		S6D05A1_wr_cmd(0xF1);		/* PASSWD2 */
		S6D05A1_wr_dat(0x5A);
		S6D05A1_wr_dat(0x5A);
		
		S6D05A1_wr_cmd(0xF2);		/* DISCTL */
		S6D05A1_wr_dat(0x3B);
		S6D05A1_wr_dat(0x48);
		S6D05A1_wr_dat(0x03);
		S6D05A1_wr_dat(0x08);
		S6D05A1_wr_dat(0x08);
		S6D05A1_wr_dat(0x08);
		S6D05A1_wr_dat(0x08);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x08);
		S6D05A1_wr_dat(0x08);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x54);
		S6D05A1_wr_dat(0x08);
		S6D05A1_wr_dat(0x08);
		S6D05A1_wr_dat(0x08);
		S6D05A1_wr_dat(0x08);
		
		S6D05A1_wr_cmd(0xF4);		/* PWRCTL */
		S6D05A1_wr_dat(0x09);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x3F);
		S6D05A1_wr_dat(0x79);
		S6D05A1_wr_dat(0x03);
		S6D05A1_wr_dat(0x3F);
		S6D05A1_wr_dat(0x79);
		S6D05A1_wr_dat(0x03);
		
		S6D05A1_wr_cmd(0xF5);		/* VCMCTL */		
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x5D);
		S6D05A1_wr_dat(0x75);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);		
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x04);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x5D);
		S6D05A1_wr_dat(0x75);
		
		S6D05A1_wr_cmd(0xF6);		/* SRCCTL */		
		S6D05A1_wr_dat(0x04);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x08);
		S6D05A1_wr_dat(0x03);
		S6D05A1_wr_dat(0x01);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x01);
		S6D05A1_wr_dat(0x00);
		
		S6D05A1_wr_cmd(0xF7);		/* IFCTL */
		S6D05A1_wr_dat(0x48);
		S6D05A1_wr_dat(0x80);
		S6D05A1_wr_dat(0x10);
		S6D05A1_wr_dat(0x02);
		S6D05A1_wr_dat(0x00);
		
		S6D05A1_wr_cmd(0xF8);		/* PANELCTL */
		S6D05A1_wr_dat(0x11);
		S6D05A1_wr_dat(0x00);
		
		S6D05A1_wr_cmd(0xF9);		/* GAMMASEL */
		S6D05A1_wr_dat(0x24);		/* red */
		S6D05A1_wr_cmd(0xFA);		/* PGAMMACTL */
		S6D05A1_wr_dat(0x0B);
		S6D05A1_wr_dat(0x0B);
		S6D05A1_wr_dat(0x05);
		S6D05A1_wr_dat(0x01);
		S6D05A1_wr_dat(0x0B);
		S6D05A1_wr_dat(0x20);
		S6D05A1_wr_dat(0x2C);
		S6D05A1_wr_dat(0x13);
		S6D05A1_wr_dat(0x21);
		S6D05A1_wr_dat(0x24);
		S6D05A1_wr_dat(0x30);
		S6D05A1_wr_dat(0x32);
		S6D05A1_wr_dat(0x24);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x01);
		
		S6D05A1_wr_cmd(0xF9);		/* GAMMASEL */
		S6D05A1_wr_dat(0x22);		/* green */	
		S6D05A1_wr_cmd(0xFA);		/* PGAMMACTL */
		S6D05A1_wr_dat(0x0B);
		S6D05A1_wr_dat(0x0B);
		S6D05A1_wr_dat(0x10);
		S6D05A1_wr_dat(0x31);
		S6D05A1_wr_dat(0x32);
		S6D05A1_wr_dat(0x35);
		S6D05A1_wr_dat(0x36);
		S6D05A1_wr_dat(0x11);
		S6D05A1_wr_dat(0x1D);
		S6D05A1_wr_dat(0x23);
		S6D05A1_wr_dat(0x2F);
		S6D05A1_wr_dat(0x2F);
		S6D05A1_wr_dat(0x24);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x01);
		
		S6D05A1_wr_cmd(0xF9);		/* GAMMASEL */
		S6D05A1_wr_dat(0x21);		/* blue */
		S6D05A1_wr_cmd(0xFA);		/* PGAMMACTL */
		S6D05A1_wr_dat(0x0B);
		S6D05A1_wr_dat(0x0B);
		S6D05A1_wr_dat(0x1A);
		S6D05A1_wr_dat(0x3A);
		S6D05A1_wr_dat(0x3F);
		S6D05A1_wr_dat(0x3F);
		S6D05A1_wr_dat(0x3F);
		S6D05A1_wr_dat(0x07);
		S6D05A1_wr_dat(0x18);
		S6D05A1_wr_dat(0x1F);
		S6D05A1_wr_dat(0x28);
		S6D05A1_wr_dat(0x1E);
		S6D05A1_wr_dat(0x1A);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x00);
		S6D05A1_wr_dat(0x01);
		
		S6D05A1_wr_cmd(0x3A);		/* COLMOD */
		S6D05A1_wr_dat(0x55);		/* Base Customer selection      
										77H=24bits/pixel
										66H=18bits/pixel                           
										55H=16bits/pixel */
		
		S6D05A1_wr_cmd(0x36);		/* MADCTL */
		S6D05A1_wr_dat(0x00);
		
		S6D05A1_wr_cmd(0x35);		/* TEON */
		S6D05A1_wr_dat(0x00);
		
		S6D05A1_wr_cmd(0x11);		/* SLPOUT */
		_delay_ms(120);
		
		S6D05A1_wr_cmd(0x29);		/* DISPON */
	}

	else { for(;;);} /* Invalid Device Code!! */

	S6D05A1_clear();

#if 0 	/* test code RED */
	volatile uint32_t n;

	S6D05A1_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6D05A1_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
