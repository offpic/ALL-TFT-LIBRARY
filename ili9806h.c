/********************************************************************************/
/*!
	@file			ili9806h.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TK040FH001		(ILI9806H)	8/16bit mode.

    @section HISTORY
		2015.10.20	V1.00	First Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili9806h.h"
/* check header file version for fool proof */
#if ILI9806H_H != 0x0300
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#warning "S**kly,This TFT controller has HALF-RAM Structure ! \
So You CANNOT set odd X/Y-Axis start address! \
So You CANNOT use several GFX sub Functions....! Too F**K! "

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Display Module Reset Routine.
*/
/**************************************************************************/
inline void ILI9806H_reset(void)
{
	ILI9806H_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI9806H_RD_SET();
	ILI9806H_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9806H_RES_CLR();							/* RES=L, CS=L   			*/
	ILI9806H_CS_CLR();

	_delay_ms(10);								/* wait 10ms     			*/
	ILI9806H_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait over 120ms     		*/
}


/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9806H_wr_cmd(uint8_t cmd)
{
	ILI9806H_DC_CLR();							/* DC=L						*/

	ILI9806H_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	ILI9806H_WR();								/* WR=L->H					*/

	ILI9806H_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ILI9806H_wr_dat(uint8_t dat)
{
	ILI9806H_DATA = dat;						/* data(8bit_Low or 16bit)	*/
	ILI9806H_WR();							/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ILI9806H_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9806H_DATA = (uint8_t)(gram>>8);		/* upper 8bit data		*/
	ILI9806H_WR();							/* WR=L->H				*/
	ILI9806H_DATA = (uint8_t)gram;			/* lower 8bit data		*/
#else
	ILI9806H_DATA = gram;					/* 16bit data			*/
#endif
	ILI9806H_WR();							/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9806H_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI9806H_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		ILI9806H_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
uint16_t ILI9806H_rd_cmd(uint8_t cmd)
{
	uint8_t temp,i;
	uint16_t val;

 	ILI9806H_wr_cmd(0xFF);
	ILI9806H_wr_dat(0xFF);
	ILI9806H_wr_dat(0x98);
	ILI9806H_wr_dat(0x26);

	ILI9806H_wr_cmd(cmd);
	ILI9806H_WR_SET();

	for(i=0;i<3;i++){
		ReadLCDData(temp);
	}

    ReadLCDData(val);

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ILI9806H_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ILI9806H_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	ILI9806H_wr_dat((OFS_COL + x)>>8);
	ILI9806H_wr_dat(OFS_COL + x);
	ILI9806H_wr_dat((OFS_COL + width)>>8);
	ILI9806H_wr_dat(OFS_COL + width);

	ILI9806H_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	ILI9806H_wr_dat((OFS_RAW + y)>>8);
	ILI9806H_wr_dat(OFS_RAW + y);
	ILI9806H_wr_dat((OFS_RAW + height)>>8);
	ILI9806H_wr_dat(OFS_RAW + height);

	ILI9806H_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI9806H_clear(void)
{
	volatile uint32_t n;

	ILI9806H_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9806H_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ILI9806H_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	ILI9806H_reset();

	/* Check Device Code */
	devicetype = ILI9806H_rd_cmd(0xD3);		/* Confirm Vaild LCD Controller */

	if(devicetype  == 0x9826)
	{
		/* Initialize ILI9806H */
		ILI9806H_wr_cmd(0xFF);		/* EXTC Command Set enable register */ 
		ILI9806H_wr_dat(0xFF);
		ILI9806H_wr_dat(0x98);
		ILI9806H_wr_dat(0x26);

 		ILI9806H_wr_cmd(0xBC);		/* GIP 1 */ 
		ILI9806H_wr_dat(0x21);
		ILI9806H_wr_dat(0x06);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x01);
		ILI9806H_wr_dat(0x01);
		ILI9806H_wr_dat(0x80);
		ILI9806H_wr_dat(0x02);
		ILI9806H_wr_dat(0x05);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x01);
		ILI9806H_wr_dat(0x01);
		ILI9806H_wr_dat(0xF0);
		ILI9806H_wr_dat(0xF4);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0xC0);
		ILI9806H_wr_dat(0x08);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x00);

 		ILI9806H_wr_cmd(0xBD);		/* GIP 2 */
		ILI9806H_wr_dat(0x02);
		ILI9806H_wr_dat(0x13);
		ILI9806H_wr_dat(0x45);
		ILI9806H_wr_dat(0x67);
		ILI9806H_wr_dat(0x01);
		ILI9806H_wr_dat(0x23);
		ILI9806H_wr_dat(0x45);
		ILI9806H_wr_dat(0x67);

 		ILI9806H_wr_cmd(0xBE);		/* GIP 3 */
		ILI9806H_wr_dat(0x13);
		ILI9806H_wr_dat(0x22);
		ILI9806H_wr_dat(0x22);
		ILI9806H_wr_dat(0x22);
		ILI9806H_wr_dat(0x22);
		ILI9806H_wr_dat(0xBB);
		ILI9806H_wr_dat(0xAA);
		ILI9806H_wr_dat(0xDD);
		ILI9806H_wr_dat(0xCC);
		ILI9806H_wr_dat(0x66);
		ILI9806H_wr_dat(0x77);
		ILI9806H_wr_dat(0x22);
		ILI9806H_wr_dat(0x22);
		ILI9806H_wr_dat(0x22);
		ILI9806H_wr_dat(0x22);
		ILI9806H_wr_dat(0x22);
		ILI9806H_wr_dat(0x22);

 		ILI9806H_wr_cmd(0xFA);
		ILI9806H_wr_dat(0x08);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x02);
		ILI9806H_wr_dat(0x08);

 		ILI9806H_wr_cmd(0xB1);		/* FRAME RATE 60.18Hz (TE ON) */
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x58);
		ILI9806H_wr_dat(0x03);

 		ILI9806H_wr_cmd(0xB4);		/* 2 dot */
		ILI9806H_wr_dat(0x02);
		ILI9806H_wr_dat(0x02);
		ILI9806H_wr_dat(0x02);

 		ILI9806H_wr_cmd(0xC1);		/* AVDD=2xVCI=5.06, AVEE=-2xVCI=-4.82 */
		ILI9806H_wr_dat(0x15);
		ILI9806H_wr_dat(0x78);
		ILI9806H_wr_dat(0x6A);

 		ILI9806H_wr_cmd(0xC7);		/* VCOM=-1.337 */	
		ILI9806H_wr_dat(0x2F);

 		ILI9806H_wr_cmd(0xED);
		ILI9806H_wr_dat(0x7F);
		ILI9806H_wr_dat(0x0F);

 		ILI9806H_wr_cmd(0x35);		/* TE ON */
		ILI9806H_wr_dat(0x00);

 		ILI9806H_wr_cmd(0xF7);		/* 480x800 */
		ILI9806H_wr_dat(0x02);

 		ILI9806H_wr_cmd(0xF2);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x07);
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x8E);
		ILI9806H_wr_dat(0x0A);
		ILI9806H_wr_dat(0x0A);

 		ILI9806H_wr_cmd(0xE0);		/* Positive Gamma Control */
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x03);
		ILI9806H_wr_dat(0x0B);
		ILI9806H_wr_dat(0x0F);
		ILI9806H_wr_dat(0x12);
		ILI9806H_wr_dat(0x17);
		ILI9806H_wr_dat(0xCB);
		ILI9806H_wr_dat(0x0B);
		ILI9806H_wr_dat(0x02);
		ILI9806H_wr_dat(0x07);
		ILI9806H_wr_dat(0x06);
		ILI9806H_wr_dat(0x0B);
		ILI9806H_wr_dat(0x0A);
		ILI9806H_wr_dat(0x34);
		ILI9806H_wr_dat(0x30);
		ILI9806H_wr_dat(0x00);

 		ILI9806H_wr_cmd(0xE1);		/* Negative Gamma Control */ 
		ILI9806H_wr_dat(0x00);
		ILI9806H_wr_dat(0x03);
		ILI9806H_wr_dat(0x0A);
		ILI9806H_wr_dat(0x0E);
		ILI9806H_wr_dat(0x12);
		ILI9806H_wr_dat(0x16);
		ILI9806H_wr_dat(0x7A);
		ILI9806H_wr_dat(0x07);
		ILI9806H_wr_dat(0x03);
		ILI9806H_wr_dat(0x07);
		ILI9806H_wr_dat(0x07);
		ILI9806H_wr_dat(0x0B);
		ILI9806H_wr_dat(0x0C);
		ILI9806H_wr_dat(0x27);
		ILI9806H_wr_dat(0x22);
		ILI9806H_wr_dat(0x00);

 		ILI9806H_wr_cmd(0x3A);
		ILI9806H_wr_dat(0x55);

 		ILI9806H_wr_cmd(0x36); 
		ILI9806H_wr_dat(0x00);

 		ILI9806H_wr_cmd(0x11);		/* Exit Sleep */
		_delay_ms(120);
 		ILI9806H_wr_cmd(0x29);		/* Display On */
		_delay_ms(10);

 		ILI9806H_wr_cmd(0x11);		/* Exit Sleep */
		_delay_ms(120);
 		ILI9806H_wr_cmd(0x29);		/* Display On */
		_delay_ms(10);

	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI9806H_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ILI9806H_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9806H_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
