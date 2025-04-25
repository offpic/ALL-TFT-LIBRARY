/********************************************************************************/
/*!
	@file			r61408.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        2.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -TK040F1532				(R61408)	8/16bit mode.		

    @section HISTORY
		2023.06.01	V1.00	Start Here.
		2023.08.01	V2.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "r61408.h"
/* check header file version for fool proof */
#if R61408_H != 0x0200
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
inline void R61408_reset(void)
{
	R61408_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	R61408_RD_SET();
	R61408_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/
	
	R61408_RES_CLR();							/* RES=L, CS=L   			*/
	R61408_CS_CLR();
	_delay_ms(10);								/* wait 10ms     			*/
	
	R61408_RES_SET();						  	/* RES=H					*/
	_delay_ms(100);				    			/* wait 100ms     			*/
}


/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void R61408_wr_cmd(uint8_t cmd)
{
	R61408_DC_CLR();							/* DC=L						*/

	R61408_CMD = cmd;							/* cmd(8bit)				*/
	R61408_WR();								/* WR=L->H					*/

	R61408_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void R61408_wr_dat(uint8_t dat)
{
	R61408_DATA = dat;							/* data						*/
	R61408_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void R61408_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	R61408_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	R61408_WR();								/* WR=L->H					*/
	R61408_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	R61408_DATA = gram;							/* 16bit data 				*/
#endif
	R61408_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void R61408_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		R61408_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		R61408_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t R61408_rd_cmd(uint8_t cmd)
{
	uint8_t temp,i;
	uint16_t val;


	R61408_wr_cmd(cmd);
	R61408_WR_SET();

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
    Set Rectangle.
*/
/**************************************************************************/
inline void R61408_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	R61408_wr_cmd(0x2A);					/* Horizontal RAM Start ADDR */
	R61408_wr_dat((OFS_COL + x)>>8);
	R61408_wr_dat(OFS_COL + x);
	R61408_wr_dat((OFS_COL + width)>>8);
	R61408_wr_dat(OFS_COL + width);

	R61408_wr_cmd(0x2B);					/* Horizontal RAM Start ADDR */
	R61408_wr_dat((OFS_RAW + y)>>8);
	R61408_wr_dat(OFS_RAW + y);
	R61408_wr_dat((OFS_RAW + height)>>8);
	R61408_wr_dat(OFS_RAW + height);

	R61408_wr_cmd(0x2C);					/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void R61408_clear(void)
{
	volatile uint32_t n;

	R61408_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61408_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void R61408_init(void)
{
	volatile uint16_t devicetype;

	Display_IoInit_If();

	R61408_reset();

	/* Enable ALL Manufactutre Command For R61408 */
	R61408_wr_cmd(0xB0);
	R61408_wr_dat(0x00);

	/* Check Device Code */
	devicetype = R61408_rd_cmd(0xBF);  	/* Confirm Vaild LCD Controller */

	if(devicetype == 0x1408)
	{
		/* Initialize R61408 */
		R61408_wr_cmd(0x11);	/* exit_sleep_mode */
		_delay_ms(40);
		
		R61408_wr_cmd(0xB0);	/* MCAP: Manufacturer Command Access Protect */
		R61408_wr_dat(0x04);	/* Manufacturer Command inputs are enabled - enabled all */
		
		R61408_wr_cmd(0xB3);	/* Frame Memory Access and Interface Setting (B3h) */
		R61408_wr_dat(0x02);	/* MIPI DBI Type-C internal Clocked */
		R61408_wr_dat(0x00);	/* MIPI DBI Type-B internal Clocked */
		
		R61408_wr_cmd(0xB6);	/* DSI Control (B6h)*/
		R61408_wr_dat(0x52);
		R61408_wr_dat(0x83);	/* Operating frequency (MHz), Min: 140 Max: 210*/
		
		R61408_wr_cmd(0xB7);	/* MDDI Control */
		R61408_wr_dat(0x80);
		R61408_wr_dat(0x72);
		R61408_wr_dat(0x11);
		R61408_wr_dat(0x25);
		
		R61408_wr_cmd(0xB8);	/* Backlight Control (1) */
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x0F);
		R61408_wr_dat(0x0F);
		R61408_wr_dat(0xFF);
		R61408_wr_dat(0xFF);
		R61408_wr_dat(0xC8);
		R61408_wr_dat(0xC8);
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x18);
		R61408_wr_dat(0x10);
		R61408_wr_dat(0x10);
		R61408_wr_dat(0x37);
		R61408_wr_dat(0x5A);
		R61408_wr_dat(0x87);
		R61408_wr_dat(0xBE);
		R61408_wr_dat(0xFF);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		
		R61408_wr_cmd(0xB9);	/* Backlight Control (2) (B9h) */
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		
		R61408_wr_cmd(0xBD);	/* Resizing Control (BDh) */
		R61408_wr_dat(0x00);
		
		R61408_wr_cmd(0xC0);	/* Panel Driving Setting 1 (C0h) */
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x76);
		
		R61408_wr_cmd(0xC1);	/* Panel Driving Setting 2 (C1h) */
		R61408_wr_dat(0x63);
		R61408_wr_dat(0x31);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x27);
		R61408_wr_dat(0x27);
		R61408_wr_dat(0x32);
		R61408_wr_dat(0x12);
		R61408_wr_dat(0x28);
		R61408_wr_dat(0x4E);
		R61408_wr_dat(0x10);
		R61408_wr_dat(0xA5);
		R61408_wr_dat(0x0F);
		R61408_wr_dat(0x58);
		R61408_wr_dat(0x21);
		R61408_wr_dat(0x01);
		
		R61408_wr_cmd(0xC2);	/* Display V-Timing Setting (C2h) */
		R61408_wr_dat(0x28);
		R61408_wr_dat(0x06);
		R61408_wr_dat(0x06);
		R61408_wr_dat(0x01);
		R61408_wr_dat(0x03);
		R61408_wr_dat(0x00);
		
		R61408_wr_cmd(0xC4);	/* Panel Driving Setting 3 (C4h) */
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x01);
		
		R61408_wr_cmd(0xC6);	/* Outline Sharpening Control (C6h) */
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		
		R61408_wr_cmd(0xC7);	/* Panel Driving Setting 4 (C7h) */
		R61408_wr_dat(0x11);
		R61408_wr_dat(0x8D);
		R61408_wr_dat(0xA0);
		R61408_wr_dat(0xF5);
		R61408_wr_dat(0x27);
		
		R61408_wr_cmd(0xC8);	/* Gamma Setting A Set (C8h) */
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x13);
		R61408_wr_dat(0x18);
		R61408_wr_dat(0x25);
		R61408_wr_dat(0x34);
		R61408_wr_dat(0x4E);
		R61408_wr_dat(0x36);
		R61408_wr_dat(0x23);
		R61408_wr_dat(0x17);
		R61408_wr_dat(0x0E);
		R61408_wr_dat(0x0C);
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x13);
		R61408_wr_dat(0x18);
		R61408_wr_dat(0x25);
		R61408_wr_dat(0x34);
		R61408_wr_dat(0x4E);
		R61408_wr_dat(0x36);
		R61408_wr_dat(0x23);
		R61408_wr_dat(0x17);
		R61408_wr_dat(0x0E);
		R61408_wr_dat(0x0C);
		R61408_wr_dat(0x02);
		
		R61408_wr_cmd(0xC9);	/* Gamma Setting B Set (C9h) */
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x13);
		R61408_wr_dat(0x18);
		R61408_wr_dat(0x25);
		R61408_wr_dat(0x34);
		R61408_wr_dat(0x4E);
		R61408_wr_dat(0x36);
		R61408_wr_dat(0x23);
		R61408_wr_dat(0x17);
		R61408_wr_dat(0x0E);
		R61408_wr_dat(0x0C);
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x13);
		R61408_wr_dat(0x18);
		R61408_wr_dat(0x25);
		R61408_wr_dat(0x34);
		R61408_wr_dat(0x4E);
		R61408_wr_dat(0x36);
		R61408_wr_dat(0x23);
		R61408_wr_dat(0x17);
		R61408_wr_dat(0x0E);
		R61408_wr_dat(0x0C);
		R61408_wr_dat(0x02);
		
		R61408_wr_cmd(0xCA);	/* Gamma Setting C Set (CAh) */
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x13);
		R61408_wr_dat(0x18);
		R61408_wr_dat(0x25);
		R61408_wr_dat(0x34);
		R61408_wr_dat(0x4E);
		R61408_wr_dat(0x36);
		R61408_wr_dat(0x23);
		R61408_wr_dat(0x17);
		R61408_wr_dat(0x0E);
		R61408_wr_dat(0x0C);
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x13);
		R61408_wr_dat(0x18);
		R61408_wr_dat(0x25);
		R61408_wr_dat(0x34);
		R61408_wr_dat(0x4E);
		R61408_wr_dat(0x36);
		R61408_wr_dat(0x23);
		R61408_wr_dat(0x17);
		R61408_wr_dat(0x0E);
		R61408_wr_dat(0x0C);
		R61408_wr_dat(0x02);
		
		R61408_wr_cmd(0xD0);	/* Power Setting (Charge Pump Setting) (D0h) */
		R61408_wr_dat(0xA9);
		R61408_wr_dat(0x03);
		R61408_wr_dat(0xCC);
		R61408_wr_dat(0xA5);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x53);
		R61408_wr_dat(0x20);
		R61408_wr_dat(0x10);
		R61408_wr_dat(0x01);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x01);
		R61408_wr_dat(0x01);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x03);
		R61408_wr_dat(0x01);
		R61408_wr_dat(0x00);
		
		R61408_wr_cmd(0xD1);	/* Power Setting (Switching Regulator Setting) (D1h) */
		R61408_wr_dat(0x18);
		R61408_wr_dat(0x0C);
		R61408_wr_dat(0x23);
		R61408_wr_dat(0x03);
		R61408_wr_dat(0x75);
		R61408_wr_dat(0x02);
		R61408_wr_dat(0x50);
		
		R61408_wr_cmd(0xD3);	/* Power Setting for Internal Mode (D3h) */
		R61408_wr_dat(0x33);
		R61408_wr_cmd(0xD5);	/* VPLVL/VNLVL Setting (D5h) */
		R61408_wr_dat(0x2a);
		R61408_wr_dat(0x2a);
		
		R61408_wr_cmd(0xDE);	/* VCOMDC Setting (DEh) */
		R61408_wr_dat(0x01);
		R61408_wr_dat(0x51);
		
		R61408_wr_cmd(0xE1);	/* set_DDB_write_control (E1h) */
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		R61408_wr_dat(0x00);
		
		R61408_wr_cmd(0xE6);	/* VCOMDC Setting 2 (E6h) */
		R61408_wr_dat(0x55);
		
		R61408_wr_cmd(0xFA);	/* VDC_SEL Setting (FAh) */
		R61408_wr_dat(0x01);
		
		R61408_wr_cmd(0xB0);	/* MCAP: Manufacturer Command Access Protect */
		R61408_wr_dat(0x04);	/* Manufacturer Command inputs are enabled - enabled all */
		_delay_ms(40);
		
		R61408_wr_cmd(0x35);	/* set_tear_on: 35h */
		R61408_wr_dat(0x00);
		
		R61408_wr_cmd(0x44);	/* set_tear_scanline:44h */
		R61408_wr_dat(0x00);
		
		R61408_wr_cmd(0x36);	/* set_address_mode: 36h */
		R61408_wr_dat(0x00);
		
		R61408_wr_cmd(0x3A);	/* set_pixel_format: 3Ah */
		R61408_wr_dat(0x55);	/* 16 bits/pixel (65,536 colors) */
		
		R61408_wr_cmd(0x29);	/* set_display_on: 29h */
		R61408_wr_dat(0x00);
	}

	else { for(;;);} /* Invalid Device Code!! */

	R61408_clear();

#if 0 	/* test code RED */
	volatile uint32_t n;

	R61408_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		R61408_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
