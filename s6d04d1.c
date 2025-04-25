/********************************************************************************/
/*!
	@file			s6d04d1.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TFT1P3520-E				(S6D04D1)	16bit mode.

    @section HISTORY
		2014.08.01	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s6d04d1.h"
/* check header file version for fool proof */
#if S6D04D1_H != 0x0300
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
inline void S6D04D1_reset(void)
{
	S6D04D1_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S6D04D1_RD_SET();
	S6D04D1_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	S6D04D1_RES_CLR();							/* RES=L, CS=L   			*/
	S6D04D1_CS_CLR();

	_delay_ms(20);								/* wait 10ms     			*/
	S6D04D1_RES_SET();						  	/* RES=H					*/
	_delay_ms(100);				    			/* wait 100ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D04D1_wr_cmd(uint8_t cmd)
{
	S6D04D1_DC_CLR();							/* DC=L						*/

	S6D04D1_CMD = cmd;							/* cmd(8bit)				*/
	S6D04D1_WR();								/* WR=L->H					*/

	S6D04D1_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void S6D04D1_wr_dat(uint8_t dat)
{
	S6D04D1_DATA = dat;							/* data						*/
	S6D04D1_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void S6D04D1_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6D04D1_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	S6D04D1_WR();								/* WR=L->H					*/
	S6D04D1_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	S6D04D1_DATA = gram;						/* 16bit data 				*/
#endif
	S6D04D1_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6D04D1_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		S6D04D1_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		S6D04D1_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t S6D04D1_rd_cmd(uint8_t cmd)
{
	uint16_t val,temp;

	S6D04D1_wr_cmd(cmd);
	S6D04D1_WR_SET();

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
inline void S6D04D1_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	S6D04D1_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	S6D04D1_wr_dat((OFS_COL + x)>>8);
	S6D04D1_wr_dat(OFS_COL + x);
	S6D04D1_wr_dat((OFS_COL + width)>>8);
	S6D04D1_wr_dat(OFS_COL + width);

	S6D04D1_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	S6D04D1_wr_dat((OFS_RAW + y)>>8);
	S6D04D1_wr_dat(OFS_RAW + y);
	S6D04D1_wr_dat((OFS_RAW + height)>>8);
	S6D04D1_wr_dat(OFS_RAW + height);

	S6D04D1_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S6D04D1_clear(void)
{
	volatile uint32_t n;

	S6D04D1_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6D04D1_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S6D04D1_init(void)
{
	Display_IoInit_If();

	S6D04D1_reset();
	
	/* Initialize S6D04D1 */
	S6D04D1_wr_cmd(0xF4);
	S6D04D1_wr_dat(0x59);
	S6D04D1_wr_dat(0x59);
	S6D04D1_wr_dat(0x52);
	S6D04D1_wr_dat(0x52);
	S6D04D1_wr_dat(0x11);

	S6D04D1_wr_cmd(0xF5);
	S6D04D1_wr_dat(0x12);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x0B);
	S6D04D1_wr_dat(0xF0);
	S6D04D1_wr_dat(0x00);
	_delay_ms(10);

	S6D04D1_wr_cmd(0xF3);
	S6D04D1_wr_dat(0xFF);
	S6D04D1_wr_dat(0x2A);
	S6D04D1_wr_dat(0x2A);
	S6D04D1_wr_dat(0x0A);
	S6D04D1_wr_dat(0x22);
	S6D04D1_wr_dat(0x72);
	S6D04D1_wr_dat(0x72);
	S6D04D1_wr_dat(0x20);

	S6D04D1_wr_cmd(0x3A);
	S6D04D1_wr_dat(0x55);

	S6D04D1_wr_cmd(0xF2);
	S6D04D1_wr_dat(0x10);
	S6D04D1_wr_dat(0x10);
	S6D04D1_wr_dat(0x01);
	S6D04D1_wr_dat(0x08);
	S6D04D1_wr_dat(0x08);
	S6D04D1_wr_dat(0x08);
	S6D04D1_wr_dat(0x08);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x00);		/* D2:SM D1:GS D0:REV */
	S6D04D1_wr_dat(0x1A);
	S6D04D1_wr_dat(0x1A);

	S6D04D1_wr_cmd(0xF6);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x88);
	S6D04D1_wr_dat(0x10);

	S6D04D1_wr_cmd(0xF7);
	S6D04D1_wr_dat(0x0D);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x03);
	S6D04D1_wr_dat(0x0E);
	S6D04D1_wr_dat(0x1C);
	S6D04D1_wr_dat(0x29);
	S6D04D1_wr_dat(0x2D);
	S6D04D1_wr_dat(0x34);
	S6D04D1_wr_dat(0x0E);
	S6D04D1_wr_dat(0x12);
	S6D04D1_wr_dat(0x24);
	S6D04D1_wr_dat(0x1E);
	S6D04D1_wr_dat(0x07);
	S6D04D1_wr_dat(0x22);
	S6D04D1_wr_dat(0x22);

	S6D04D1_wr_cmd(0xF8);
	S6D04D1_wr_dat(0x0D);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x03);
	S6D04D1_wr_dat(0x0E);
	S6D04D1_wr_dat(0x1C);
	S6D04D1_wr_dat(0x29);
	S6D04D1_wr_dat(0x2D);
	S6D04D1_wr_dat(0x34);
	S6D04D1_wr_dat(0x0E);
	S6D04D1_wr_dat(0x12);
	S6D04D1_wr_dat(0x24);
	S6D04D1_wr_dat(0x1E);
	S6D04D1_wr_dat(0x07);
	S6D04D1_wr_dat(0x22);
	S6D04D1_wr_dat(0x22);

	S6D04D1_wr_cmd(0xF9);
	S6D04D1_wr_dat(0x1E);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x0A);
	S6D04D1_wr_dat(0x19);
	S6D04D1_wr_dat(0x23);
	S6D04D1_wr_dat(0x31);
	S6D04D1_wr_dat(0x37);
	S6D04D1_wr_dat(0x3F);
	S6D04D1_wr_dat(0x01);
	S6D04D1_wr_dat(0x03);
	S6D04D1_wr_dat(0x16);
	S6D04D1_wr_dat(0x19);
	S6D04D1_wr_dat(0x07);
	S6D04D1_wr_dat(0x22);
	S6D04D1_wr_dat(0x22);

	S6D04D1_wr_cmd(0xFA);
	S6D04D1_wr_dat(0x0D);
	S6D04D1_wr_dat(0x11);
	S6D04D1_wr_dat(0x0A);
	S6D04D1_wr_dat(0x19);
	S6D04D1_wr_dat(0x23);
	S6D04D1_wr_dat(0x31);
	S6D04D1_wr_dat(0x37);
	S6D04D1_wr_dat(0x3F);
	S6D04D1_wr_dat(0x01);
	S6D04D1_wr_dat(0x03);
	S6D04D1_wr_dat(0x16);
	S6D04D1_wr_dat(0x19);
	S6D04D1_wr_dat(0x07);
	S6D04D1_wr_dat(0x22);
	S6D04D1_wr_dat(0x22);

	S6D04D1_wr_cmd(0xFB);
	S6D04D1_wr_dat(0x0D);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x03);
	S6D04D1_wr_dat(0x0E);
	S6D04D1_wr_dat(0x1C);
	S6D04D1_wr_dat(0x29);
	S6D04D1_wr_dat(0x2D);
	S6D04D1_wr_dat(0x34);
	S6D04D1_wr_dat(0x0E);
	S6D04D1_wr_dat(0x12);
	S6D04D1_wr_dat(0x24);
	S6D04D1_wr_dat(0x1E);
	S6D04D1_wr_dat(0x07);
	S6D04D1_wr_dat(0x22);
	S6D04D1_wr_dat(0x22);

	S6D04D1_wr_cmd(0xFC);
	S6D04D1_wr_dat(0x0D);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x03);
	S6D04D1_wr_dat(0x0E);
	S6D04D1_wr_dat(0x1C);
	S6D04D1_wr_dat(0x29);
	S6D04D1_wr_dat(0x2D);
	S6D04D1_wr_dat(0x34);
	S6D04D1_wr_dat(0x0E);
	S6D04D1_wr_dat(0x12);
	S6D04D1_wr_dat(0x24);
	S6D04D1_wr_dat(0x1E);
	S6D04D1_wr_dat(0x07);
	S6D04D1_wr_dat(0x22);
	S6D04D1_wr_dat(0x22);

	S6D04D1_wr_cmd(0xFD);
	S6D04D1_wr_dat(0x11);
	S6D04D1_wr_dat(0x01);

	S6D04D1_wr_cmd(0x36);
	S6D04D1_wr_dat(0x48);

	S6D04D1_wr_cmd(0x35);
	S6D04D1_wr_dat(0x00);

	S6D04D1_wr_cmd(0xF1);
	S6D04D1_wr_dat(0x5A);

	S6D04D1_wr_cmd(0xFF);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x00);
	S6D04D1_wr_dat(0x40);

	S6D04D1_wr_cmd(0x11);
	_delay_ms(120);

	S6D04D1_wr_cmd(0xF1);
	S6D04D1_wr_dat(0x00);

	S6D04D1_wr_cmd(0x29);

	S6D04D1_clear();

#if 0 	/* test code RED */
	volatile uint32_t n;

	S6D04D1_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6D04D1_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
