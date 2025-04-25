/********************************************************************************/
/*!
	@file			ili9327.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -DST9901A-NH				(ILI9327)	8/16bit mode.		@n
					 -S95361A					(ILI9327)   8/16bit mode.       @n

    @section HISTORY
		2010.10.01	V1.00	Stable Release
		2010.12.31	V2.00	Cleanup SourceCode.
		2011.03.10	V3.00	C++ Ready.
		2011.10.25	V4.00	Added DMA TransactionSupport.
		2023.05.01	V5.00	Removed unused delay function.
		2023.08.01	V6.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili9327.h"
/* check header file version for fool proof */
#if ILI9327_H != 0x0600
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
inline void ILI9327_reset(void)
{
	ILI9327_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI9327_RD_SET();
	ILI9327_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	ILI9327_RES_CLR();							/* RES=L, CS=L   			*/
	ILI9327_CS_CLR();
	_delay_ms(10);								/* wait 10ms     			*/
	
	ILI9327_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9327_wr_cmd(uint8_t cmd)
{
	ILI9327_DC_CLR();							/* DC=L						*/

	ILI9327_CMD = cmd;							/* cmd(8bit)				*/
	ILI9327_WR();								/* WR=L->H					*/

	ILI9327_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void ILI9327_wr_dat(uint8_t dat)
{
	ILI9327_DATA = dat;							/* data(8bit)				*/
	ILI9327_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ILI9327_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9327_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	ILI9327_WR();								/* WR=L->H					*/
	ILI9327_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	ILI9327_DATA = gram;						/* 16bit data 				*/
#endif
	ILI9327_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9327_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI9327_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		ILI9327_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif
	
}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ILI9327_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ILI9327_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	ILI9327_wr_dat((OFS_COL + x)>>8);
	ILI9327_wr_dat(OFS_COL + x);
	ILI9327_wr_dat((OFS_COL + width)>>8);
	ILI9327_wr_dat(OFS_COL + width);

	ILI9327_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	ILI9327_wr_dat((OFS_RAW + y)>>8);
	ILI9327_wr_dat(OFS_RAW + y);
	ILI9327_wr_dat((OFS_RAW + height)>>8);
	ILI9327_wr_dat(OFS_RAW + height);

	ILI9327_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI9327_clear(void)
{
	volatile uint32_t n;

	ILI9327_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9327_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI9327_rd_cmd(uint16_t cmd)
{
	uint8_t temp,i;
	uint16_t val;


	ILI9327_wr_cmd(cmd);
	ILI9327_WR_SET();

	for(i=0;i<4;i++){
		ReadLCDData(temp);
	}

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
void ILI9327_init(void)
{
	volatile uint16_t devicetype;
	
	Display_IoInit_If();

	ILI9327_reset();

	/* Check Device Code */
	devicetype = ILI9327_rd_cmd(0xEF);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x9327)
	{
		/* Initialize ILI9327 */
		ILI9327_wr_cmd(0xE9);
		ILI9327_wr_dat(0x20);
		
		ILI9327_wr_cmd(0x11); /* Exit Sleep */
		_delay_ms(100);
		
		ILI9327_wr_cmd(0xD1);
		ILI9327_wr_dat(0x00);
		ILI9327_wr_dat(0x71);
		ILI9327_wr_dat(0x19);
		
		ILI9327_wr_cmd(0xD0);
		ILI9327_wr_dat(0x07);
		ILI9327_wr_dat(0x01);
		ILI9327_wr_dat(0x08);
		
		ILI9327_wr_cmd(0x36);
		ILI9327_wr_dat(0x48);
		
		ILI9327_wr_cmd(0x3A);
		ILI9327_wr_dat(0x55);
		
		ILI9327_wr_cmd(0xC1);
		ILI9327_wr_dat(0x10);
		ILI9327_wr_dat(0x10);
		ILI9327_wr_dat(0x02);
		ILI9327_wr_dat(0x02);
		
		ILI9327_wr_cmd(0xC0); /* Set Default Gamma */
		ILI9327_wr_dat(0x00);
		ILI9327_wr_dat(0x35);
		ILI9327_wr_dat(0x00);
		ILI9327_wr_dat(0x00);
		ILI9327_wr_dat(0x01);
		ILI9327_wr_dat(0x02);
		
		ILI9327_wr_cmd(0xC5); /* Set frame rate */
		ILI9327_wr_dat(0x04);
		
		ILI9327_wr_cmd(0xD2); /* power setting */
		ILI9327_wr_dat(0x01);
		ILI9327_wr_dat(0x44);
		
		ILI9327_wr_cmd(0xC8); /* Set Gamma */
		ILI9327_wr_dat(0x04);
		ILI9327_wr_dat(0x67);
		ILI9327_wr_dat(0x35);
		ILI9327_wr_dat(0x04);
		ILI9327_wr_dat(0x08);
		ILI9327_wr_dat(0x06);
		ILI9327_wr_dat(0x24);
		ILI9327_wr_dat(0x01);
		ILI9327_wr_dat(0x37);
		ILI9327_wr_dat(0x40);
		ILI9327_wr_dat(0x03);
		ILI9327_wr_dat(0x10);
		ILI9327_wr_dat(0x08);
		ILI9327_wr_dat(0x80);
		ILI9327_wr_dat(0x00);
		
		ILI9327_wr_cmd(0x29); 				/* display on */ 
	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI9327_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ILI9327_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9327_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
