/********************************************************************************/
/*!
	@file			ssd1332.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        9.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive ALO-095BWNN-J9 OLED module(8bit,spi mode).

    @section HISTORY
		2010.06.01	V1.00	Unstable Release.
		2010.10.01	V2.00	Changed CTRL-Port Contol Procedure.
		2010.12.31	V3.00	Added GRAM write function.
		2011.03.10	V4.00	C++ Ready.
		2012.02.15	V5.00	Add Foolest WorkAround In Write GGRAM Access.
		2015.02.14	V6.00	Fixed Foolest WorkAround In Write GGRAM Access.
		2023.03.01	V7.00	Removed Foolest WorkAround.
		2023.05.01	V8.00	Removed unused delay function.
		2023.08.01	V9.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ssd1332.h"
/* check header file version for fool proof */
#if SSD1332_H != 0x0900
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Abstract Layer Delay Settings.
*/
/**************************************************************************/
void SSD1332_reset(void)
{
	SSD1332_RES_SET();							/* RES=H, CS=H				*/
	SSD1332_CS_SET();
	_delay_ms(5);								/* wait 5ms     			*/

	SSD1332_RES_CLR();							/* RES=L		   			*/
	_delay_ms(80);								/* wait 80ms     			*/
	
	SSD1332_RES_SET();						  	/* RES=H		   			*/
	_delay_ms(20);				    			/* wait 20ms     			*/
}

/**************************************************************************/
/*! 
    Write OLED Command.
*/
/**************************************************************************/
inline void SSD1332_wr_cmd(uint8_t cmd)
{
	SSD1332_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	SSD1332_DC_SET();							/* DC=H		     */
}

/**************************************************************************/
/*! 
    Write OLED Data.
*/
/**************************************************************************/
inline void SSD1332_wr_dat(uint8_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write OLED GRAM.
*/
/**************************************************************************/
inline void SSD1332_wr_gram(uint16_t gram)
{	
	DISPLAY_ASSART_CS();						/* CS=L		    		*/
	
	SendSPI16(gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     		*/
}

/**************************************************************************/
/*! 
    Write OLED Block Data.
*/
/**************************************************************************/
inline void SSD1332_wr_block(uint8_t *p,unsigned int cnt)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		SSD1332_wr_dat (*p++);
		SSD1332_wr_dat (*p++);
		SSD1332_wr_dat (*p++);
		SSD1332_wr_dat (*p++);
	}
	while (n--) {
		SSD1332_wr_dat (*p++);
	}
#endif

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SSD1332_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	SSD1332_wr_cmd(0x15);
	SSD1332_wr_cmd(OFS_COL + x);
	SSD1332_wr_cmd(OFS_COL + width);

	/* Set RAS Address */
	SSD1332_wr_cmd(0x75);
	SSD1332_wr_cmd(OFS_RAW + y);
	SSD1332_wr_cmd(OFS_RAW + height); 
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SSD1332_clear(void)
{
	volatile uint32_t n;

	SSD1332_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1332_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    OLED Module Initialize.
*/
/**************************************************************************/
void SSD1332_init(void)
{
	Display_IoInit_If();
	
	SSD1332_reset();
	
	/* SSD1332 init */
	SSD1332_wr_cmd(0xAE); 			/* Display OFF */
	SSD1332_wr_cmd(0xA0); 			/* Set remap & data format */
	SSD1332_wr_cmd(0x70);
	SSD1332_wr_cmd(0xA1); 			/* Set display start row RAM */
	SSD1332_wr_cmd(0x00);
	SSD1332_wr_cmd(0xA2);			/* Set display offset */
	SSD1332_wr_cmd(0x00);
	SSD1332_wr_cmd(0xA4); 			/* Set Display Mode Nomal Display  */
	SSD1332_wr_cmd(0xA8); 			/* Set Multiplex Ratio */
	SSD1332_wr_cmd(0x3F);
	SSD1332_wr_cmd(0xAD); 			/* Set Master Configuration */
	SSD1332_wr_cmd(0x8E); 			/* (8F->8E External VCC Supply Selected) */
	SSD1332_wr_cmd(0xB0); 			/* Set Power Saving Mode */
	SSD1332_wr_cmd(0x12);
	SSD1332_wr_cmd(0xB1); 			/* Set Phase 1 & 2 Period Adjustment */
	SSD1332_wr_cmd(0x74);
	SSD1332_wr_cmd(0xB3); 			/* Set Display Clock Divide Ratio / Oscillator */
	SSD1332_wr_cmd(0xD0);
	SSD1332_wr_cmd(0x8A); 			/* Set Second Pre-charge Speed of Color A */
	SSD1332_wr_cmd(0x81);
	SSD1332_wr_cmd(0x8B); 			/* Set Second Pre-charge Speed of Color B */
	SSD1332_wr_cmd(0x82);
	SSD1332_wr_cmd(0x8C); 			/* Set Second Pre-charge Speed of Color C */
	SSD1332_wr_cmd(0x83);
	SSD1332_wr_cmd(0xBB); 			/* Set Pre-charge Level */
	SSD1332_wr_cmd(0x3E);
	SSD1332_wr_cmd(0xBE); 			/* Set VCOMH */
	SSD1332_wr_cmd(0x3E);
	SSD1332_wr_cmd(0x87); 			/* Set Master Current Control */
	SSD1332_wr_cmd(0x0F);
	SSD1332_wr_cmd(0x81); 			/* Set Contrast Control for Color ÅgAÅh*/
	SSD1332_wr_cmd(0x90);
	SSD1332_wr_cmd(0x82); 			/* Set Contrast Control for Color ÅgBÅh*/
	SSD1332_wr_cmd(0x80);
	SSD1332_wr_cmd(0x83); 			/* Set Contrast Control for Color ÅgCÅh*/
	SSD1332_wr_cmd(0x80);
	
	SSD1332_clear();

	SSD1332_wr_cmd(0xAF); 			/* Display ON */

#if 0	/* test code RED */
	volatile uint32_t n;

	SSD1332_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1332_wr_gram(COL_RED);
	} while (--n);
	_delay_ms(500);
	for(;;);

#endif

}


/* End Of File ---------------------------------------------------------------*/
