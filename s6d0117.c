/********************************************************************************/
/*!
	@file			s6d0117.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive H016IN01 V3 TFT module(8/16bit mode).

    @section HISTORY
		2012.08.31	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s6d0117.h"
/* check header file version for fool proof */
#if S6D0117_H != 0x0300
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
inline void S6D0117_reset(void)
{
	S6D0117_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S6D0117_RD_SET();
	S6D0117_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	S6D0117_RES_CLR();							/* RES=L, CS=L   			*/
	S6D0117_CS_CLR();
	_delay_ms(60);								/* wait 60ms     			*/
	
	S6D0117_RES_SET();						  	/* RES=H					*/
	_delay_ms(10);				    			/* wait 10ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D0117_wr_cmd(uint8_t cmd)
{
	S6D0117_DC_CLR();							/* DC=L						*/

	S6D0117_CMD = cmd;							/* cmd(8bit)				*/
	S6D0117_WR();								/* WR=L->H					*/

	S6D0117_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void S6D0117_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6D0117_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	S6D0117_WR();								/* WR=L->H					*/
	S6D0117_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	S6D0117_DATA = dat;							/* 16bit data 				*/
#endif
	S6D0117_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6D0117_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		S6D0117_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		S6D0117_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void S6D0117_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	S6D0117_wr_cmd(0x44);				/* Horizontal Start,End ADDR */
	S6D0117_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	S6D0117_wr_cmd(0x45);				/* Vertical Start,End ADDR */
	S6D0117_wr_dat(((OFS_RAW + height)<<8)|(OFS_RAW + y));

	S6D0117_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD15) */
	S6D0117_wr_dat(((OFS_RAW + y)<<8)|(OFS_COL + x));

	S6D0117_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S6D0117_clear(void)
{
	volatile uint32_t n;

	S6D0117_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6D0117_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t S6D0117_rd_cmd(uint8_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	S6D0117_wr_cmd(cmd);
	S6D0117_WR_SET();

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
    ReadLCDData(temp);
#endif

    ReadLCDData(val);

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	val &= 0x00FF;
	val |= temp<<8;
#endif

	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S6D0117_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	S6D0117_reset();

	/* Check Device Code */
	devicetype = S6D0117_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0117)
	{
		/* Initialize S6D0117 */
		S6D0117_wr_cmd(0x00);
		S6D0117_wr_dat(0x0001);
		S6D0117_wr_cmd(0x12);
		S6D0117_wr_dat(0x0109);  
		S6D0117_wr_cmd(0x13);
		S6D0117_wr_dat(0x0E1D);
		S6D0117_wr_cmd(0x14);
		S6D0117_wr_dat(0x6224);
		S6D0117_wr_cmd(0x10);
		S6D0117_wr_dat(0x2004);
		_delay_ms(50);
		
		S6D0117_wr_cmd(0x13);
		S6D0117_wr_dat(0x0E5D);
		_delay_ms(50);
		
		S6D0117_wr_cmd(0x01);
		S6D0117_wr_dat(0x010F);
		S6D0117_wr_cmd(0x02);
		S6D0117_wr_dat(0x0700);
		S6D0117_wr_cmd(0x03);
		S6D0117_wr_dat(0x1030);
		S6D0117_wr_cmd(0x07);
		S6D0117_wr_dat(0x0000);
		S6D0117_wr_cmd(0x08);
		S6D0117_wr_dat(0x0202);
		S6D0117_wr_cmd(0x0B);
		S6D0117_wr_dat(0x0001);
		S6D0117_wr_cmd(0x0C);
		S6D0117_wr_dat(0x0000);
		S6D0117_wr_cmd(0x40);
		S6D0117_wr_dat(0x0000);
		S6D0117_wr_cmd(0x42);
		S6D0117_wr_dat(0x8300);  
		S6D0117_wr_cmd(0x43);
		S6D0117_wr_dat(0x0000);
		S6D0117_wr_cmd(0x44);
		S6D0117_wr_dat(0x8304);
		S6D0117_wr_cmd(0x45);
		S6D0117_wr_dat(0x7F00);
		S6D0117_wr_cmd(0x25);
		S6D0117_wr_dat(0x0002);
		S6D0117_wr_cmd(0x26);
		S6D0117_wr_dat(0x0002);
		S6D0117_wr_cmd(0x30);
		S6D0117_wr_dat(0x0000);
		S6D0117_wr_cmd(0x31);
		S6D0117_wr_dat(0x0102);
		S6D0117_wr_cmd(0x32);
		S6D0117_wr_dat(0x0001);
		S6D0117_wr_cmd(0x33);
		S6D0117_wr_dat(0x0202);  
		S6D0117_wr_cmd(0x34);
		S6D0117_wr_dat(0x0707);
		S6D0117_wr_cmd(0x35);
		S6D0117_wr_dat(0x0707);
		S6D0117_wr_cmd(0x36);
		S6D0117_wr_dat(0x0707);
		S6D0117_wr_cmd(0x37);
		S6D0117_wr_dat(0x0202);
		S6D0117_wr_cmd(0x07);
		S6D0117_wr_dat(0x0005);
		_delay_ms(50);
		
		S6D0117_wr_cmd(0x07);
		S6D0117_wr_dat(0x0015);
		_delay_ms(50);
		
		S6D0117_wr_cmd(0x10);
		S6D0117_wr_dat(0x0000);
		_delay_ms(50);
		
		S6D0117_wr_cmd(0x03);
		S6D0117_wr_dat(0x1030);
		_delay_ms(50); 
		
		S6D0117_wr_cmd(0x10);
		S6D0117_wr_dat(0x2004);
		_delay_ms(50);
		
		S6D0117_wr_cmd(0x13);
		S6D0117_wr_dat(0x0E5D);
		_delay_ms(50);
		
		S6D0117_wr_cmd(0x07);
		S6D0117_wr_dat(0x0017);
		_delay_ms(50);
	}

	else { for(;;);} /* Invalid Device Code!! */

	S6D0117_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	S6D0117_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	do {
		S6D0117_wr_dat(COL_RED);
	} while (--n);
	
		_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
