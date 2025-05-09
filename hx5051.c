/********************************************************************************/
/*!
	@file			hx5051.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive C0200QILC-C OLED module(8/16bit mode).

    @section HISTORY
		2011.11.30	V1.00	Stable Release.
		2013.01.03  V2.00	Removed SPI Handling(not need anymore...).
		2014.10.15	V3.00	Fixed 8-bit access bug.
		2023.05.01	V4.00	Removed unused delay function.
		2023.08.01	V5.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx5051.h"
/* check header file version for fool proof */
#if HX5051_H != 0x0500
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
inline void HX5051_reset(void)
{
	HX5051_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX5051_RD_SET();
	HX5051_WR_SET();
	_delay_ms(100);								/* wait 100ms     			*/

	HX5051_RES_CLR();							/* RES=L, CS=L   			*/
	HX5051_CS_CLR();

	_delay_ms(10);								/* wait 10ms     			*/
	HX5051_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX5051_wr_cmd(uint16_t cmd)
{
	HX5051_DC_CLR();						/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX5051_CMD = 0;
	HX5051_WR();
#endif

	HX5051_CMD = (uint8_t)cmd;				/* cmd(8bit_Low or 16bit)	*/
	HX5051_WR();							/* WR=L->H					*/

	HX5051_DC_SET();						/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void HX5051_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX5051_DATA = (uint8_t)(dat>>8);		/* upper 8bit data		*/
	HX5051_WR();							/* WR=L->H				*/
	HX5051_DATA = (uint8_t)dat;				/* lower 8bit data		*/
#else
	HX5051_DATA = dat;						/* 16bit data			*/
#endif
	HX5051_WR();							/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX5051_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX5051_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		HX5051_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t HX5051_rd_cmd(uint16_t cmd)
{
	/* C0200QILC-C OLED module DON'T HAVE RD Pin!! 
	   So CANNOT USE ANY read function.*/
	return  0x8319;
}


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX5051_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	HX5051_wr_cmd(0x23);				/* Horizontal Start,End ADDR */
	HX5051_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	HX5051_wr_cmd(0x24);				/* Vertical Start,End ADDR */
	HX5051_wr_dat(((OFS_RAW + height)<<8)|(OFS_RAW + y));

	HX5051_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD15) */
	HX5051_wr_dat(((OFS_RAW + y)<<8)|(OFS_COL + x));

	HX5051_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX5051_clear(void)
{
	volatile uint32_t n;

	HX5051_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX5051_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX5051_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	HX5051_reset();

	/* Check Device Code */
	devicetype = HX5051_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x8319)
	{
		/* Initialize HX5051 */
		HX5051_wr_cmd(0x07);
		HX5051_wr_dat((1<<11)|(1<<10)|(1<<9)|(0<<8)|(1<<2)|(0<<1));
	
		HX5051_wr_cmd(0x03);
		HX5051_wr_dat(0x0015);
	
		HX5051_wr_cmd(0x01);
		HX5051_wr_dat(0x4740);

		HX5051_wr_cmd(0x21);
		HX5051_wr_dat(0x00AF);
		
		HX5051_wr_cmd(0x05);     
		HX5051_wr_dat((1<<5)|(1<<4)|(0<<3)|(0<<2)|(0<<1)|(0<<0));

		HX5051_wr_cmd(0x1C);
		HX5051_wr_dat((1<<15));

		HX5051_wr_cmd(0x02);
		HX5051_wr_dat(0x0305);

		HX5051_wr_cmd(0x09);
		HX5051_wr_dat((1<<12)|(1<<11)|0x0003);

	}

	else { for(;;);} /* Invalid Device Code!! */

	HX5051_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX5051_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX5051_wr_dat(COL_BLUE);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
