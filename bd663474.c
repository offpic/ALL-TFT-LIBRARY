/********************************************************************************/
/*!
	@file			bd663474.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive MZTX06A TFT module(spi mode only).

    @section HISTORY
		2013.08.20	V1.00	Stable Release.
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "bd663474.h"
/* check header file version for fool proof */
#if BD663474_H != 0x0400
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
inline void BD663474_reset(void)
{
#ifdef USE_BD663474_TFT
	BD663474_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	BD663474_RD_SET();
	BD663474_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	BD663474_RES_CLR();							/* RES=L, CS=L   			*/
	BD663474_CS_CLR();

#elif  USE_BD663474_SPI_TFT
	BD663474_RES_SET();							/* RES=H, CS=H				*/
	BD663474_CS_SET();
	BD663474_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 1ms     			*/

	BD663474_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(100);								/* wait 100ms     			*/
	BD663474_RES_SET();						  	/* RES=H					*/
	_delay_ms(100);				    			/* wait 100ms     			*/

}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_BD663474_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void BD663474_wr_cmd(uint16_t cmd)
{
	BD663474_DC_CLR();						/* DC=L					*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	BD663474_CMD = (uint8_t)(cmd>>8);		/* upper 8bit command	*/
	BD663474_WR();							/* WR=L->H				*/
	BD663474_CMD = (uint8_t)cmd;			/* lower 8bit command	*/
#else
	BD663474_CMD = cmd;						/* 16bit command		*/
#endif
	BD663474_WR();							/* WR=L->H				*/

	BD663474_DC_SET();						/* DC=H					*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void BD663474_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	BD663474_DATA = (uint8_t)(dat>>8);		/* upper 8bit data		*/
	BD663474_WR();							/* WR=L->H				*/
	BD663474_DATA = (uint8_t)dat;			/* lower 8bit data		*/
#else
	BD663474_DATA = dat;					/* 16bit data			*/
#endif
	BD663474_WR();							/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void BD663474_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		BD663474_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		BD663474_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t BD663474_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	BD663474_wr_cmd(cmd);
	BD663474_WR_SET();

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


#elif USE_BD663474_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void BD663474_wr_cmd(uint16_t cmd)
{	
	BD663474_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI16(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	BD663474_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void BD663474_wr_dat(uint16_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI16(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void BD663474_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		SendSPI16(((*(p+1))|(*(p)<<8)));
		p++;p++;
		SendSPI16(((*(p+1))|(*(p)<<8)));
		p++;p++;
	}
#endif

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline  uint16_t BD663474_rd_cmd(uint16_t cmd)
{
	return 0x0000;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void BD663474_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	BD663474_wr_cmd(0x0210);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	BD663474_wr_dat(OFS_COL + x);
	BD663474_wr_cmd(0x0211);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	BD663474_wr_dat(OFS_COL + width);
	BD663474_wr_cmd(0x0212);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	BD663474_wr_dat(OFS_RAW + y);
	BD663474_wr_cmd(0x0213);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	BD663474_wr_dat(OFS_RAW + height);

	BD663474_wr_cmd(0x0201);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	BD663474_wr_dat(OFS_RAW + y);
	BD663474_wr_cmd(0x0200);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	BD663474_wr_dat(OFS_COL + x);

	BD663474_wr_cmd(0x0202);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void BD663474_clear(void)
{
	volatile uint32_t n;

	BD663474_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		BD663474_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void BD663474_init(void)
{
	Display_IoInit_If();

	BD663474_reset();

	/* Initialize BD663474 */
	BD663474_wr_cmd(0x0000);	/* oschilliation start */
	BD663474_wr_dat(0x0001);
	_delay_ms(1);

	/* Power settings */
	BD663474_wr_cmd(0x0100);	/*power supply setup*/
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0101);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0102);
	BD663474_wr_dat(0x3110);
	BD663474_wr_cmd(0x0103);
	BD663474_wr_dat(0xE200);
	BD663474_wr_cmd(0x0110);
	BD663474_wr_dat(0x009D);
	BD663474_wr_cmd(0x0111);
	BD663474_wr_dat(0x0022);
	BD663474_wr_cmd(0x0100);
	BD663474_wr_dat(0x0120);
	_delay_ms(2);

	BD663474_wr_cmd(0x0100);
	BD663474_wr_dat(0x3120);
	_delay_ms(8);
	
	/* Display control */
	BD663474_wr_cmd(0x0002);
	BD663474_wr_dat(0x0000);
#ifdef LANDSCAPE
	BD663474_wr_cmd(0x0001);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0003);
	BD663474_wr_dat(0x12B8);
#else
	BD663474_wr_cmd(0x0001);
	BD663474_wr_dat(0x0100);
	BD663474_wr_cmd(0x0003);
	BD663474_wr_dat(0x1230);
#endif
	BD663474_wr_cmd(0x0006);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0007);
	BD663474_wr_dat(0x0101);
	BD663474_wr_cmd(0x0008);
	BD663474_wr_dat(0x0808);
	BD663474_wr_cmd(0x0009);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x000B);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x000C);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x000D);
	BD663474_wr_dat(0x0018);

	/* LTPS control settings */
	BD663474_wr_cmd(0x0012);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0013);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0018);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0019);
	BD663474_wr_dat(0x0000);

	BD663474_wr_cmd(0x0203);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0204);
	BD663474_wr_dat(0x0000);

	BD663474_wr_cmd(0x0210);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0211);
	BD663474_wr_dat(0x00EF);
	BD663474_wr_cmd(0x0212);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0213);
	BD663474_wr_dat(0x013F);
	BD663474_wr_cmd(0x0214);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0215);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0216);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0217);
	BD663474_wr_dat(0x0000);

	/* Gray scale settings */
	BD663474_wr_cmd(0x0300);
	BD663474_wr_dat(0x5343);
	BD663474_wr_cmd(0x0301);
	BD663474_wr_dat(0x1021);
	BD663474_wr_cmd(0x0302);
	BD663474_wr_dat(0x0003);
	BD663474_wr_cmd(0x0303);
	BD663474_wr_dat(0x0011);
	BD663474_wr_cmd(0x0304);
	BD663474_wr_dat(0x050A);
	BD663474_wr_cmd(0x0305);
	BD663474_wr_dat(0x4342);
	BD663474_wr_cmd(0x0306);
	BD663474_wr_dat(0x1100);
	BD663474_wr_cmd(0x0307);
	BD663474_wr_dat(0x0003);
	BD663474_wr_cmd(0x0308);
	BD663474_wr_dat(0x1201);
	BD663474_wr_cmd(0x0309);
	BD663474_wr_dat(0x050A);

	/* RAM access settINGS */
	BD663474_wr_cmd(0x0400);
	BD663474_wr_dat(0x4027);
	BD663474_wr_cmd(0x0401);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0402);
	BD663474_wr_dat(0x0000);	/* First screen drive position (1) */
	BD663474_wr_cmd(0x0403);
	BD663474_wr_dat(0x013F);	/* First screen drive position (2) */
	BD663474_wr_cmd(0x0404);
	BD663474_wr_dat(0x0000);

	BD663474_wr_cmd(0x0200);
	BD663474_wr_dat(0x0000);
	BD663474_wr_cmd(0x0201);
	BD663474_wr_dat(0x0000);

	BD663474_wr_cmd(0x0100);
	BD663474_wr_dat(0x7120);
	BD663474_wr_cmd(0x0007);
	BD663474_wr_dat(0x0103);
	_delay_ms(1);
	BD663474_wr_cmd(0x0007);
	BD663474_wr_dat(0x0113);


	BD663474_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	BD663474_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		BD663474_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
