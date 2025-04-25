/********************************************************************************/
/*!
	@file			ssd1963.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available "Raw" TFT-Modules are listed below.				@n
					This driver is intened for 565-16bit Interface Only!		@m
 					 -AT043TN24 V.1												@n
 					 -HSD043-BOO

    @section HISTORY
		2011.05.22	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2011.12.23	V3.00	Optimize Some Codes.
		2012.08.01	V4.00	Add HSD043-B00 Support.
		2023.05.01	V5.00	Removed unused delay function.
		2023.08.01	V6.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ssd1963.h"
/* check header file version for fool proof */
#if SSD1963_H != 0x0600
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#if defined(USE_AT043TN24_V1)
 #warning "You Select AT043TN24 V.1 Module!"
#elif  defined(USE_HSD043_BOO)
 #warning "You Select HSD043-BOO Module!"
#else
 #error "U MUST select LCD Molule Model at first!."
#endif

/* Variables -----------------------------------------------------------------*/

/* Constants -----------------------------------------------------------------*/

/* Function prototypes -------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

/**************************************************************************/
/*! 
    Display Module Reset Routine.
*/
/**************************************************************************/
inline void SSD1963_reset(void)
{
	SSD1963_RES_CLR();							/* RES=H, RD=H, WR=H   		*/
	SSD1963_RD_SET();
	SSD1963_WR_SET();
	SSD1963_CS_SET();
	_delay_ms(50);								/* wait 50ms     			*/

	SSD1963_RES_CLR();							/* RES=L, CS=L   			*/
	SSD1963_CS_CLR();
	_delay_ms(10);								/* wait 10ms     			*/
	
	SSD1963_RES_SET();						  	/* RES=H					*/
	_delay_ms(130);				    			/* wait least 120ms     	*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SSD1963_wr_cmd(uint8_t cmd)
{
	SSD1963_DC_CLR();							/* DC=L						*/

	SSD1963_CMD = cmd;							/* cmd(8bit)				*/
	SSD1963_WR();								/* WR=L->H					*/

	SSD1963_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and Gram.
*/
/**************************************************************************/
inline void SSD1963_wr_dat(uint16_t dat)
{
	SSD1963_DATA = (uint8_t)dat;				/* lower 8bit data			*/
	SSD1963_WR();
}

/**************************************************************************/
/*! 
    Write LCD Gram(565-16Bit).
*/
/**************************************************************************/
inline void SSD1963_wr_gram(uint16_t gram)
{
	SSD1963_DATA = gram;						/* 16bit data 				*/
	SSD1963_WR();								/* WR=L->H					*/
}


/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SSD1963_wr_block(uint8_t *p, unsigned int cnt)
{

	/* SSD1963_wr_cmd(0x2C);*/	 /* MUST Need Every Block Transfer, but... */

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		SSD1963_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		SSD1963_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SSD1963_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	SSD1963_wr_cmd(0x2A);				/* Horizontal Start,End ADDR */
	SSD1963_wr_dat((OFS_COL + x)>>8);
	SSD1963_wr_dat(OFS_COL + x);
	SSD1963_wr_dat((OFS_COL + width)>>8);
	SSD1963_wr_dat(OFS_COL + width);

	SSD1963_wr_cmd(0x2B);				/* Vertical Start,End ADDR */
	SSD1963_wr_dat((OFS_RAW + y)>>8);
	SSD1963_wr_dat(OFS_RAW + y);
	SSD1963_wr_dat((OFS_RAW + height)>>8);
	SSD1963_wr_dat(OFS_RAW + height);

	SSD1963_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SSD1963_clear(void)
{
	volatile uint32_t n;

	SSD1963_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1963_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t SSD1963_rd_cmd(uint8_t cmd)
{
	volatile uint16_t valh,vall;

	SSD1963_wr_cmd(cmd);
	SSD1963_WR_SET();

	_delay_ms(1);		/* Some Wait Must be Need, Nemui-San Said So... */
    ReadLCDData(valh);
	ReadLCDData(vall);

	vall &= 0x00FF;
	vall |= valh<<8;

	ReadLCDData(valh);	/* Dummy Read */
	ReadLCDData(valh);  /* Dummy Read */
	ReadLCDData(valh);  /* Dummy Read */

	return vall;

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void SSD1963_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	SSD1963_reset();

	/* Check Device Code */
	devicetype = SSD1963_rd_cmd(0xA1);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0157)
	{
		/* Initialize SSD1963 */
		SSD1963_wr_cmd(0xE2);						/* PLL multiplier, set PLL clock to 120M */
		SSD1963_wr_dat(0x23);	   					/* N=0x36 for 6.5M, 0x23 for 10M crystal */
		SSD1963_wr_dat(0x02);
		SSD1963_wr_dat(0x04);
		
		SSD1963_wr_cmd(0xE0);						/* PLL enable */
		SSD1963_wr_dat(0x01);
		_delay_ms(1);
		
		SSD1963_wr_cmd(0xE0);
		SSD1963_wr_dat(0x03);
		_delay_ms(5);
		
		SSD1963_wr_cmd(0x01);  						/* software reset */
		_delay_ms(5);
		
		SSD1963_wr_cmd(0xE6);						/* PLL setting for PCLK, depends on resolution */
		SSD1963_wr_dat(0x01);
		SSD1963_wr_dat(0x48);
		SSD1963_wr_dat(0x9E);
		
		SSD1963_wr_cmd(0xB0);						/* LCD SPECIFICATION */
		SSD1963_wr_dat(MODULETYPE);
		SSD1963_wr_dat(0x00);
		SSD1963_wr_dat((HDP>>8));  					/* Set HDP */
		SSD1963_wr_dat(HDP);
		SSD1963_wr_dat((VDP>>8)); 					/* Set VDP */
		SSD1963_wr_dat(VDP);
		SSD1963_wr_dat(0x00);
		_delay_ms(5);
		
		SSD1963_wr_cmd(0xB4);						/* HSYNC */
		SSD1963_wr_dat((HT>>8));  					/* Set HT */
		SSD1963_wr_dat(HT);
		SSD1963_wr_dat((HPS>>8));  					/* Set HPS */
		SSD1963_wr_dat(HPS);
		SSD1963_wr_dat(HPW);			   			/* Set HPW */
		SSD1963_wr_dat((LPS>>8));  					/* SetLPS */
		SSD1963_wr_dat(LPS);
		SSD1963_wr_dat(0x00);
		
		SSD1963_wr_cmd(0xB6);						/* VSYNC */
		SSD1963_wr_dat((VT>>8));   					/* Set VT */
		SSD1963_wr_dat(VT);
		SSD1963_wr_dat((VPS>>8));  					/* Set VPS */
		SSD1963_wr_dat(VPS);
		SSD1963_wr_dat(VPW);			   			/* Set VPW */
		SSD1963_wr_dat((FPS>>8));  					/* Set FPS */
		SSD1963_wr_dat(FPS);
		
		SSD1963_wr_cmd(0x36); 						/* rotation */
		SSD1963_wr_dat((0<<1)|(0<<0));
		
		SSD1963_wr_cmd(0xF0); 						/* pixel data interface */
		SSD1963_wr_dat(0x03);						/* 565 16Bit-Bus Interface */
		_delay_ms(5);
		
		SSD1963_wr_cmd(0x29); 						/* display on */
	}

	else { for(;;);} /* Invalid Device Code!! */

	SSD1963_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	SSD1963_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		SSD1963_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
