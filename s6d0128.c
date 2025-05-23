/********************************************************************************/
/*!
	@file			s6d0128.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive AS021350D TFT module(8/16bit mode).

    @section HISTORY
		2011.04.30	V1.00	Stable Release
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2011.12.23	V3.00	Optimize Some Codes.
		2023.05.01	V4.00	Removed unused delay function.
		2023.08.01	V5.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s6d0128.h"
/* check header file version for fool proof */
#if S6D0128_H != 0x0500
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
inline void S6D0128_reset(void)
{
	S6D0128_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S6D0128_RD_SET();
	S6D0128_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	S6D0128_RES_CLR();							/* RES=L, CS=L   			*/
	S6D0128_CS_CLR();
	_delay_ms(60);								/* wait 60ms     			*/
	
	S6D0128_RES_SET();						  	/* RES=H					*/
	_delay_ms(10);				    			/* wait 10ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D0128_wr_cmd(uint8_t cmd)
{
	S6D0128_DC_CLR();							/* DC=L						*/

	S6D0128_CMD = cmd;							/* cmd(8bit)				*/
	S6D0128_WR();								/* WR=L->H					*/

	S6D0128_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void S6D0128_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6D0128_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	S6D0128_WR();								/* WR=L->H					*/
	S6D0128_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	S6D0128_DATA = dat;							/* 16bit data 				*/
#endif
	S6D0128_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6D0128_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		S6D0128_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		S6D0128_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void S6D0128_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	S6D0128_wr_cmd(0x44);				/* Horizontal Start,End ADDR */
	S6D0128_wr_dat(((OFS_COL + width)<<8)|(OFS_COL + x));

	S6D0128_wr_cmd(0x45);				/* Vertical Start,End ADDR */
	S6D0128_wr_dat(((OFS_RAW + height)<<8)|(OFS_RAW + y));

	S6D0128_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD15) */
	S6D0128_wr_dat(((OFS_RAW + y)<<8)|(OFS_COL + x));

	S6D0128_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S6D0128_clear(void)
{
	volatile uint32_t n;

	S6D0128_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6D0128_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t S6D0128_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	S6D0128_wr_cmd(cmd);
	S6D0128_WR_SET();

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
void S6D0128_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	S6D0128_reset();

	/* Check Device Code */
	devicetype = S6D0128_rd_cmd(0x0000);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0128)
	{
		/* Initialize S6D0128 */
		S6D0128_wr_cmd(0x00); S6D0128_wr_dat(0x0001);
		S6D0128_wr_cmd(0x11); S6D0128_wr_dat(0x3003);	/* power control:PON=0,,PON1=0,AON=0 */
		S6D0128_wr_cmd(0x14); S6D0128_wr_dat(0x2D2A);	/* power control */
		S6D0128_wr_cmd(0x10); S6D0128_wr_dat(0x1B00);	/* power control */
		_delay_ms(50); 
		S6D0128_wr_cmd(0x13); S6D0128_wr_dat(0x0070);	/* power control :PON=1,PON1=0,AON=0 */
		_delay_ms(40); 
		
		S6D0128_wr_cmd(0x01); S6D0128_wr_dat(0x011B);	/* Driver Output Control */
														/* 0000 0xxx 000x xxxx---0000 0001 0001 1101 */
														/* SM=0,GS=0,SS=1,NL[4:0]=1 0101---176*220 pixels */
		S6D0128_wr_cmd(0x02); S6D0128_wr_dat(0x0700);	/* LCD-Driving Waveform Control */
														/* 0000 xxxx 00xx xxxx---0000 0100 0000 0000 */
														/* FLD[1:0]=01,B/C=0,EOR=0,NW[5:0]=000000 */
														
		S6D0128_wr_cmd(0x03); S6D0128_wr_dat(0x1030);	/* Entry Mode */
		
		S6D0128_wr_cmd(0x08); S6D0128_wr_dat(0x0102);	/* display control---blink control */
		S6D0128_wr_cmd(0x0B); S6D0128_wr_dat(0x0000); 
		S6D0128_wr_cmd(0x0C); S6D0128_wr_dat(0x0003);	/* display interface selection:16-bit RGB */
		
		S6D0128_wr_cmd(0x40); S6D0128_wr_dat(0x0000);	/* Gate Scan Position */
		S6D0128_wr_cmd(0x41); S6D0128_wr_dat(0x00DB);	/* Gate Scan Position */
		
		S6D0128_wr_cmd(0x42); S6D0128_wr_dat(0xDB00);	/* Screen Driving Position */
		S6D0128_wr_cmd(0x43); S6D0128_wr_dat(0x0000);	/* Screen Driving Position */
		S6D0128_wr_cmd(0x44); S6D0128_wr_dat(0xAF00);	/* Horzontal RAM Address Position:176 */
		S6D0128_wr_cmd(0x45); S6D0128_wr_dat(0xDB00);	/* Vertical Address Position:220 */
		
		/* MP[5:0]([2:0]):gamma adjustment regiSTER for positive polarity output */
		S6D0128_wr_cmd(0x30); S6D0128_wr_dat(0x0000);   /* Gamma Control */
		S6D0128_wr_cmd(0x31); S6D0128_wr_dat(0x0406);   /* Gamma Control */
		S6D0128_wr_cmd(0x32); S6D0128_wr_dat(0x0102);   /* Gamma Control */
		S6D0128_wr_cmd(0x33); S6D0128_wr_dat(0x0202);   /* Gamma Control */
		
		S6D0128_wr_cmd(0x38); S6D0128_wr_dat(0x0000);   /* Gamma Control */
		
		S6D0128_wr_cmd(0x34); S6D0128_wr_dat(0x0306);   /* Gamma Control */
		S6D0128_wr_cmd(0x35); S6D0128_wr_dat(0x0103);   /* Gamma Control */
		S6D0128_wr_cmd(0x36); S6D0128_wr_dat(0x0707);   /* Gamma Control */
		S6D0128_wr_cmd(0x37); S6D0128_wr_dat(0x0202);   /* Gamma Control */
		
		S6D0128_wr_cmd(0x39); S6D0128_wr_dat(0x0202);   /* Gamma Control */
		
		S6D0128_wr_cmd(0x07); S6D0128_wr_dat(0x0016); 	/* GON=1,DTE=0,D[1:0]=01 */
		_delay_ms(40); 
		
		S6D0128_wr_cmd(0x07); S6D0128_wr_dat(0x0017); 	/* GON=1,DTE=0,D[1:0]=11 */
		S6D0128_wr_cmd(0x21); S6D0128_wr_dat(0x0000); 	/* 0000 0000 0000 0000 */
		S6D0128_wr_cmd(0x07); S6D0128_wr_dat(0x0014); 	/* GON=1,DTE=0,D[1:0]=11 */
		S6D0128_wr_cmd(0x07); S6D0128_wr_dat(0x0016); 	/* GON=1,DTE=0,D[1:0]=11 */
		_delay_ms(50);
		
		S6D0128_wr_cmd(0x07); S6D0128_wr_dat(0x0017); 	/* GON=1,DTE=0,D[1:0]=11 */
	}

	else { for(;;);} /* Invalid Device Code!! */

	S6D0128_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	S6D0128_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	do {
		S6D0128_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
