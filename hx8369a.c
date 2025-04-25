/********************************************************************************/
/*!
	@file			hx8369a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -KD043FM-1			(HX8369A)	8/16bit mode.

    @section HISTORY
		2014.05.01	V1.00	First Release.
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8369a.h"
/* check header file version for fool proof */
#if HX8369A_H != 0x0400
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
inline void HX8369A_reset(void)
{
	HX8369A_RES_SET();						/* RES=H, RD=H, WR=H   		*/
	HX8369A_RD_SET();
	HX8369A_WR_SET();
	_delay_ms(10);							/* wait 10ms     			*/

	HX8369A_RES_CLR();						/* RES=L, CS=L   			*/
	HX8369A_CS_CLR();

	_delay_ms(10);							/* wait 10ms     			*/
	HX8369A_RES_SET();						/* RES=H					*/
	_delay_ms(125);				    		/* wait over 120ms     		*/
}


/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8369A_wr_cmd(uint8_t cmd)
{
	HX8369A_DC_CLR();						/* DC=L						*/

	HX8369A_CMD = cmd;						/* cmd(8bit_Low or 16bit)	*/
	HX8369A_WR();							/* WR=L->H					*/

	HX8369A_DC_SET();						/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void HX8369A_wr_dat(uint8_t dat)
{
	HX8369A_DATA = dat;						/* data(8bit_Low or 16bit)	*/
	HX8369A_WR();							/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8369A_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8369A_DATA = (uint8_t)(gram>>8);		/* upper 8bit data			*/
	HX8369A_WR();							/* WR=L->H					*/
	HX8369A_DATA = (uint8_t)gram;			/* lower 8bit data			*/
#else
	HX8369A_DATA = gram;					/* 16bit data				*/
#endif
	HX8369A_WR();							/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8369A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8369A_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		HX8369A_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
uint16_t HX8369A_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	
 	HX8369A_wr_cmd(0xB9);
	HX8369A_wr_dat(0xFF); 
	HX8369A_wr_dat(0x83); 
	HX8369A_wr_dat(0x69);

	HX8369A_wr_cmd(cmd);
	HX8369A_WR_SET();
    ReadLCDData(val);							/* Dummy */
    ReadLCDData(val);							/* Read Data */

	val &= 0x00FF;

	return val;
}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8369A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8369A_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	HX8369A_wr_dat((OFS_COL + x)>>8);
	HX8369A_wr_dat(OFS_COL + x);
	HX8369A_wr_dat((OFS_COL + width)>>8);
	HX8369A_wr_dat(OFS_COL + width);

	HX8369A_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	HX8369A_wr_dat((OFS_RAW + y)>>8);
	HX8369A_wr_dat(OFS_RAW + y);
	HX8369A_wr_dat((OFS_RAW + height)>>8);
	HX8369A_wr_dat(OFS_RAW + height);

	HX8369A_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8369A_clear(void)
{
	volatile uint32_t n;

	HX8369A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8369A_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8369A_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	HX8369A_reset();

	/* Check Device Code */
	devicetype = HX8369A_rd_cmd(0xF4);		/* Confirm Vaild LCD Controller */

	if(devicetype  == 0x69)
	{
		/* Initialize HX8369A */
 		HX8369A_wr_cmd(0xB9);
		HX8369A_wr_dat(0xFF); 
		HX8369A_wr_dat(0x83); 
		HX8369A_wr_dat(0x69); 
		
 		HX8369A_wr_cmd(0xB1);	/* Set Power */
		HX8369A_wr_dat(0x85);
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x34);
		HX8369A_wr_dat(0x06);
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x10);
		HX8369A_wr_dat(0x10);
		HX8369A_wr_dat(0x2F);
		HX8369A_wr_dat(0x3A);
		HX8369A_wr_dat(0x3F);
		HX8369A_wr_dat(0x3F);
		HX8369A_wr_dat(0x07);
		HX8369A_wr_dat(0x23);
		HX8369A_wr_dat(0x01);
		HX8369A_wr_dat(0xE6);
		HX8369A_wr_dat(0xE6);
		HX8369A_wr_dat(0xE6);
		HX8369A_wr_dat(0xE6);
		HX8369A_wr_dat(0xE6);
		
 		HX8369A_wr_cmd(0xB2);	/* SET Display  480x800 */
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x20);	/* 2B FOR rgb Interface */
		HX8369A_wr_dat(0x03);
		HX8369A_wr_dat(0x03);
		HX8369A_wr_dat(0x70);
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0xFF);
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x03);
		HX8369A_wr_dat(0x03);
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x01);
      
 		HX8369A_wr_cmd(0xB4);	/* SET Display  480x800 */
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x0C);
		HX8369A_wr_dat(0xA0);
		HX8369A_wr_dat(0x0E);
		HX8369A_wr_dat(0x06);
		
 		HX8369A_wr_cmd(0xB6);	/* SET VCOM */
		HX8369A_wr_dat(0x35);
		HX8369A_wr_dat(0x35);
		
 		HX8369A_wr_cmd(0xD5);
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x05);
		HX8369A_wr_dat(0x03);
		HX8369A_wr_dat(0x00);
		HX8369A_wr_dat(0x01);
		HX8369A_wr_dat(0x09);
		HX8369A_wr_dat(0x10);
		HX8369A_wr_dat(0x80);
		HX8369A_wr_dat(0x37);
		HX8369A_wr_dat(0x37);
		HX8369A_wr_dat(0x20);
		HX8369A_wr_dat(0x31);
		HX8369A_wr_dat(0x46);
		HX8369A_wr_dat(0x8A);
		HX8369A_wr_dat(0x57);
		HX8369A_wr_dat(0x9B);
		HX8369A_wr_dat(0x20);
		HX8369A_wr_dat(0x31);
		HX8369A_wr_dat(0x46);
		HX8369A_wr_dat(0x8A);
		HX8369A_wr_dat(0x57);
		HX8369A_wr_dat(0x9B);
		HX8369A_wr_dat(0x07);
		HX8369A_wr_dat(0x0F);
		HX8369A_wr_dat(0x02);
		HX8369A_wr_dat(0x00);
		
		/* Gamma2.2 */ 
 		HX8369A_wr_cmd(0xE0);
		HX8369A_wr_dat(0x01);
		HX8369A_wr_dat(0x08);
		HX8369A_wr_dat(0x0D);
		HX8369A_wr_dat(0x2D);
		HX8369A_wr_dat(0x34);
		HX8369A_wr_dat(0x37);
		HX8369A_wr_dat(0x21);
		HX8369A_wr_dat(0x3C);
		HX8369A_wr_dat(0x0E);
		HX8369A_wr_dat(0x10);
		HX8369A_wr_dat(0x0E);
		HX8369A_wr_dat(0x12);
		HX8369A_wr_dat(0x14);
		HX8369A_wr_dat(0x12);
		HX8369A_wr_dat(0x14);
		HX8369A_wr_dat(0x13);
		HX8369A_wr_dat(0x19);
		HX8369A_wr_dat(0x01);
		HX8369A_wr_dat(0x08);
		HX8369A_wr_dat(0x0D);
		HX8369A_wr_dat(0x2D);
		HX8369A_wr_dat(0x34);
		HX8369A_wr_dat(0x37);
		HX8369A_wr_dat(0x21);
		HX8369A_wr_dat(0x3C);
		HX8369A_wr_dat(0x0E);
		HX8369A_wr_dat(0x10);
		HX8369A_wr_dat(0x0E);
		HX8369A_wr_dat(0x12);
		HX8369A_wr_dat(0x14);
		HX8369A_wr_dat(0x12);
		HX8369A_wr_dat(0x14);
		HX8369A_wr_dat(0x13);
		HX8369A_wr_dat(0x19);
		_delay_ms(10); 
		
 		HX8369A_wr_cmd(0x2d);
		for(int k=0;k<64;k++)		/* RED */
			{ HX8369A_wr_dat(k*8); }		
		for(int k=0;k<64;k++) 		/* GREEN */
			{ HX8369A_wr_dat(k*4); }
		for(int k=0;k<64;k++)		/* BLUE */
			{ HX8369A_wr_dat(k*8); }
			
  		HX8369A_wr_cmd(0x3A);		/* set pixel format */
  		HX8369A_wr_dat(0x55);		/* 16bit */
		
   		HX8369A_wr_cmd(0x11);
		_delay_ms(120); 
		
   		HX8369A_wr_cmd(0x29);
	}

	else { for(;;);} /* Invalid Device Code!! */

	HX8369A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8369A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8369A_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
