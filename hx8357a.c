/********************************************************************************/
/*!
	@file			hx8357a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TFT1N3277-E TFT module(8/16bit mode).

    @section HISTORY
		2012.06.30	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8357a.h"
/* check header file version for fool proof */
#if HX8357A_H != 0x0300
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
inline void HX8357A_reset(void)
{
	HX8357A_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX8357A_RD_SET();
	HX8357A_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/
	
	HX8357A_RES_CLR();							/* RES=L, CS=L   			*/
	HX8357A_CS_CLR();

	_delay_ms(10);								/* wait 10ms     			*/
	
	HX8357A_RES_SET();						  	/* RES=H					*/
	_delay_ms(10);				    			/* wait 10ms     			*/
}


/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8357A_wr_cmd(uint8_t cmd)
{
	HX8357A_DC_CLR();							/* DC=L						*/

	HX8357A_CMD = cmd;							/* cmd(8bit)				*/
	HX8357A_WR();								/* WR=L->H					*/

	HX8357A_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8357A_wr_dat(uint8_t dat)
{
	HX8357A_DATA = dat;							/* data						*/
	HX8357A_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8357A_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8357A_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	HX8357A_WR();								/* WR=L->H					*/
	HX8357A_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	HX8357A_DATA = gram;						/* 16bit data 				*/
#endif
	HX8357A_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8357A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8357A_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		HX8357A_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t HX8357A_rd_cmd(uint8_t cmd)
{
	uint8_t val;

	HX8357A_wr_cmd(cmd);
	HX8357A_WR_SET();

	_delay_ms(1);		/* Some Wait Must be Need on HX8347A, Nemui-San Said So... */
    ReadLCDData(val);

	val &= 0x00FF;
	return val;
}


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8357A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8357A_wr_cmd(0x02);				/* Horizontal RAM Start ADDR2 */
	HX8357A_wr_dat((OFS_COL + x)>>8);
	HX8357A_wr_cmd(0x03);				/* Horizontal RAM Start ADDR1 */
	HX8357A_wr_dat(OFS_COL + x);
	HX8357A_wr_cmd(0x04);				/* Horizontal RAM End ADDR2 */
	HX8357A_wr_dat((OFS_COL + width)>>8);
	HX8357A_wr_cmd(0x05);				/* Horizontal RAM End ADDR1 */
	HX8357A_wr_dat(OFS_COL + width);
	HX8357A_wr_cmd(0x06);				/* Vertical RAM Start ADDR2 */
	HX8357A_wr_dat((OFS_RAW + y)>>8);
	HX8357A_wr_cmd(0x07);				/* Vertical RAM Start ADDR1 */
	HX8357A_wr_dat(OFS_RAW + y);
	HX8357A_wr_cmd(0x08);				/* Vertical RAM End ADDR2 */
	HX8357A_wr_dat((OFS_RAW + height)>>8);
	HX8357A_wr_cmd(0x09);				/* Vertical RAM End ADDR1 */
	HX8357A_wr_dat(OFS_RAW + height);

	HX8357A_wr_cmd(0x80);				/* Horizontal RAM Start ADDR2 */
	HX8357A_wr_dat((OFS_COL + x)>>8);
	HX8357A_wr_cmd(0x81);				/* Horizontal RAM Start ADDR1 */
	HX8357A_wr_dat(OFS_COL + x);
	
	HX8357A_wr_cmd(0x82);				/* Vertical RAM Start ADDR2 */
	HX8357A_wr_dat((OFS_RAW + y)>>8);
	HX8357A_wr_cmd(0x83);				/* Vertical RAM Start ADDR1 */
	HX8357A_wr_dat(OFS_RAW + y);

	HX8357A_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8357A_clear(void)
{
	volatile uint32_t n;

	HX8357A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8357A_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8357A_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	HX8357A_reset();

	/* Check Device Code */
	devicetype = HX8357A_rd_cmd(0x00);  		/* Confirm Vaild LCD Controller */

	if(devicetype == 0x57)
	{
		/* Initialize HX8357A */
		HX8357A_wr_cmd(0xFF);					/* Command page select */
		HX8357A_wr_dat(0x00);
		HX8357A_wr_cmd(0x72);					/* SUB_PANEL Control */
		HX8357A_wr_dat(0xF6);
		
		/* power saving for HX8357-A */
		HX8357A_wr_cmd(0xFF);					/* Command page select */
		HX8357A_wr_dat(0x00);
		HX8357A_wr_cmd(0xF2);
		HX8357A_wr_dat(0x00);		
		HX8357A_wr_cmd(0xE4);
		HX8357A_wr_dat(0x00);
		HX8357A_wr_cmd(0xE5);
		HX8357A_wr_dat(0x1C);
		HX8357A_wr_cmd(0xE6);
		HX8357A_wr_dat(0x00);
		HX8357A_wr_cmd(0xE7);
		HX8357A_wr_dat(0x1C);
		HX8357A_wr_cmd(0xEE);
		HX8357A_wr_dat(0x42);
		HX8357A_wr_cmd(0xEF);
		HX8357A_wr_dat(0xDB);
		HX8357A_wr_cmd(0x2E);
		HX8357A_wr_dat(0x98);
		
		/* Gamma Correction */
		HX8357A_wr_cmd(0x40);
		HX8357A_wr_dat(0x07);
		HX8357A_wr_cmd(0x41);
		HX8357A_wr_dat(0x2E);
		HX8357A_wr_cmd(0x42);
		HX8357A_wr_dat(0x2C);
		HX8357A_wr_cmd(0x43);
		HX8357A_wr_dat(0x3D);
		HX8357A_wr_cmd(0x44);
		HX8357A_wr_dat(0x38);
		HX8357A_wr_cmd(0x45);
		HX8357A_wr_dat(0x3D);
		HX8357A_wr_cmd(0x46);
		HX8357A_wr_dat(0x27);
		HX8357A_wr_cmd(0x47);
		HX8357A_wr_dat(0x76);
		HX8357A_wr_cmd(0x48);
		HX8357A_wr_dat(0x08);
		HX8357A_wr_cmd(0x49);
		HX8357A_wr_dat(0x06);
		HX8357A_wr_cmd(0x4A);
		HX8357A_wr_dat(0x06);
		HX8357A_wr_cmd(0x4B);
		HX8357A_wr_dat(0x0C);
		HX8357A_wr_cmd(0x4C);
		HX8357A_wr_dat(0x17);
		
		HX8357A_wr_cmd(0x50);
		HX8357A_wr_dat(0x02);
		HX8357A_wr_cmd(0x51);
		HX8357A_wr_dat(0x07);
		HX8357A_wr_cmd(0x52);
		HX8357A_wr_dat(0x02);
		HX8357A_wr_cmd(0x53);
		HX8357A_wr_dat(0x13);
		HX8357A_wr_cmd(0x54);
		HX8357A_wr_dat(0x11);
		HX8357A_wr_cmd(0x55);
		HX8357A_wr_dat(0x3D);
		HX8357A_wr_cmd(0x56);
		HX8357A_wr_dat(0x09);
		HX8357A_wr_cmd(0x57);
		HX8357A_wr_dat(0x58);
		HX8357A_wr_cmd(0x58);
		HX8357A_wr_dat(0x08);
		HX8357A_wr_cmd(0x59);
		HX8357A_wr_dat(0x13);
		HX8357A_wr_cmd(0x5A);
		HX8357A_wr_dat(0x19);
		HX8357A_wr_cmd(0x5B);
		HX8357A_wr_dat(0x19);
		HX8357A_wr_cmd(0x5C);
		HX8357A_wr_dat(0x17);
		HX8357A_wr_cmd(0x5D);
		HX8357A_wr_dat(0x3C);
		
		/* Set GRAM area 320*480 */
		/*
		HX8357A_wr_cmd(0x02);
		HX8357A_wr_dat(0x00);
		HX8357A_wr_cmd(0x03);
		HX8357A_wr_dat(0x00);
		HX8357A_wr_cmd(0x04);
		HX8357A_wr_dat(0x01);
		HX8357A_wr_cmd(0x05);
		HX8357A_wr_dat(0x3F);
		HX8357A_wr_cmd(0x06);
		HX8357A_wr_dat(0x00);
		HX8357A_wr_cmd(0x07);
		HX8357A_wr_dat(0x00);
		HX8357A_wr_cmd(0x08);
		HX8357A_wr_dat(0x01);
		HX8357A_wr_cmd(0x09);
		HX8357A_wr_dat(0xDF);
		*/
		/* power setting */
		HX8357A_wr_cmd(0x24);
		HX8357A_wr_dat(0x64);
		HX8357A_wr_cmd(0x25);
		HX8357A_wr_dat(0x71);
		HX8357A_wr_cmd(0x23);
		HX8357A_wr_dat(0x52);
		HX8357A_wr_cmd(0x1B);
		HX8357A_wr_dat(0x1E);
		HX8357A_wr_cmd(0x1D);
		HX8357A_wr_dat(0x11);
		
		/* power on setting */
		HX8357A_wr_cmd(0x19);
		HX8357A_wr_dat(0x01);
		HX8357A_wr_cmd(0x1C);
		HX8357A_wr_dat(0x03);
		HX8357A_wr_cmd(0x01);
		HX8357A_wr_dat(0x00);
		HX8357A_wr_cmd(0x1F);
		HX8357A_wr_dat(0x80);
		_delay_ms(5);
		
		HX8357A_wr_cmd(0x1F);
		HX8357A_wr_dat(0x90);
		_delay_ms(5);
		
		HX8357A_wr_cmd(0x1F);
		HX8357A_wr_dat(0xD4);
		_delay_ms(5);
		
		/* Display on setting */
		HX8357A_wr_cmd(0x28);
		HX8357A_wr_dat(0x08);
		_delay_ms(40);
		
		HX8357A_wr_cmd(0x28);
		HX8357A_wr_dat(0x38);
		_delay_ms(40);
		
		HX8357A_wr_cmd(0x28);
		HX8357A_wr_dat(0x3c);	
		HX8357A_wr_cmd(0x17);
		HX8357A_wr_dat(0x05);
		HX8357A_wr_cmd(0x16);		/* Memory access control register */
		HX8357A_wr_dat((0<<7)|(0<<6)|(0<<5)|(1<<3)|(0<<1)|(1<<0));
	}

	else { for(;;);}					/* Invalid Device Code!! */

	HX8357A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8357A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8357A_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
