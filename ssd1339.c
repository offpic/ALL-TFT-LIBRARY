/********************************************************************************/
/*!
	@file			ssd1339.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive UG-2828GFEFF01 OLED module(8bit,spi mode).

    @section HISTORY
		2011.10.31	V1.00	Renewal from SED1339.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ssd1339.h"
/* check header file version for fool proof */
#if SSD1339_H != 0x0300
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
inline void SSD1339_reset(void)
{
#ifdef USE_SSD1339_OLED
	SSD1339_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	SSD1339_RD_SET();
	SSD1339_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	SSD1339_RES_CLR();							/* RES=L, CS=L   			*/
	SSD1339_CS_CLR();

#elif  USE_SSD1339_SPI_OLED
	SSD1339_RES_SET();							/* RES=H, CS=H				*/
	SSD1339_CS_SET();
	SSD1339_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	SSD1339_RES_CLR();							/* RES=L		   			*/
#endif

	_delay_ms(10);								/* wait 10ms     			*/
	
	SSD1339_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_SSD1339_OLED
/**************************************************************************/
/*! 
    Write OLED Command.
*/
/**************************************************************************/
inline void SSD1339_wr_cmd(uint8_t cmd)
{
	SSD1339_DC_CLR();							/* DC=L		     */
	
	SSD1339_CMD = cmd;							/* D7..D0=cmd    */
	SSD1339_WR();								/* WR=L->H       */
	
	SSD1339_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write OLED Data.
*/
/**************************************************************************/
inline void SSD1339_wr_dat(uint8_t dat)
{
	SSD1339_DATA = dat;							/* D7..D0=dat    */
	SSD1339_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write OLED GRAM.
*/
/**************************************************************************/
inline void SSD1339_wr_gram(uint16_t gram)
{
	SSD1339_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	SSD1339_WR();								/* WR=L->H					*/

	SSD1339_DATA = (uint8_t)gram;				/* lower 8bit data			*/
	SSD1339_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write OLED Block Data.
*/
/**************************************************************************/
inline void SSD1339_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		SSD1339_wr_dat (*p++);
		SSD1339_wr_dat (*p++);
		SSD1339_wr_dat (*p++);
		SSD1339_wr_dat (*p++);
	}
	while (n--) {
		SSD1339_wr_dat (*p++);
	}
#endif

}

#elif USE_SSD1339_SPI_OLED
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SSD1339_wr_cmd(uint8_t cmd)
{
	SSD1339_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	SSD1339_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void SSD1339_wr_dat(uint8_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void SSD1339_wr_gram(uint16_t gram)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI16(gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SSD1339_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt );
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
#endif

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SSD1339_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	SSD1339_wr_cmd(0x15);
	SSD1339_wr_dat(OFS_COL + x);
	SSD1339_wr_dat(OFS_COL + width);

	/* Set RAS Address */
	SSD1339_wr_cmd(0x75);
	SSD1339_wr_dat(OFS_RAW + y);
	SSD1339_wr_dat(OFS_RAW + height); 

	/* Write RAM */
	SSD1339_wr_cmd(0x5C);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SSD1339_clear(void)
{
	volatile uint32_t n;

	SSD1339_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
	    /* 16Bit Colour Access */
		SSD1339_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    OLED Module Initialize.
*/
/**************************************************************************/
void SSD1339_init(void)
{
	Display_IoInit_If();
	
	SSD1339_reset();
	
	/* SSD1339 */
	SSD1339_wr_cmd(0xA0);	/* Set Re-map / Color Depth */
	SSD1339_wr_dat(0x74);	/* 64K color, COM split, COM remap, 8bitBus, Color remap, 
								Non column address remap, Hotizontal increment */
	SSD1339_wr_cmd(0xA1);	/* Set display start line */
	SSD1339_wr_dat(0x00);	/* 00h start */

	SSD1339_wr_cmd(0xA2);	/* Set display offset */
	SSD1339_wr_dat(0x80);	/* 80h start */

	SSD1339_wr_cmd(0xA6);	/* Normal display */

	SSD1339_wr_cmd(0xAD);	/* Set Master Configuration */
	SSD1339_wr_dat(0x8E);	/* DC-DC off & external VcomH voltage & external pre-charge voltage */

	SSD1339_wr_cmd(0xB0);	/* Power saving mode */
	SSD1339_wr_dat(0x05);

	SSD1339_wr_cmd(0xB1);	/* Set pre & dis_charge */
	SSD1339_wr_dat(0x11);	/* pre=1h dis=1h */

	SSD1339_wr_cmd(0xB3);	/* clock & frequency */
 	SSD1339_wr_dat(0x61);	/* clock=Divser+2 frequency=6 */

	SSD1339_wr_cmd(0xBB);	/* Set pre-charge voltage of color A B C */
	SSD1339_wr_dat(0x1C);	/* color A */
	SSD1339_wr_dat(0x1C);	/* color B */
	SSD1339_wr_dat(0x1C);	/* color C */

	SSD1339_wr_cmd(0xBE);	/* Set VcomH */
	SSD1339_wr_dat(0x1F);	

	SSD1339_wr_cmd(0xC1);	/* Set contrast current for A B C */
	SSD1339_wr_dat(0xAA);	/* Color A */
	SSD1339_wr_dat(0xB4);	/* Color B */ 
	SSD1339_wr_dat(0xC8);	/* Color C */

	SSD1339_wr_cmd(0xC7);	/* Set master contrast */
	SSD1339_wr_dat(0x0F);	/* no change */

	SSD1339_wr_cmd(0xCA);	/* Duty */ 
	SSD1339_wr_dat(0x7F);	/* 128 */

	SSD1339_clear();

	SSD1339_wr_cmd(0xAF);	/* Display on */
	_delay_ms(10);

#if 0	/* test code RED */
	volatile uint32_t n;

	SSD1339_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1339_wr_gram(COL_RED);
	} while (--n);

	_delay_ms(500);
	for(;;);

#endif

}


/* End Of File ---------------------------------------------------------------*/
