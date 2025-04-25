/********************************************************************************/
/*!
	@file			st7735r.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -JD-T18003-T01				(8bit4wire serial only)			@n

    @section HISTORY
		2010.06.03	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "st7735r.h"
/* check header file version for fool proof */
#if ST7735R_H != 0x0400
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
inline void ST7735R_reset(void)
{
#ifdef USE_ST7735R_TFT
	ST7735R_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ST7735R_RD_SET();
	ST7735R_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	ST7735R_RES_CLR();							/* RES=L, CS=L   			*/
	ST7735R_CS_CLR();

#elif  USE_ST7735R_SPI_TFT
	ST7735R_RES_SET();							/* RES=H, CS=H				*/
	ST7735R_CS_SET();
	ST7735R_SCLK_SET();							/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 1ms     			*/

	ST7735R_RES_CLR();							/* RES=L, CS=L   			*/
	ST7735R_CS_CLR();

#endif

	_delay_ms(20);								/* wait 20ms     			*/
	ST7735R_RES_SET();						  	/* RES=H					*/
	_delay_ms(20);				    			/* wait 20ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_ST7735R_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ST7735R_wr_cmd(uint8_t cmd)
{
	ST7735R_DC_CLR();							/* DC=L		     */
	
	ST7735R_CMD = cmd;							/* D7..D0=cmd    */
	ST7735R_WR();								/* WR=L->H       */
	
	ST7735R_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ST7735R_wr_dat(uint8_t dat)
{
	ST7735R_DATA = dat;							/* D7..D0=dat    */
	ST7735R_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ST7735R_wr_gram(uint16_t gram)
{
	ST7735R_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	ST7735R_WR();								/* WR=L->H					*/

	ST7735R_DATA = (uint8_t)gram;				/* lower 8bit data			*/
	ST7735R_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ST7735R_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		ST7735R_wr_dat(*p++);
		ST7735R_wr_dat(*p++);
		ST7735R_wr_dat(*p++);
		ST7735R_wr_dat(*p++);
	}
	while (n--) {
		ST7735R_wr_dat(*p++);
	}
#endif

}

#else /* USE_ST7735R_SPI_TFT */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ST7735R_wr_cmd(uint8_t cmd)
{
	ST7735R_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	ST7735R_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ST7735R_wr_dat(uint8_t dat)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ST7735R_wr_gram(uint16_t gram)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI16(gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t ST7735R_rd_cmd(uint8_t cmd)
{
#ifdef ST7735R_SPI_4WIRE_READID_IGNORE
 #warning "Ingnore ST7735R RDID1 check!"
	return 0x54;
#else	
 #warning "ST7735R USES SDA(Input&Output Multiplexed) Line!"
 	uint8_t temp;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	ST7735R_DC_CLR();							/* DC=L		     */
	SendSPI(cmd);
	ST7735R_DC_SET();							/* DC=L		     */

	Display_ChangeSDA_If(TFT_SDA_READ);			/* Change SDI to Input HI */
	temp  = RecvSPI();
	Display_ChangeSDA_If(TFT_SDA_WRITE);		/* Change SDI to Ouput */
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */
	return temp;
#endif
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ST7735R_wr_block(uint8_t *p,unsigned int cnt)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
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
inline void ST7735R_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	ST7735R_wr_cmd(CASET); 
	ST7735R_wr_dat(0);
	ST7735R_wr_dat(OFS_COL + x);
	ST7735R_wr_dat(0);
	ST7735R_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	ST7735R_wr_cmd(RASET);
	ST7735R_wr_dat(0);
	ST7735R_wr_dat(OFS_RAW + y); 
	ST7735R_wr_dat(0);
	ST7735R_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	ST7735R_wr_cmd(RAMWR);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ST7735R_clear(void)
{
	volatile uint32_t n;

	ST7735R_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		ST7735R_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ST7735R_init(void)
{
	uint8_t devicetype;
	
	Display_IoInit_If();

	ST7735R_reset();

	/* Check Device Code */
	devicetype = ST7735R_rd_cmd(RDID1);  			/* Confirm Vaild LCD Controller */

	if((devicetype == 0x5C) || (devicetype == 0x54) || (devicetype == 0xE3))
	{
		/* Initialize ST7735R */
		ST7735R_wr_cmd(SWRESET);   		/* Sofeware setting */
		_delay_ms(10);
		
		ST7735R_wr_cmd(SLPOUT);			/* Sleep out */
		_delay_ms(120);
		
		ST7735R_wr_cmd(FRMCTR1);        /* ST7735R Frame Rate */
		ST7735R_wr_dat(0x01);
		ST7735R_wr_dat(0x2C);
		ST7735R_wr_dat(0x2D);
		
		ST7735R_wr_cmd(FRMCTR2);        /* ST7735R Frame Rate */
		ST7735R_wr_dat(0x01);
		ST7735R_wr_dat(0x2C);
		ST7735R_wr_dat(0x2D);
		
		ST7735R_wr_cmd(FRMCTR3);        /* ST7735R Frame Rate */
		ST7735R_wr_dat(0x01);
		ST7735R_wr_dat(0x2C);
		ST7735R_wr_dat(0x2D);
		ST7735R_wr_dat(0x01);
		ST7735R_wr_dat(0x2C);
		ST7735R_wr_dat(0x2D);
		
		ST7735R_wr_cmd(INVCTR);			/* Column inversion */
		ST7735R_wr_dat(0x07);
		
		ST7735R_wr_cmd(PWCTR1);			/* ST7735R Power Sequence */
		ST7735R_wr_dat(0xA2);
		ST7735R_wr_dat(0x02);
		ST7735R_wr_dat(0x84);
		
		ST7735R_wr_cmd(PWCTR2);			/* ST7735R Power Sequence */
		ST7735R_wr_dat(0xC5);
		
		ST7735R_wr_cmd(PWCTR3);			/* ST7735R Power Sequence */
		ST7735R_wr_dat(0x0A);
		ST7735R_wr_dat(0x00);
		
		ST7735R_wr_cmd(PWCTR4);			/* ST7735R Power Sequence */
		ST7735R_wr_dat(0x8A);
		ST7735R_wr_dat(0x2A);
		
		ST7735R_wr_cmd(PWCTR5);			/* ST7735R Power Sequence */
		ST7735R_wr_dat(0x8A);
		ST7735R_wr_dat(0xEE);
		
		ST7735R_wr_cmd(VMCTR1);			/* VCOM */
		ST7735R_wr_dat(0x0E);
		
		ST7735R_wr_cmd(MADCTL);			/* MX, MY, RGB mode */
		ST7735R_wr_dat(0xC8);
		
		ST7735R_wr_cmd(GAMCTRP1);		/* ST7735R Gamma Sequence */
		ST7735R_wr_dat(0x02);
		ST7735R_wr_dat(0x1C);
		ST7735R_wr_dat(0x07);
		ST7735R_wr_dat(0x12);
		ST7735R_wr_dat(0x37);
		ST7735R_wr_dat(0x32);
		ST7735R_wr_dat(0x29);
		ST7735R_wr_dat(0x2D);
		ST7735R_wr_dat(0x29);
		ST7735R_wr_dat(0x25);
		ST7735R_wr_dat(0x2B);
		ST7735R_wr_dat(0x39);
		ST7735R_wr_dat(0x00);
		ST7735R_wr_dat(0x01);
		ST7735R_wr_dat(0x03);
		ST7735R_wr_dat(0x10);
		
		ST7735R_wr_cmd(GAMCTRN1);		/* ST7735R Gamma Sequence */
		ST7735R_wr_dat(0x03);
		ST7735R_wr_dat(0x1D);
		ST7735R_wr_dat(0x07);
		ST7735R_wr_dat(0x06);
		ST7735R_wr_dat(0x2E);
		ST7735R_wr_dat(0x2C);
		ST7735R_wr_dat(0x29);
		ST7735R_wr_dat(0x2D);
		ST7735R_wr_dat(0x2E);
		ST7735R_wr_dat(0x2E);
		ST7735R_wr_dat(0x37);
		ST7735R_wr_dat(0x3F);
		ST7735R_wr_dat(0x00);
		ST7735R_wr_dat(0x00);
		ST7735R_wr_dat(0x02);
		ST7735R_wr_dat(0x10);
		
		ST7735R_wr_cmd(CASET);			/* ST7735R Column Address Init */
		ST7735R_wr_dat(0x00);
		ST7735R_wr_dat(0x02);
		ST7735R_wr_dat(0x00);
		ST7735R_wr_dat(0x81);
		
		ST7735R_wr_cmd(RASET);			/* ST7735R Raw Address Init */
		ST7735R_wr_dat(0x00);
		ST7735R_wr_dat(0x01);
		ST7735R_wr_dat(0x00);
		ST7735R_wr_dat(0xA0);
		
		ST7735R_wr_cmd(COLMOD); 		/* ST7735R Colour Mode */
		ST7735R_wr_dat(0x05); 
	 
		ST7735R_wr_cmd(DISPON);			/* Display on */
		_delay_ms(10);
	}

	else { for(;;);} /* Invalid Device Code!! */

	ST7735R_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ST7735R_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		ST7735R_wr_gram(COL_RED);
	} while (--n);


	_delay_ms(500);
	for(;;);

#endif

}


/* End Of File ---------------------------------------------------------------*/
