/********************************************************************************/
/*!
	@file			spfd54126.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive MF_LCM2.0_V3 TFT module(8/16bit mode).

    @section HISTORY
		2012.09.30	V1.00	Stable Release.
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "spfd54126.h"
/* check header file version for fool proof */
#if SPFD54126_H != 0x0400
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
inline void SPFD54126_reset(void)
{
	SPFD54126_RES_SET();						/* RES=H, RD=H, WR=H   		*/
	SPFD54126_RD_SET();
	SPFD54126_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	SPFD54126_RES_CLR();						/* RES=L, CS=L   			*/
	SPFD54126_CS_CLR();

	_delay_ms(20);								/* wait 20ms     			*/
	SPFD54126_RES_SET();						/* RES=H					*/
	_delay_ms(150);				    			/* wait over 120ms     		*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SPFD54126_wr_cmd(uint8_t cmd)
{
	SPFD54126_DC_CLR();							/* DC=L		     */
	
	SPFD54126_CMD = cmd;						/* D7..D0=cmd    */
	SPFD54126_WR();								/* WR=L->H       */
	
	SPFD54126_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void SPFD54126_wr_dat(uint8_t dat)
{
	SPFD54126_DATA = dat;						/* D7..D0=dat    */
	SPFD54126_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void SPFD54126_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	SPFD54126_DATA = (uint8_t)(gram>>8);		/* upper 8bit data		*/
	SPFD54126_WR();								/* WR=L->H				*/
	SPFD54126_DATA = (uint8_t)gram;				/* lower 8bit data		*/
#else
	SPFD54126_DATA = gram;						/* 16bit data			*/
#endif
	SPFD54126_WR();								/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t SPFD54126_rd_cmd(uint8_t cmd)
{
	uint16_t val,temp;

	SPFD54126_wr_cmd(cmd);
	SPFD54126_WR_SET();

    ReadLCDData(temp);						/* Dummy Read				*/
    ReadLCDData(temp);						/* Upper Read				*/
    ReadLCDData(val);						/* Lower Read				*/

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SPFD54126_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		SPFD54126_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		SPFD54126_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SPFD54126_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	SPFD54126_wr_cmd(CASET); 
	SPFD54126_wr_dat(0);
	SPFD54126_wr_dat(OFS_COL + x);
	SPFD54126_wr_dat(0);
	SPFD54126_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	SPFD54126_wr_cmd(RASET);
	SPFD54126_wr_dat(0);
	SPFD54126_wr_dat(OFS_RAW + y); 
	SPFD54126_wr_dat(0);
	SPFD54126_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	SPFD54126_wr_cmd(RAMWR);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SPFD54126_clear(void)
{
	volatile uint32_t n;

	SPFD54126_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		SPFD54126_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void SPFD54126_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	SPFD54126_reset();

	/* Check Device Code */
	devicetype = SPFD54126_rd_cmd(RDID4);  		/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0616)
	{
		/* Initialize SPFD54126 */
		SPFD54126_wr_cmd(SLPOUT);
		_delay_ms(100);
		
		SPFD54126_wr_cmd(VMCTR1);
		SPFD54126_wr_dat(0xC5);
		
		SPFD54126_wr_cmd(0xF0);
		SPFD54126_wr_dat(0x5A);
		
		SPFD54126_wr_cmd(0xF2);
		SPFD54126_wr_dat(0x01);
		
		SPFD54126_wr_cmd(GAMCTRP1);
		SPFD54126_wr_dat(0x00);
		SPFD54126_wr_dat(0x01);
		SPFD54126_wr_dat(0x06);
		SPFD54126_wr_dat(0x2E);
		SPFD54126_wr_dat(0x2B);
		SPFD54126_wr_dat(0x0B);
		SPFD54126_wr_dat(0x1A);
		SPFD54126_wr_dat(0x02);
		SPFD54126_wr_dat(0x06);
		SPFD54126_wr_dat(0x05);
		SPFD54126_wr_dat(0x0C);
		SPFD54126_wr_dat(0x0D);
		SPFD54126_wr_dat(0x00);
		SPFD54126_wr_dat(0x05);
		SPFD54126_wr_dat(0x02);
		SPFD54126_wr_dat(0x05);
		
		SPFD54126_wr_cmd(GAMCTRN1);
		SPFD54126_wr_dat(0x06);
		SPFD54126_wr_dat(0x23);
		SPFD54126_wr_dat(0x25);
		SPFD54126_wr_dat(0x0F);
		SPFD54126_wr_dat(0x0A);
		SPFD54126_wr_dat(0x04);
		SPFD54126_wr_dat(0x02);
		SPFD54126_wr_dat(0x1A);
		SPFD54126_wr_dat(0x05);
		SPFD54126_wr_dat(0x03);
		SPFD54126_wr_dat(0x06);
		SPFD54126_wr_dat(0x01);
		SPFD54126_wr_dat(0x0C);
		SPFD54126_wr_dat(0x0B);
		SPFD54126_wr_dat(0x05);
		SPFD54126_wr_dat(0x05);
		
		SPFD54126_wr_cmd(COLMOD);
		SPFD54126_wr_dat(0x65);
		
		SPFD54126_wr_cmd(DISPON);
		_delay_ms(10); 
	}

	else { for(;;);} /* Invalid Device Code!! */

	SPFD54126_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	SPFD54126_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		SPFD54126_wr_gram(COL_RED);
	} while (--n);


	_delay_ms(500);
	for(;;);

#endif

}


/* End Of File ---------------------------------------------------------------*/
