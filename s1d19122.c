/********************************************************************************/
/*!
	@file			s1d19122.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        9.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive STM025QVT-001 TFT module(8/16bit mode).

    @section HISTORY
		2010.10.01	V1.00	Stable Release
		2010.12.31	V2.00	Added GRAM write function.
		2011.03.10	V3.00	C++ Ready.
		2011.05.27	V4.00	Fixed S1D19122_clear() in 8-bit access.
		2011.10.25	V5.00	Added DMA TransactionSupport.
		2011.12.23	V6.00	Optimize Some Codes.
		2012.01.16	V7.00	Fixed Startup Failure.
		2023.05.01	V8.00	Removed unused delay function.
		2023.08.01	V9.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s1d19122.h"
/* check header file version for fool proof */
#if S1D19122_H != 0x0900
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
inline void S1D19122_reset(void)
{
	S1D19122_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S1D19122_RD_SET();
	S1D19122_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	S1D19122_RES_CLR();							/* RES=L, CS=L   			*/
	S1D19122_CS_CLR();
	_delay_ms(1);								/* wait 1ms     			*/
	
	S1D19122_RES_SET();						  	/* RES=H					*/
	_delay_ms(10);				    			/* wait 10ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S1D19122_wr_cmd(uint8_t cmd)
{
	S1D19122_DC_CLR();							/* DC=L						*/

	S1D19122_CMD = cmd;							/* cmd(8bit)				*/
	S1D19122_WR();								/* WR=L->H					*/

	S1D19122_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void S1D19122_wr_dat(uint8_t dat)
{
	S1D19122_DATA = dat;						/* data(8bit)				*/
	S1D19122_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void S1D19122_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S1D19122_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	S1D19122_WR();								/* WR=L->H					*/
	S1D19122_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	S1D19122_DATA = gram;						/* 16bit data 				*/
#endif
	S1D19122_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S1D19122_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		S1D19122_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		S1D19122_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void S1D19122_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	S1D19122_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	S1D19122_wr_dat((OFS_COL + x)>>8);
	S1D19122_wr_dat(OFS_COL + x);
	S1D19122_wr_dat((OFS_COL + width)>>8);
	S1D19122_wr_dat(OFS_COL + width);

	S1D19122_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	S1D19122_wr_dat((OFS_RAW + y)>>8);
	S1D19122_wr_dat(OFS_RAW + y);
	S1D19122_wr_dat((OFS_RAW + height)>>8);
	S1D19122_wr_dat(OFS_RAW + height);

	S1D19122_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S1D19122_clear(void)
{
	volatile uint32_t n;

	S1D19122_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S1D19122_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t S1D19122_rd_cmd(uint8_t cmd)
{
	uint16_t val;


	S1D19122_wr_cmd(cmd);
	S1D19122_WR_SET();

	ReadLCDData(val);	/* Dummy Read */

    ReadLCDData(val);

	val &= 0x00FF;

	return val;
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S1D19122_init(void)
{
	uint16_t devicetype;
	int i,R,G,B;

	Display_IoInit_If();

	S1D19122_reset();

	/* Check Device Code */
	devicetype = S1D19122_rd_cmd(0xDA);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0029)
	{
		/* Initialize S1D19122 */
		S1D19122_wr_cmd(0x01);
		_delay_ms(10);
		
		S1D19122_wr_cmd(0x11);
		S1D19122_wr_cmd(0x13);
		S1D19122_wr_cmd(0x29);
		
		/*--------------  Display Control ---------*/
		S1D19122_wr_cmd(0xB0);
		S1D19122_wr_dat(0x05);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0xF0);
		S1D19122_wr_dat(0x0A);
		S1D19122_wr_dat(0x41);
		S1D19122_wr_dat(0x02); 
		S1D19122_wr_dat(0x0A);
		S1D19122_wr_dat(0x30);
		S1D19122_wr_dat(0x31);
		S1D19122_wr_dat(0x36);
		S1D19122_wr_dat(0x37);
		S1D19122_wr_dat(0x40);
		S1D19122_wr_dat(0x02);
		S1D19122_wr_dat(0x3F);
		S1D19122_wr_dat(0x40);
		S1D19122_wr_dat(0x02);
		S1D19122_wr_dat(0x81);
		S1D19122_wr_dat(0x04);
		S1D19122_wr_dat(0x05);
		S1D19122_wr_dat(0x64);
		
		/*----------- Gamma  Curve  Set3 Postive----------*/
		S1D19122_wr_cmd(0xFC);
		S1D19122_wr_dat(0x88);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x10);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x10);
		S1D19122_wr_dat(0x42);
		S1D19122_wr_dat(0x42);
		S1D19122_wr_dat(0x22);
		S1D19122_wr_dat(0x11);
		S1D19122_wr_dat(0x11);
		S1D19122_wr_dat(0x22);
		S1D19122_wr_dat(0x99);
		S1D19122_wr_dat(0xAA);
		S1D19122_wr_dat(0xAA);
		S1D19122_wr_dat(0xAA);
		S1D19122_wr_dat(0xBB);
		S1D19122_wr_dat(0xBB);
		S1D19122_wr_dat(0xAA);
		S1D19122_wr_dat(0x33);
		S1D19122_wr_dat(0x33);
		S1D19122_wr_dat(0x11);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0xC0);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		
		/*----------- Gamma  Curve  Set3 Negative----------*/
		S1D19122_wr_cmd(0xFD);
		S1D19122_wr_dat(0x88);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x10);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x10);
		S1D19122_wr_dat(0x42);
		S1D19122_wr_dat(0x42);
		S1D19122_wr_dat(0x22);
		S1D19122_wr_dat(0x11);
		S1D19122_wr_dat(0x11);
		S1D19122_wr_dat(0x22);
		S1D19122_wr_dat(0x99);
		S1D19122_wr_dat(0xAA);
		S1D19122_wr_dat(0xAA);
		S1D19122_wr_dat(0xAA);
		S1D19122_wr_dat(0xBB);
		S1D19122_wr_dat(0xBB);
		S1D19122_wr_dat(0xAA);
		S1D19122_wr_dat(0x33);
		S1D19122_wr_dat(0x33);
		S1D19122_wr_dat(0x11);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x03);
		
		/*----------- EVRSER Regulator Voltage Setting---------*/
		S1D19122_wr_cmd(0xBE);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x15);
		S1D19122_wr_dat(0x16);
		S1D19122_wr_dat(0x08);
		S1D19122_wr_dat(0x09);
		S1D19122_wr_dat(0x15);
		S1D19122_wr_dat(0x10);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		
		/*-----------Module Definiton Setting---------*/
		S1D19122_wr_cmd(0xC0);
		S1D19122_wr_dat(0x0E);
		S1D19122_wr_dat(0x01);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		S1D19122_wr_dat(0x00);
		
		/*-----------PWRDEF Power Ability Ddfinition----------*/
		S1D19122_wr_cmd(0xC1);
		S1D19122_wr_dat(0x2F);
		S1D19122_wr_dat(0x23);
		S1D19122_wr_dat(0xB4);
		S1D19122_wr_dat(0xFF);
		S1D19122_wr_dat(0x24);
		S1D19122_wr_dat(0x03);
		S1D19122_wr_dat(0x20);
		S1D19122_wr_dat(0x02);
		S1D19122_wr_dat(0x02);
		S1D19122_wr_dat(0x02);
		S1D19122_wr_dat(0x20);
		S1D19122_wr_dat(0x20);
		S1D19122_wr_dat(0x00);
		
		/* -----------Other Setting---------- */
		S1D19122_wr_cmd(0xC2);
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
		S1D19122_wr_dat(0x00);		/* mode 8-bit  */
#else
		S1D19122_wr_dat(0x03);		/* mode 16-bit */
#endif		
		S1D19122_wr_cmd(0x26);
		S1D19122_wr_dat(0x08);
		S1D19122_wr_cmd(0x35);
		
		S1D19122_wr_cmd(0x36);
		S1D19122_wr_dat(0x04);
		/*S1D19122_wr_cmd(0x39);*/
		S1D19122_wr_cmd(0x3A);
		S1D19122_wr_dat(0x05);
		/*S1D19122_wr_cmd(0x20);*/  
		S1D19122_wr_cmd(0x2C);
		
		/* -----------RGB Setting---------- */
		S1D19122_wr_cmd(0x2D);
		R=0;
		G=0;
		B=0;
		
		for(i=0;i<32;i++)
		{ 
			S1D19122_wr_dat(R);
			R=R+2;
		}
		for(i=0;i<64;i++)
		{ 
			S1D19122_wr_dat(G);
			G=G+1;
		}
		for(i=0;i<32;i++)
		{ 
			S1D19122_wr_dat(B);
			B=B+2;
		}
		_delay_ms(20);
	}

	else { for(;;);} /* Invalid Device Code!! */

	S1D19122_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	S1D19122_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S1D19122_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
