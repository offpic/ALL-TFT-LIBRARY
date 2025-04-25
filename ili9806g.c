/********************************************************************************/
/*!
	@file			ili9806g.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TK050F5590		(ILI9806G)	8/16bit mode.

    @section HISTORY
		2016.07.01	V1.00	First Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili9806g.h"
/* check header file version for fool proof */
#if ILI9806G_H != 0x0300
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
inline void ILI9806G_reset(void)
{
	ILI9806G_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI9806G_RD_SET();
	ILI9806G_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9806G_RES_CLR();							/* RES=L, CS=L   			*/
	ILI9806G_CS_CLR();

	_delay_ms(10);								/* wait 10ms     			*/
	ILI9806G_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait over 120ms     		*/
}


/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9806G_wr_cmd(uint8_t cmd)
{
	ILI9806G_DC_CLR();							/* DC=L						*/

	ILI9806G_CMD = cmd;							/* cmd(8bit_Low or 16bit)	*/
	ILI9806G_WR();								/* WR=L->H					*/

	ILI9806G_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ILI9806G_wr_dat(uint8_t dat)
{
	ILI9806G_DATA = dat;						/* data(8bit_Low or 16bit)	*/
	ILI9806G_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ILI9806G_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9806G_DATA = (uint8_t)(gram>>8);			/* upper 8bit data		*/
	ILI9806G_WR();								/* WR=L->H				*/
	ILI9806G_DATA = (uint8_t)gram;				/* lower 8bit data		*/
#else
	ILI9806G_DATA = gram;						/* 16bit data			*/
#endif
	ILI9806G_WR();								/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9806G_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI9806G_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		ILI9806G_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
uint16_t ILI9806G_rd_cmd(uint8_t cmd)
{
	uint8_t temp,i;
	uint16_t val;

 	ILI9806G_wr_cmd(0xFF);
	ILI9806G_wr_dat(0xFF);
	ILI9806G_wr_dat(0x98);
	ILI9806G_wr_dat(0x06);

	ILI9806G_wr_cmd(cmd);
	ILI9806G_WR_SET();

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
inline void ILI9806G_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ILI9806G_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	ILI9806G_wr_dat((OFS_COL + x)>>8);
	ILI9806G_wr_dat(OFS_COL + x);
	ILI9806G_wr_dat((OFS_COL + width)>>8);
	ILI9806G_wr_dat(OFS_COL + width);

	ILI9806G_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	ILI9806G_wr_dat((OFS_RAW + y)>>8);
	ILI9806G_wr_dat(OFS_RAW + y);
	ILI9806G_wr_dat((OFS_RAW + height)>>8);
	ILI9806G_wr_dat(OFS_RAW + height);

	ILI9806G_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI9806G_clear(void)
{
	volatile uint32_t n;

	ILI9806G_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9806G_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ILI9806G_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	ILI9806G_reset();

	/* Check Device Code */
	devicetype = ILI9806G_rd_cmd(0xD3);		/* Confirm Vaild LCD Controller */

	if(devicetype  == 0x9806)
	{
		/* Initialize ILI9806G */
		ILI9806G_wr_cmd(0xFF);		/* EXTC Command Set enable register */
		ILI9806G_wr_dat(0xFF);
		ILI9806G_wr_dat(0x98);
		ILI9806G_wr_dat(0x06);
		
		ILI9806G_wr_cmd(0xBA);		/* SPI Interface Setting */
		ILI9806G_wr_dat(0xE0);
		
		ILI9806G_wr_cmd(0xBC);		/* GIP 1 */
		ILI9806G_wr_dat(0x03);
		ILI9806G_wr_dat(0x0F);
		ILI9806G_wr_dat(0x63);
		ILI9806G_wr_dat(0x69);
		ILI9806G_wr_dat(0x01);
		ILI9806G_wr_dat(0x01);
		ILI9806G_wr_dat(0x1B);
		ILI9806G_wr_dat(0x11);
		ILI9806G_wr_dat(0x70);
		ILI9806G_wr_dat(0x73);
		ILI9806G_wr_dat(0xFF);
		ILI9806G_wr_dat(0xFF);
		ILI9806G_wr_dat(0x08);
		ILI9806G_wr_dat(0x09);
		ILI9806G_wr_dat(0x05);
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0xEE);
		ILI9806G_wr_dat(0xE2);
		ILI9806G_wr_dat(0x01);
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0xC1);
		
		ILI9806G_wr_cmd(0xBD);		/* GIP 2 */
		ILI9806G_wr_dat(0x01);
		ILI9806G_wr_dat(0x23);
		ILI9806G_wr_dat(0x45);
		ILI9806G_wr_dat(0x67);
		ILI9806G_wr_dat(0x01);
		ILI9806G_wr_dat(0x23);
		ILI9806G_wr_dat(0x45);
		ILI9806G_wr_dat(0x67);
		
		ILI9806G_wr_cmd(0xBE);		/* GIP 3 */
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0x22);
		ILI9806G_wr_dat(0x27);
		ILI9806G_wr_dat(0x6A);
		ILI9806G_wr_dat(0xBC);
		ILI9806G_wr_dat(0xD8);
		ILI9806G_wr_dat(0x92);
		ILI9806G_wr_dat(0x22);
		ILI9806G_wr_dat(0x22);
		
		ILI9806G_wr_cmd(0xC7);		/* Vcom */
		ILI9806G_wr_dat(0x1E);
		
		ILI9806G_wr_cmd(0xED);		/* EN_volt_reg */
		ILI9806G_wr_dat(0x7F); 
		ILI9806G_wr_dat(0x0F); 
		ILI9806G_wr_dat(0x00); 
		
		ILI9806G_wr_cmd(0xC0);		/* Power Control 1 */
		ILI9806G_wr_dat(0xE3);
		ILI9806G_wr_dat(0x0B);
		ILI9806G_wr_dat(0x00);
 
		ILI9806G_wr_cmd(0xFC);
		ILI9806G_wr_dat(0x08);
		
		ILI9806G_wr_cmd(0xDF);		/* Engineering Setting */
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0x02);
		
		ILI9806G_wr_cmd(0xF3);		/* DVDD Voltage Setting */
		ILI9806G_wr_dat(0x74);
		
		ILI9806G_wr_cmd(0xB4);		/* Display Inversion Control */
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0x00);
		
		ILI9806G_wr_cmd(0xF7);		/* 480x854*/
		ILI9806G_wr_dat(0x81); 
		
		ILI9806G_wr_cmd(0xB1);		/* Frame Rate */
		ILI9806G_wr_dat(0x00);
		ILI9806G_wr_dat(0x10);
		ILI9806G_wr_dat(0x14);
		
		ILI9806G_wr_cmd(0xF1);		/* Panel Timing Control */
		ILI9806G_wr_dat(0x29);
		ILI9806G_wr_dat(0x8A);
		ILI9806G_wr_dat(0x07);
		
		ILI9806G_wr_cmd(0xF2);		/* Panel Timing Control */
		ILI9806G_wr_dat(0x40);
		ILI9806G_wr_dat(0xD2);
		ILI9806G_wr_dat(0x50);
		ILI9806G_wr_dat(0x28);
		
		ILI9806G_wr_cmd(0xC1);		/* Power Control 2 */
		ILI9806G_wr_dat(0x17);
		ILI9806G_wr_dat(0x85);
		ILI9806G_wr_dat(0x85);
		ILI9806G_wr_dat(0x20);
		
		ILI9806G_wr_cmd(0xE0); 
		ILI9806G_wr_dat(0x00);		/* P1 */
		ILI9806G_wr_dat(0x0C);		/* P2 */
		ILI9806G_wr_dat(0x15);		/* P3 */
		ILI9806G_wr_dat(0x0D);		/* P4 */
		ILI9806G_wr_dat(0x0F);		/* P5 */
		ILI9806G_wr_dat(0x0C);		/* P6 */
		ILI9806G_wr_dat(0x07);		/* P7 */
		ILI9806G_wr_dat(0x05);		/* P8 */
		ILI9806G_wr_dat(0x07);		/* P9 */
		ILI9806G_wr_dat(0x0B);		/* P10 */
		ILI9806G_wr_dat(0x10);		/* P11 */
		ILI9806G_wr_dat(0x10);		/* P12 */
		ILI9806G_wr_dat(0x0D);		/* P13 */
		ILI9806G_wr_dat(0x17);		/* P14 */
		ILI9806G_wr_dat(0x0F);		/* P15 */
		ILI9806G_wr_dat(0x00);		/* P16 */
		
		ILI9806G_wr_cmd(0xE1); 
		ILI9806G_wr_dat(0x00);		/* P1 */
		ILI9806G_wr_dat(0x0D);		/* P2 */
		ILI9806G_wr_dat(0x15);		/* P3 */
		ILI9806G_wr_dat(0x0E);		/* P4 */
		ILI9806G_wr_dat(0x10);		/* P5 */
		ILI9806G_wr_dat(0x0D);		/* P6 */
		ILI9806G_wr_dat(0x08);		/* P7 */
		ILI9806G_wr_dat(0x06);		/* P8 */
		ILI9806G_wr_dat(0x07);		/* P9 */
		ILI9806G_wr_dat(0x0C);		/* P10 */
		ILI9806G_wr_dat(0x11);		/* P11 */
		ILI9806G_wr_dat(0x11);		/* P12 */
		ILI9806G_wr_dat(0x0E);		/* P13 */
		ILI9806G_wr_dat(0x17);		/* P14 */
		ILI9806G_wr_dat(0x0F);		/* P15 */
		ILI9806G_wr_dat(0x00);		/* P16 */
		
		ILI9806G_wr_cmd(0x35);		/* Tearing Effect ON */
		ILI9806G_wr_dat(0x00);
		
		ILI9806G_wr_cmd(0x36);
		ILI9806G_wr_dat(0x00);
		
		ILI9806G_wr_cmd(0x3A);
		ILI9806G_wr_dat(0x55);
		
		ILI9806G_wr_cmd(0x11);		/* Exit Sleep */
		_delay_ms(120); 
		ILI9806G_wr_cmd(0x29);		/* Display On */
	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI9806G_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ILI9806G_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9806G_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
