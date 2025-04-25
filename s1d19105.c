/********************************************************************************/
/*!
	@file			s1d19105.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive FPC-SH9516-11A TFT module(8bit mode only).

    @section HISTORY
		2011.06.08	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s1d19105.h"
/* check header file version for fool proof */
#if S1D19105_H != 0x0400
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
inline void S1D19105_reset(void)
{
	S1D19105_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S1D19105_RD_SET();
	S1D19105_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	S1D19105_RES_CLR();							/* RES=L, CS=L   			*/
	S1D19105_CS_CLR();

	_delay_ms(20);								/* wait 20ms     			*/
	S1D19105_RES_SET();						  	/* RES=H					*/
	_delay_ms(20);				    			/* wait 20ms     			*/
}


/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S1D19105_wr_cmd(uint8_t cmd)
{
	S1D19105_DC_CLR();							/* DC=L		     			*/
	
	S1D19105_CMD = cmd;							/* D7..D0=cmd   		 	*/
	S1D19105_WR();								/* WR=L->H      		 	*/
	
	S1D19105_DC_SET();							/* DC=H   	     			*/
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void S1D19105_wr_dat(uint8_t dat)
{
	S1D19105_DATA = dat;						/* D7..D0=dat    			*/
	S1D19105_WR();								/* WR=L->H       			*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void S1D19105_wr_gram(uint16_t gram)
{
	S1D19105_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	S1D19105_WR();								/* WR=L->H					*/

	S1D19105_DATA = (uint8_t)gram;				/* lower 8bit data			*/
	S1D19105_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S1D19105_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		S1D19105_wr_dat(*p++);
		S1D19105_wr_dat(*p++);
		S1D19105_wr_dat(*p++);
		S1D19105_wr_dat(*p++);
	}
	while (n--) {
		S1D19105_wr_dat(*p++);
	}
#endif

}


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void S1D19105_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set START Address */
	S1D19105_wr_cmd(0x15); 
	S1D19105_wr_dat(OFS_COL + x);
	S1D19105_wr_dat(OFS_RAW + y); 
	
	/* Set END Address */
	S1D19105_wr_cmd(0x75);
	S1D19105_wr_dat(OFS_COL + width);
	S1D19105_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	S1D19105_wr_cmd(0x5C);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S1D19105_clear(void)
{
	volatile uint32_t n;

	S1D19105_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		S1D19105_wr_gram(COL_BLACK);
	} while (--n);
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t S1D19105_rd_cmd(uint8_t cmd)
{
	uint8_t val;

    ReadLCDData(val);

	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S1D19105_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	S1D19105_reset();

	/* Check Device Code */
	devicetype = S1D19105_rd_cmd(0xE9);  		/* Confirm Vaild LCD Controller */

	if(devicetype == 0x10)						/* Revision 0.1.0 read nemui-san */
	{
		/* Initialize S1D19105 */
		S1D19105_wr_cmd(0x99);
		_delay_ms(1);
		S1D19105_wr_cmd(0x94);
		S1D19105_wr_cmd(0x96);					/* Starts up Vcore and Vosc. */
		
		S1D19105_wr_cmd(0x92);					/* Starts the built-in oscillation circuit. */
		_delay_ms(1);
		
		S1D19105_wr_cmd(0xCA);					/* SET DISPLAY */
		S1D19105_wr_dat(0x00);
		S1D19105_wr_dat(0x3E);
		S1D19105_wr_dat(0x00);
		S1D19105_wr_dat(0x55);
		S1D19105_wr_dat(0xDB);
		S1D19105_wr_dat(0x00);
		S1D19105_wr_dat(0x02);
		S1D19105_wr_dat(0x01);
		S1D19105_wr_dat(0x00);
		
		S1D19105_wr_cmd(0xA1);					/* SET DISPLAY */
		S1D19105_wr_dat(0x01);
		S1D19105_wr_dat(0x39);
		S1D19105_wr_dat(0x02);
		S1D19105_wr_dat(0x37);
		S1D19105_wr_dat(0x04);
		S1D19105_wr_dat(0x12);
		S1D19105_wr_dat(0x2F);
		
		S1D19105_wr_cmd(0xBC);					/* SET DISPLAY */
		S1D19105_wr_dat(0x00);
		
		S1D19105_wr_cmd(0xAB);					/* SET DISPLAY */
		S1D19105_wr_dat(0x00);
		
		S1D19105_wr_cmd(0x31);					/* SET DISPLAY */
		S1D19105_wr_dat(0x00);
		S1D19105_wr_dat(0x00);
		
		S1D19105_wr_cmd(0x8B);					/* SET DISPLAY */
		S1D19105_wr_dat(0x00);
		S1D19105_wr_dat(0x11);
		S1D19105_wr_dat(0x34);
		S1D19105_wr_dat(0x44);
		
		S1D19105_wr_cmd(0x6F);					/* SET DISPLAY */
		S1D19105_wr_dat(0x00);
		S1D19105_wr_dat(0x00);
		S1D19105_wr_dat(0xDB);
		
		S1D19105_wr_cmd(0x8C);					/* SET DISPLAY */
		S1D19105_wr_dat(0x00);
		S1D19105_wr_dat(0x00);
		
		S1D19105_wr_cmd(0x20);					/* SET DISPLAY */
		S1D19105_wr_dat(0x0F);
		S1D19105_wr_dat(0x0A);
		S1D19105_wr_dat(0x08);
		S1D19105_wr_dat(0x0F);
		S1D19105_wr_dat(0x0D);
		S1D19105_wr_dat(0x0C);
		S1D19105_wr_dat(0x00);
		S1D19105_wr_dat(0x00);
		
		S1D19105_wr_cmd(0x21);					/* SET power control */
		S1D19105_wr_dat(0x11);					/* wait 1.2 */
		S1D19105_wr_dat(0x63);					/* wait 3.4 */
		S1D19105_wr_dat(0x0e);					/* booster circuit */
		S1D19105_wr_dat(0x1F);
		S1D19105_wr_dat(0xAD);
		S1D19105_wr_dat(0x00);
		S1D19105_wr_dat(0x77);
		S1D19105_wr_dat(0xA5);
		S1D19105_wr_dat(0x03);
		S1D19105_wr_dat(0xA5);
		S1D19105_wr_dat(0x03);
		S1D19105_wr_dat(0x44);
		S1D19105_wr_dat(0x77);
		
		S1D19105_wr_cmd(0x22);					/* SET DISPLAY */
		S1D19105_wr_dat(0x63);
		S1D19105_wr_dat(0x33);
		S1D19105_wr_dat(0x44);
		S1D19105_wr_dat(0x76);
		S1D19105_wr_dat(0x35);
		S1D19105_wr_dat(0x10);
		
		S1D19105_wr_cmd(0x23);					/* Set Partial Power control */
		S1D19105_wr_dat(0x01);					/* VCOM ability setting */
		S1D19105_wr_dat(0x44);					/* Boosting clock frequency */
		S1D19105_wr_dat(0x1F);					/* Regulator 1 */
		S1D19105_wr_dat(0xAD);					/* Regulator 2 */
		S1D19105_wr_dat(0x00);					/* Regulator 3 */
		S1D19105_wr_dat(0x44);					/* Gray scale amplifier ability */
		S1D19105_wr_dat(0x77);					/* bias setting */
		
		S1D19105_wr_cmd(0x94);					/* Execute Automatic On Sequence Comman. */
		
		S1D19105_wr_cmd(0xAF);					/* Turns the LCD display ON. */
	}

	else { for(;;);} /* Invalid Device Code!! */

	S1D19105_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	S1D19105_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		S1D19105_wr_gram(COL_RED);
	} while (--n);


	_delay_ms(500);
	for(;;);

#endif

}


/* End Of File ---------------------------------------------------------------*/
