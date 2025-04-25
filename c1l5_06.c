/********************************************************************************/
/*!
	@file			c1l5_06.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive CS1802		TFT module(8bit mode only).			@n

    @section HISTORY
		2012.01.21	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "c1l5_06.h"
/* check header file version for fool proof */
#if C1L5_06_H != 0x0300
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
inline void C1L5_06_reset(void)
{
	C1L5_06_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	C1L5_06_RD_SET();
	C1L5_06_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	C1L5_06_RES_CLR();							/* RES=L, CS=L   			*/
	C1L5_06_CS_CLR();
	_delay_ms(5);								/* wait 1ms at least     	*/
	
	C1L5_06_RES_SET();						  	/* RES=H					*/
	_delay_ms(10);				    			/* wait 10ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void C1L5_06_wr_cmd(uint8_t cmd)
{
	C1L5_06_DC_CLR();							/* DC=L							*/

	C1L5_06_CMD = cmd;							/* command(8bit_Low or 16bit)	*/
	C1L5_06_WR();								/* WR=L->H						*/

	C1L5_06_DC_SET();							/* DC=H							*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void C1L5_06_wr_dat(uint8_t dat)
{
	C1L5_06_DATA = dat;							/* lower 8bit data			*/
	C1L5_06_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void C1L5_06_wr_gram(uint16_t dat)
{
	C1L5_06_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	C1L5_06_WR();								/* WR=L->H					*/
	
	C1L5_06_DATA = (uint8_t)dat;				/* lower 8bit data			*/
	C1L5_06_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void C1L5_06_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		C1L5_06_wr_dat(*p++);
		C1L5_06_wr_dat(*p++);
		C1L5_06_wr_dat(*p++);
		C1L5_06_wr_dat(*p++);
	}
	while (n--) {
		C1L5_06_wr_dat(*p++);
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void C1L5_06_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	C1L5_06_wr_cmd(0x09);				/* GRAM Horizontal ADDR Start Set */
	C1L5_06_wr_dat(OFS_COL + x);
	C1L5_06_wr_cmd(0x10);				/* GRAM Vertical ADDR Start Set */
	C1L5_06_wr_dat(OFS_RAW + y);

	C1L5_06_wr_cmd(0x11);				/* GRAM Horizontal ADDR End Set */
	C1L5_06_wr_dat(OFS_COL + width);
	C1L5_06_wr_cmd(0x12);				/* GRAM Vertical ADDR End Set */
	C1L5_06_wr_dat(OFS_RAW + height);
	
	C1L5_06_wr_cmd(0x18);				/* GRAM Vertical/Horizontal ADDR Set */
	C1L5_06_wr_dat(OFS_COL + x);
	C1L5_06_wr_cmd(0x19);				/* GRAM Vertical/Horizontal ADDR Set */
	C1L5_06_wr_dat(OFS_RAW + y);

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void C1L5_06_clear(void)
{
	volatile uint32_t n;

	C1L5_06_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		C1L5_06_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t C1L5_06_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	C1L5_06_wr_cmd(cmd);
	C1L5_06_WR_SET();

    ReadLCDData(temp);
    ReadLCDData(val);

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void C1L5_06_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	C1L5_06_reset();

	/* Check Device Code */
	devicetype = C1L5_06_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0800)
	{
		/* Initialize C1L5_06 */
		C1L5_06_wr_cmd(0x01);
		C1L5_06_wr_dat(0x02);   /* Mode_SEL1 */
		C1L5_06_wr_cmd(0x02);
		C1L5_06_wr_dat(0x12);   /* Mode_SEL2 */
		C1L5_06_wr_cmd(0x03);
		C1L5_06_wr_dat(0x20);   /* Mode_SEL3 */
		C1L5_06_wr_cmd(0x05);
		C1L5_06_wr_dat(0x08);   /* VCO_Mode */
		C1L5_06_wr_cmd(0x07);
		C1L5_06_wr_dat(0x7F);   /* VCOMH_CTRL */
		C1L5_06_wr_cmd(0x08);
		C1L5_06_wr_dat(0x17);   /* VCOML_CTRL */
		C1L5_06_wr_cmd(0x17);
		C1L5_06_wr_dat(0x00);   /* SRAM Control */
		C1L5_06_wr_cmd(0x09);
		C1L5_06_wr_dat(0x00);   /* SRAM CONTROL:PWS-X */
		C1L5_06_wr_cmd(0x10);
		C1L5_06_wr_dat(0x00);   /* SRAM CONTROL:PWS-Y */
		C1L5_06_wr_cmd(0x11);
		C1L5_06_wr_dat(0x7F);   /* SRAM CONTROL:PWE-X */
		C1L5_06_wr_cmd(0x12);
		C1L5_06_wr_dat(0x9F);   /* SRAM CONTROL:PWE-Y */
		C1L5_06_wr_cmd(0x14);
		C1L5_06_wr_dat(0x00);   /* SRAM CONTROL:PDS-X */
		C1L5_06_wr_cmd(0x16);
		C1L5_06_wr_dat(0x9F);   /* SRAM CONTROL:PDS-Y */
		C1L5_06_wr_cmd(0x18);
		C1L5_06_wr_dat(0x00);   /* SRAM_POSITION_X */
		C1L5_06_wr_cmd(0x19);
		C1L5_06_wr_dat(0x00);   /* SRAM_POSITION_Y */
		C1L5_06_wr_cmd(0x21);
		C1L5_06_wr_dat(0x88);   /* Gamma control 1 */
		C1L5_06_wr_cmd(0x22);
		C1L5_06_wr_dat(0x33);   /* Gamma control 2 */
		C1L5_06_wr_cmd(0x23);
		C1L5_06_wr_dat(0xCC);   /* Gamma control 3 */
		C1L5_06_wr_cmd(0x24);
		C1L5_06_wr_dat(0xCC);   /* Gamma control 4 */
		C1L5_06_wr_cmd(0x25);
		C1L5_06_wr_dat(0xBB);   /* Gamma control 5 */
		C1L5_06_wr_cmd(0x26);
		C1L5_06_wr_dat(0xBB);   /* Gamma control 6 */
		C1L5_06_wr_cmd(0x27);
		C1L5_06_wr_dat(0x77);   /* Gamma control 7 */
		C1L5_06_wr_cmd(0x28);
		C1L5_06_wr_dat(0x77);   /* Gamma control 8 */
		C1L5_06_wr_cmd(0x29);
		C1L5_06_wr_dat(0x77);   /* Gamma control 9 */
		C1L5_06_wr_cmd(0x06);
		C1L5_06_wr_dat(0xC7);   /* DAC_OP_CTRL2 */
	}

	else { for(;;);} /* Invalid Device Code!! */

	C1L5_06_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	C1L5_06_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		C1L5_06_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
