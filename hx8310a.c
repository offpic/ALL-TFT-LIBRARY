/********************************************************************************/
/*!
	@file			hx8310a.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -MT240TMA7B-04				(HX8310A)	16bit mode.			@n
					 -H018IN01_V8               (NT3915/HD66773) 8/16bit mode.	@n
					 -KTFT1500QQGH0				(ILI9160)   8/16bit mode.

    @section HISTORY
		2012.11.30	V1.00	Start Here.
		2013.06.16	V2.00	Added NT3915/HD66773 Device Support.
		2016.05.20	V3.00	Added ILI9160 Device Support.
		2023.05.01	V4.00	Removed unused delay function.
		2023.08.01	V5.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8310a.h"
/* check header file version for fool proof */
#if HX8310A_H != 0x0500
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
inline void HX8310A_reset(void)
{
	HX8310A_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX8310A_RD_SET();
	HX8310A_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	HX8310A_RES_CLR();							/* RES=L, CS=L   			*/
	HX8310A_CS_CLR();

	_delay_ms(20);								/* wait 20ms     			*/
	HX8310A_RES_SET();						  	/* RES=H					*/
	_delay_ms(30);				    			/* wait 30ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8310A_wr_cmd(uint8_t cmd)
{
	HX8310A_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8310A_CMD = 0;
	HX8310A_WR();
#endif

	HX8310A_CMD = cmd;							/* cmd(8bit)				*/
	HX8310A_WR();								/* WR=L->H					*/

	HX8310A_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void HX8310A_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8310A_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	HX8310A_WR();								/* WR=L->H					*/
	HX8310A_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	HX8310A_DATA = dat;							/* 16bit data 				*/
#endif
	HX8310A_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8310A_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8310A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		HX8310A_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t HX8310A_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	HX8310A_wr_cmd(cmd);
	HX8310A_WR_SET();

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
    ReadLCDData(temp);
#endif

    ReadLCDData(val);

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	val &= 0x00FF;
	val |= temp<<8;
#endif

	return val;
}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8310A_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8310A_wr_cmd(0x16);				/* GRAM Horizontal ADDR Set */
	HX8310A_wr_dat(((OFS_COL + width) <<8) | (OFS_COL + x));
	HX8310A_wr_cmd(0x17);				/* GRAM Vertical ADDR Set */
	HX8310A_wr_dat(((OFS_RAW + height)<<8) | (OFS_RAW + y));
	HX8310A_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	HX8310A_wr_dat(((OFS_RAW + y)<<8) | (OFS_COL + x));

	HX8310A_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8310A_clear(void)
{
	volatile uint32_t n;

	HX8310A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8310A_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8310A_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	HX8310A_reset();

	/* Check Device Code */
	devicetype = HX8310A_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x8310)
	{
		/* Initialize HX8310A */
		HX8310A_wr_cmd(0x00);
		HX8310A_wr_dat(0x0001);

		HX8310A_wr_cmd(0x07);
		HX8310A_wr_dat(0x0004);	

		HX8310A_wr_cmd(0x01);			/* Driver Output Direction */
		HX8310A_wr_dat((1<<8)|0x0013);	/* Up   --> Down;Left --> Right */

		HX8310A_wr_cmd(0x02);			/* LCD AC Signal */	
		HX8310A_wr_dat(0x0700);	

		HX8310A_wr_cmd(0x05);			/* entry mode */
		HX8310A_wr_dat((1<<12)|(1<<5)|(1<<4)|(0<<3));

		HX8310A_wr_cmd(0x06);			/* Compare Register */	
    	HX8310A_wr_dat(0x0000);	

		HX8310A_wr_cmd(0x0B);			/* Frame Contro */
		HX8310A_wr_dat(0x0000); 	
		_delay_ms(20);

		HX8310A_wr_cmd(0x0C);			/* Power 3 */
		HX8310A_wr_dat(0x0002); 	
		_delay_ms(20);              

		HX8310A_wr_cmd(0x03);			/* Power 1 */
		HX8310A_wr_dat(0x0410); 	
		_delay_ms(20);

		HX8310A_wr_cmd(0x04);			/* Power 2 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x0E);			/* Power 5 */
		HX8310A_wr_dat(0x351E);
		HX8310A_wr_cmd(0x0D);			/* Power 4 */
		HX8310A_wr_dat(0x0A1B);
		HX8310A_wr_cmd(0x30);			/* Gamma 1 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x31);			/* Gamma 2 */
		HX8310A_wr_dat(0x0000);	
		HX8310A_wr_cmd(0x32);			/* Gamma 3 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x33);			/* Gamma 4 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x34);			/* Gamma 5 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x35);			/* Gamma 6 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x36);			/* Gamma 7 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x37);			/* Gamma 8 */
		HX8310A_wr_dat(0x0000);	
		HX8310A_wr_cmd(0x3A);			/* Gamma 9 */
		HX8310A_wr_dat(0x0000);	
		HX8310A_wr_cmd(0x3B);			/* Gamma 10 */
		HX8310A_wr_dat(0x0000);

		HX8310A_wr_cmd(0x0F);			/* COM Scan Position */
		HX8310A_wr_dat(0x0000);			/* 02 */

		HX8310A_wr_cmd(0x14);			/* 1'Screen Address */
		HX8310A_wr_dat(0x9F00);
	
		HX8310A_wr_cmd(0x16);			/* Window Horizonta Segm */
		HX8310A_wr_dat(0x7F00);
		HX8310A_wr_cmd(0x17);			/* Window Vertical Com */
		HX8310A_wr_dat(0x9F00);

		HX8310A_wr_cmd(0x07);			/* Diaplay Control */
		HX8310A_wr_dat(0x0005);

		HX8310A_wr_cmd(0x07);   
		HX8310A_wr_dat(0x0025);
		HX8310A_wr_cmd(0x07);
		HX8310A_wr_dat(0x0027);
		HX8310A_wr_cmd(0x07);
		HX8310A_wr_dat(0x0037);
	}

	else if((devicetype == 0x3911) || (devicetype == 0x0773))
	{
		/* Initialize NT3915(HD66773?) */
		HX8310A_wr_cmd(0x07);			/* Display control GON=0,DTE=0,D1=0,D0=0, */
		HX8310A_wr_dat(0x0100);	
		HX8310A_wr_cmd(0x0D);			/* Power 4 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x00);			/* OSC Start */
		HX8310A_wr_dat(0x0001);
		_delay_ms(20);
		HX8310A_wr_cmd(0x0E);			/* Power 5 */
		HX8310A_wr_dat(0x2000);
		HX8310A_wr_cmd(0x0C);			/* Power 3 */
		HX8310A_wr_dat(0x0001);
		HX8310A_wr_cmd(0x0D);			/* Power 4 */
		HX8310A_wr_dat(0x0003);
		HX8310A_wr_cmd(0x04);			/* Power 2 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x0D);			/* Power 4 */
		HX8310A_wr_dat(0x0903);
		HX8310A_wr_cmd(0x03);			/* Power 1 */
		HX8310A_wr_dat(0x0414);
		HX8310A_wr_cmd(0x0E);			/* Power 5 */
		HX8310A_wr_dat(0x1212);
		_delay_ms(50);
		HX8310A_wr_cmd(0x0D);			/* Power 4 */
		HX8310A_wr_dat(0x031D);

		HX8310A_wr_cmd(0x01);			/* Driver Output Direction */
		HX8310A_wr_dat((1<<8)|0x0013);	/* Up   --> Down;Left --> Right */
		HX8310A_wr_cmd(0x02);			/* LCD AC Signal */	
		HX8310A_wr_dat(0x0700);	
		HX8310A_wr_cmd(0x05);			/* entry mode */
		HX8310A_wr_dat((1<<12)|(1<<5)|(1<<4)|(0<<3));
		HX8310A_wr_cmd(0x06);			/* Compare Register */	
    	HX8310A_wr_dat(0x0000);	
		HX8310A_wr_cmd(0x0B);			/* Frame Contro */
		HX8310A_wr_dat(0x4001);
		HX8310A_wr_cmd(0x0F);			/* COM Scan Position */
		HX8310A_wr_dat(0x0000);			/* 02 */

		HX8310A_wr_cmd(0x11);
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x14);			/* 1'Screen Address */
		HX8310A_wr_dat(0x9F00);
		HX8310A_wr_cmd(0x15);			/* 1'Screen Address */
		HX8310A_wr_dat(0x0000);

		HX8310A_wr_cmd(0x30);			/* Gamma 1 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x31);			/* Gamma 2 */
		HX8310A_wr_dat(0x0204);	
		HX8310A_wr_cmd(0x32);			/* Gamma 3 */
		HX8310A_wr_dat(0x0302);
		HX8310A_wr_cmd(0x33);			/* Gamma 4 */
		HX8310A_wr_dat(0x0000);
		HX8310A_wr_cmd(0x34);			/* Gamma 5 */
		HX8310A_wr_dat(0x0504);
		HX8310A_wr_cmd(0x35);			/* Gamma 6 */
		HX8310A_wr_dat(0x0405);
		HX8310A_wr_cmd(0x36);			/* Gamma 7 */
		HX8310A_wr_dat(0x0707);
		HX8310A_wr_cmd(0x37);			/* Gamma 8 */
		HX8310A_wr_dat(0x0100);	
		HX8310A_wr_cmd(0x3A);			/* Gamma 9 */
		HX8310A_wr_dat(0x1506);	
		HX8310A_wr_cmd(0x3B);			/* Gamma 10 */
		HX8310A_wr_dat(0x000F);

		HX8310A_wr_cmd(0x07);			/* Diaplay Control */
		HX8310A_wr_dat(0x0005);
		HX8310A_wr_cmd(0x07);   
		HX8310A_wr_dat(0x0025);
		HX8310A_wr_cmd(0x07);
		HX8310A_wr_dat(0x0027);
		HX8310A_wr_cmd(0x07);
		HX8310A_wr_dat(0x0137);
		_delay_ms(10);
	}
	
	else if(devicetype == 0x9160)
	{
		/* Initialize ILI9160 */
		HX8310A_wr_cmd(0x00);
		HX8310A_wr_dat(0x0001); /* oscillation */

		HX8310A_wr_cmd(0x0C);
		HX8310A_wr_dat(0x0000);

		HX8310A_wr_cmd(0x0D);
		HX8310A_wr_dat(0x0014);

		HX8310A_wr_cmd(0x0E);
		HX8310A_wr_dat(0x351e);

		HX8310A_wr_cmd(0x09);
		HX8310A_wr_dat(0x0004);

		HX8310A_wr_cmd(0x03);
		HX8310A_wr_dat(0x0010);

		HX8310A_wr_cmd(0x01);
		HX8310A_wr_dat(0x0113);

		HX8310A_wr_cmd(0x02);
		HX8310A_wr_dat(0x0700);

		HX8310A_wr_cmd(0x05);
		HX8310A_wr_dat(0x1030);

		HX8310A_wr_cmd(0x25);
		HX8310A_wr_dat(0x0000);

		HX8310A_wr_cmd(0x26);
		HX8310A_wr_dat(0x0000);

		HX8310A_wr_cmd(0x0A);
		HX8310A_wr_dat(0x0000);

		HX8310A_wr_cmd(0x0B);
		HX8310A_wr_dat(0x0000);

		HX8310A_wr_cmd(0x0F);
		HX8310A_wr_dat(0x0000);

		HX8310A_wr_cmd(0x16);
		HX8310A_wr_dat(0x7F00);

		HX8310A_wr_cmd(0x17);
		HX8310A_wr_dat(0x9F00);

		HX8310A_wr_cmd(0x21);
		HX8310A_wr_dat(0x0000);

		HX8310A_wr_cmd(0x07);
		HX8310A_wr_dat(0x0037);
		HX8310A_wr_cmd(0x30);
		HX8310A_wr_dat(0x0500);
		HX8310A_wr_cmd(0x31);
		HX8310A_wr_dat(0x0707);
		HX8310A_wr_cmd(0x32);
		HX8310A_wr_dat(0x0607);
		HX8310A_wr_cmd(0x33);
		HX8310A_wr_dat(0x0300);
		HX8310A_wr_cmd(0x34);
		HX8310A_wr_dat(0x0500);
		HX8310A_wr_cmd(0x35);
		HX8310A_wr_dat(0x0707);
		HX8310A_wr_cmd(0x36);
		HX8310A_wr_dat(0x0607);
		HX8310A_wr_cmd(0x37);
		HX8310A_wr_dat(0x0300);
		HX8310A_wr_cmd(0x3A);
		HX8310A_wr_dat(0x1F00);
		HX8310A_wr_cmd(0x3B);
		HX8310A_wr_dat(0x1F00);
	}

	else { for(;;);} /* Invalid Device Code!! */

	HX8310A_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8310A_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8310A_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
