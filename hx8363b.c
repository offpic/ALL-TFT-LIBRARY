/********************************************************************************/
/*!
	@file			hx8363b.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        1.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TFT1P1400-E TFT module(8/16bit mode).

    @section HISTORY
		2023.08.01	V1.00	First Release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8363b.h"
/* check header file version for fool proof */
#if HX8363B_H != 0x0100
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
inline void HX8363B_reset(void)
{
	HX8363B_RES_SET();						/* RES=H, RD=H, WR=H   		*/
	HX8363B_RD_SET();
	HX8363B_WR_SET();
	_delay_ms(10);							/* wait 10ms     			*/

	HX8363B_RES_CLR();						/* RES=L, CS=L   			*/
	HX8363B_CS_CLR();
	_delay_ms(10);							/* wait 10ms     			*/
	
	HX8363B_RES_SET();						/* RES=H					*/
	_delay_ms(125);				    		/* wait over 120ms     		*/
}


/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8363B_wr_cmd(uint8_t cmd)
{
	HX8363B_DC_CLR();						/* DC=L						*/

	HX8363B_CMD = cmd;						/* cmd(8bit_Low or 16bit)	*/
	HX8363B_WR();							/* WR=L->H					*/

	HX8363B_DC_SET();						/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void HX8363B_wr_dat(uint8_t dat)
{
	HX8363B_DATA = dat;						/* data(8bit_Low or 16bit)	*/
	HX8363B_WR();							/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8363B_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8363B_DATA = (uint8_t)(gram>>8);		/* upper 8bit data			*/
	HX8363B_WR();							/* WR=L->H					*/
	HX8363B_DATA = (uint8_t)gram;			/* lower 8bit data			*/
#else
	HX8363B_DATA = gram;					/* 16bit data				*/
#endif
	HX8363B_WR();							/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8363B_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8363B_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		HX8363B_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
uint16_t HX8363B_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	
	/* Unlock manufacturer mode */
 	HX8363B_wr_cmd(0xB9);
	HX8363B_wr_dat(0xFF); 
	HX8363B_wr_dat(0x83); 
	HX8363B_wr_dat(0x63);

	HX8363B_wr_cmd(cmd);
	HX8363B_WR_SET();
    ReadLCDData(val);					/* Dummy */
    ReadLCDData(val);					/* Read Data */
		
	val &= 0x00FF;

	return val;
}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8363B_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8363B_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	HX8363B_wr_dat((OFS_COL + x)>>8);
	HX8363B_wr_dat(OFS_COL + x);
	HX8363B_wr_dat((OFS_COL + width)>>8);
	HX8363B_wr_dat(OFS_COL + width);

	HX8363B_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	HX8363B_wr_dat((OFS_RAW + y)>>8);
	HX8363B_wr_dat(OFS_RAW + y);
	HX8363B_wr_dat((OFS_RAW + height)>>8);
	HX8363B_wr_dat(OFS_RAW + height);

	HX8363B_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8363B_clear(void)
{
	volatile uint32_t n;

	HX8363B_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8363B_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8363B_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	HX8363B_reset();

	/* Check Device Code */
	devicetype = HX8363B_rd_cmd(0xF4);		/* Confirm Vaild LCD Controller */

	if(devicetype  == 0x84)
	{
		/* Initialize HX8363B */
  		HX8363B_wr_cmd(0xB9);
  		HX8363B_wr_dat(0xFF);
  		HX8363B_wr_dat(0x83);
  		HX8363B_wr_dat(0x63);
		
		/* Set_VCOM */
  		HX8363B_wr_cmd(0xB6);
  		HX8363B_wr_dat(0x27);
		
		/* Set_POWER */
  		HX8363B_wr_cmd(0xB1);
  		HX8363B_wr_dat(0x81);
  		HX8363B_wr_dat(0x30);
  		HX8363B_wr_dat(0x07);
  		HX8363B_wr_dat(0x33);
  		HX8363B_wr_dat(0x02);
  		HX8363B_wr_dat(0x13);
  		HX8363B_wr_dat(0x11);
  		HX8363B_wr_dat(0x00);
  		HX8363B_wr_dat(0x24);
  		HX8363B_wr_dat(0x2B);
  		HX8363B_wr_dat(0x3F);
  		HX8363B_wr_dat(0x3F);
		
  		HX8363B_wr_cmd(0xBF);
  		HX8363B_wr_dat(0x00); 
  		HX8363B_wr_dat(0x10); 
		
		/* Set_RGBIF(not used for DBIIF) */
  		HX8363B_wr_cmd(0xB3);
  		HX8363B_wr_dat(0x00);
		
		/* Set_DBIIF(8or16bit) */
		HX8363B_wr_cmd(0xC2);
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
		HX8363B_wr_dat(0x00);		/* mode 8-bit  */
#else
		HX8363B_wr_dat(0x03);		/* mode 16-bit */
#endif
		/* Set_CYC */
  		HX8363B_wr_cmd(0xB4);
  		HX8363B_wr_dat(0x08);
  		HX8363B_wr_dat(0x16);
  		HX8363B_wr_dat(0x5C);
  		HX8363B_wr_dat(0x0B);
  		HX8363B_wr_dat(0x01);
  		HX8363B_wr_dat(0x1E);
  		HX8363B_wr_dat(0x7B);
  		HX8363B_wr_dat(0x01);
  		HX8363B_wr_dat(0x4D);
		
		/* Set_PANEL */
  		HX8363B_wr_cmd(0xCC);
  		HX8363B_wr_dat(0x09);
		_delay_ms(120);
		
		/* Set Gamma 2.2 */
  		HX8363B_wr_cmd(0xE0);
  		HX8363B_wr_dat(0x00);
  		HX8363B_wr_dat(0x1E);
  		HX8363B_wr_dat(0x63);
  		HX8363B_wr_dat(0x15);
  		HX8363B_wr_dat(0x11);
  		HX8363B_wr_dat(0x30);
  		HX8363B_wr_dat(0x0C);
  		HX8363B_wr_dat(0x8F);
  		HX8363B_wr_dat(0x8F);
  		HX8363B_wr_dat(0x15);
  		HX8363B_wr_dat(0x17);
  		HX8363B_wr_dat(0xD5);
  		HX8363B_wr_dat(0x56);
  		HX8363B_wr_dat(0x0E);
  		HX8363B_wr_dat(0x15);
  		HX8363B_wr_dat(0x00);
  		HX8363B_wr_dat(0x1E);
  		HX8363B_wr_dat(0x63);
  		HX8363B_wr_dat(0x15);
  		HX8363B_wr_dat(0x11);
  		HX8363B_wr_dat(0x30);
  		HX8363B_wr_dat(0x0C);
  		HX8363B_wr_dat(0x8F);
  		HX8363B_wr_dat(0x8F);
  		HX8363B_wr_dat(0x15);
  		HX8363B_wr_dat(0x17);
  		HX8363B_wr_dat(0xD5);
  		HX8363B_wr_dat(0x56);
  		HX8363B_wr_dat(0x0E);
  		HX8363B_wr_dat(0x15);
		_delay_ms(5);
		
  		HX8363B_wr_cmd(0x3A);		/* set pixel format */
  		HX8363B_wr_dat(0x55);		/* 16bit */
		
   		HX8363B_wr_cmd(0x11);
		_delay_ms(120); 
		
   		HX8363B_wr_cmd(0x29);
	}

	else { for(;;);} /* Invalid Device Code!! */

	HX8363B_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8363B_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8363B_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
