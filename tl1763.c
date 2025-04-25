/********************************************************************************/
/*!
	@file			tl1763.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -FPC-SH9705				(TL1763)	16bit mode only.

    @section HISTORY
		2014.08.01	V1.00	Stable Release.
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "tl1763.h"
/* check header file version for fool proof */
#if TL1763_H != 0x0400
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
inline void TL1763_reset(void)
{
	TL1763_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	TL1763_RD_SET();
	TL1763_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	TL1763_RES_CLR();							/* RES=L, CS=L   			*/
	TL1763_CS_CLR();

	_delay_ms(50);								/* wait 50ms     			*/
	TL1763_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void TL1763_wr_cmd(uint8_t cmd)
{
	TL1763_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	TL1763_CMD = 0;
	TL1763_WR();
#endif

	TL1763_CMD = (uint8_t)cmd;					/* cmd(8bit_Low or 16bit)	*/
	TL1763_WR();								/* WR=L->H					*/

	TL1763_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void TL1763_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	TL1763_DATA = (uint8_t)(dat>>8);			/* upper 8bit data		*/
	TL1763_WR();								/* WR=L->H				*/
	TL1763_DATA = (uint8_t)dat;					/* lower 8bit data		*/
#else
	TL1763_DATA = dat;							/* 16bit data			*/
#endif
	TL1763_WR();								/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void TL1763_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		TL1763_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		TL1763_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t TL1763_rd_cmd(uint8_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	TL1763_wr_cmd(cmd);
	TL1763_WR_SET();

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
    Set Rectangle.
*/
/**************************************************************************/
inline void TL1763_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	TL1763_wr_cmd(0x46);				/* Horizontal RAM Start/End ADDR */
	TL1763_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));
	TL1763_wr_cmd(0x48);				/* Vertical RAM Start ADDR */
	TL1763_wr_dat(OFS_RAW + y);
	TL1763_wr_cmd(0x47);				/* Vertical End ADDR */
	TL1763_wr_dat(OFS_RAW + height);

	TL1763_wr_cmd(0x20);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	TL1763_wr_dat(OFS_COL + x);
	TL1763_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	TL1763_wr_dat(OFS_RAW + y);

	TL1763_wr_cmd(0x22);				/* Write Data to GRAM */
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void TL1763_clear(void)
{
	volatile uint32_t n;

	TL1763_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		TL1763_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void TL1763_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	TL1763_reset();

	/* Check Device Code */
	devicetype = TL1763_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x1763)
	{
		/* Initialize TL1763 */	
		/* Start Initial Sequence */
		TL1763_wr_cmd(0x11);
		TL1763_wr_dat(0x0001);
		TL1763_wr_cmd(0x13);
		TL1763_wr_dat(0x1334);
		TL1763_wr_cmd(0x10);
		TL1763_wr_dat(0x0530);
		TL1763_wr_cmd(0x12);
		TL1763_wr_dat(0x101D);
		_delay_ms(100);
		
		TL1763_wr_cmd(0x13);
		TL1763_wr_dat(0x3334);
		TL1763_wr_cmd(0x11);
		TL1763_wr_dat(0x0111);
		TL1763_wr_cmd(0x10);
		TL1763_wr_dat(0x1530); 
		_delay_ms(100);
		
		/* Initial Setting */
		TL1763_wr_cmd(0x01);
		TL1763_wr_dat(0x0100);
		TL1763_wr_cmd(0x02);
		TL1763_wr_dat(0x0700);
		TL1763_wr_cmd(0x03);
		TL1763_wr_dat(0x1030);
		TL1763_wr_cmd(0x04);
		TL1763_wr_dat(0x0141);
		TL1763_wr_cmd(0x07);
		TL1763_wr_dat(0x0005);
		TL1763_wr_cmd(0x08);
		TL1763_wr_dat(0x0202);
		TL1763_wr_cmd(0x0B);
		TL1763_wr_dat(0x0000);
		TL1763_wr_cmd(0x0C);
		TL1763_wr_dat(0x0000);
		TL1763_wr_cmd(0x0E);
		TL1763_wr_dat(0x0015);
		TL1763_wr_cmd(0xA1);
		TL1763_wr_dat(0x1000);
		TL1763_wr_cmd(0xA4);
		TL1763_wr_dat(0x7300);
		TL1763_wr_cmd(0x30);
		TL1763_wr_dat(0x0202);
		TL1763_wr_cmd(0x31);
		TL1763_wr_dat(0x0000);
		TL1763_wr_cmd(0x32);
		TL1763_wr_dat(0x0404);
		TL1763_wr_cmd(0x33);
		TL1763_wr_dat(0x0203);
		TL1763_wr_cmd(0x34);
		TL1763_wr_dat(0x0207);
		TL1763_wr_cmd(0x35);
		TL1763_wr_dat(0x0202);
		TL1763_wr_cmd(0x36);
		TL1763_wr_dat(0x0707);
		TL1763_wr_cmd(0x37);
		TL1763_wr_dat(0x0002);
		TL1763_wr_cmd(0x38);
		TL1763_wr_dat(0x0F00);
		TL1763_wr_cmd(0x39);
		TL1763_wr_dat(0x0002);
		TL1763_wr_cmd(0x40);
		TL1763_wr_dat(0x0000);
		TL1763_wr_cmd(0x41);
		TL1763_wr_dat(0x0000);
		TL1763_wr_cmd(0x42);
		TL1763_wr_dat(0x0140);
		TL1763_wr_cmd(0x43);
		TL1763_wr_dat(0x0000);
		TL1763_wr_cmd(0x44);
		TL1763_wr_dat(0x0000);
		TL1763_wr_cmd(0x45);
		TL1763_wr_dat(0x0000);
		TL1763_wr_cmd(0x46);
		TL1763_wr_dat(0xEF00);
		TL1763_wr_cmd(0x47);
		TL1763_wr_dat(0x013F);
		TL1763_wr_cmd(0x48);
		TL1763_wr_dat(0x0000);
		_delay_ms(100);
		
		/* Display on Setting */
		TL1763_wr_cmd(0x07);
		TL1763_wr_dat(0x0025);
		TL1763_wr_cmd(0x07);
		TL1763_wr_dat(0x0027);
		_delay_ms(100);
		
		TL1763_wr_cmd(0x07);
		TL1763_wr_dat(0x0037);
	}

	else { for(;;);} /* Invalid Device Code!! */

	TL1763_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	TL1763_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		TL1763_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
