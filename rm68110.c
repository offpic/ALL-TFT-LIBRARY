/********************************************************************************/
/*!
	@file			rm68110.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -DT18QQ003-B1R2				(4-wire serial)				@n
					 -BBM018027C1-F8				(8bit mode)

    @section HISTORY
		2014.10.21	V1.00	Stable Release.
		2015.11.03  V2.00	Added BBM018027C1-F8 Module Support.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "rm68110.h"
/* check header file version for fool proof */
#if RM68110_H != 0x0400
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#ifdef  USE_RM68110_SPI_TFT
#ifndef TFT_SDA_READ
 #warning Display_ChangeSDA_If() is NOT Implemented!!
 #define Display_ChangeSDA_If(...) ;
#endif
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
inline void RM68110_reset(void)
{
#ifdef USE_RM68110_TFT
	RM68110_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	RM68110_RD_SET();
	RM68110_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	RM68110_RES_CLR();							/* RES=L, CS=L   			*/
	RM68110_CS_CLR();

#elif  USE_RM68110_SPI_TFT
	RM68110_RES_SET();							/* RES=H, CS=H				*/
	RM68110_CS_SET();
	RM68110_SCLK_SET();							/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 1ms     			*/

	RM68110_RES_CLR();							/* RES=L, CS=L   			*/
	RM68110_CS_CLR();

#endif

	_delay_ms(20);								/* wait 20ms     			*/
	RM68110_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait at least 120 ms     */
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_RM68110_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void RM68110_wr_cmd(uint8_t cmd)
{
	RM68110_DC_CLR();							/* DC=L		     		*/
	
	RM68110_CMD = cmd;							/* D7..D0=cmd    		*/
	RM68110_WR();								/* WR=L->H       		*/
	
	RM68110_DC_SET();							/* DC=H   	     		*/
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void RM68110_wr_dat(uint8_t dat)
{
	RM68110_DATA = dat;							/* D7..D0=dat   		 */
	RM68110_WR();								/* WR=L->H      		 */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void RM68110_wr_gram(uint16_t gram)
{
	RM68110_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	RM68110_WR();								/* WR=L->H					*/

	RM68110_DATA = (uint8_t)gram;				/* lower 8bit data			*/
	RM68110_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t RM68110_rd_cmd(uint8_t cmd)
{
	uint16_t val,temp;

	RM68110_wr_cmd(cmd);
	RM68110_WR_SET();

    ReadLCDData(temp);	/* Dummy */
    ReadLCDData(temp);
    ReadLCDData(val);
	val &= 0x00FF;
	val |= temp<<8;

	return val;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void RM68110_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		RM68110_wr_dat(*p++);
		RM68110_wr_dat(*p++);
		RM68110_wr_dat(*p++);
		RM68110_wr_dat(*p++);
	}
	while (n--) {
		RM68110_wr_dat(*p++);
	}
#endif

}

#else /* USE_RM68110_SPI_TFT */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void RM68110_wr_cmd(uint8_t cmd)
{
	RM68110_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	RM68110_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void RM68110_wr_dat(uint8_t dat)
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
inline void RM68110_wr_gram(uint16_t gram)
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
inline uint16_t RM68110_rd_cmd(uint8_t cmd)
{
#ifdef TFT_SDA_READ
	uint16_t val;
	uint8_t temp,temp2,temp3;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	RM68110_DC_CLR();							/* DC=L			 */

	SendSPI(cmd);

	RM68110_DC_SET();							/* DC=H			 */

	Display_ChangeSDA_If(TFT_SDA_READ);
	temp   = RecvSPI();							/* Dummy Read */
	temp <<=1;									/* Shift due to Dummy Clock */
	temp2  = RecvSPI();							/* Upper Read */
	temp3  = RecvSPI();							/* Lower Read */
	Display_ChangeSDA_If(TFT_SDA_WRITE);
	val = (uint16_t)(temp<<8)|(uint16_t)(temp2<<1)|(temp3>>7);
	DISPLAY_NEGATE_CS();						/* CS=H		     */

	return val;
#else
	return 0x6811;
#endif
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void RM68110_wr_block(uint8_t *p,unsigned int cnt)
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
inline void RM68110_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	RM68110_wr_cmd(0x2A); 
	RM68110_wr_dat(0);
	RM68110_wr_dat(OFS_COL + x);
	RM68110_wr_dat(0);
	RM68110_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	RM68110_wr_cmd(0x2B);
	RM68110_wr_dat(0);
	RM68110_wr_dat(OFS_RAW + y); 
	RM68110_wr_dat(0);
	RM68110_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	RM68110_wr_cmd(0x2C);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void RM68110_clear(void)
{
	volatile uint32_t n;

	RM68110_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		RM68110_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void RM68110_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	RM68110_reset();

	/* Check Device Code */
	devicetype = RM68110_rd_cmd(0xD3);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x6811)
	{
		/* Initialize RM68110 */
		RM68110_wr_cmd(0x01);		/* Software Reset */
		RM68110_wr_cmd(0x11);		/* Sleep Out */
		_delay_ms(120);       		/* Must Delay At Least 100 ms */
		
#ifdef  USE_RM68110_SPI_TFT
		RM68110_wr_cmd(0xC0);		/* Power Control 1 */
		RM68110_wr_dat(0xF0);     	/*  GVDD Setting */
		RM68110_wr_dat(0x10);     	/*  GVCL Setting */
		
		RM68110_wr_cmd(0xC1);  		/* set VGH&VGL driver voltage */
		RM68110_wr_dat(0x09);
		
		RM68110_wr_cmd(0xC2);
		RM68110_wr_dat(0x02);
		RM68110_wr_dat(0x43);
		
		RM68110_wr_cmd(0xC9);
		RM68110_wr_dat(0x15);
		
		RM68110_wr_cmd(0xB4);
		RM68110_wr_dat(0x00);		/* Dot inversion 0X07 */
		
		RM68110_wr_cmd(0xC5);		/* Set VMH[6:0] & VML[6:0] for VOMH & VCOML */
		RM68110_wr_dat(0x0B);		/* 31 */
		
		RM68110_wr_cmd(0xF8);
		RM68110_wr_dat(0x01);
		
		RM68110_wr_cmd(0x3A);
		RM68110_wr_dat(0x05);
		
		/*  Start Gamma Setting */
		RM68110_wr_cmd(0xE0);		/* Gamma Command */
		RM68110_wr_dat(0x00);      
		RM68110_wr_dat(0x01);
		RM68110_wr_dat(0x05);
		RM68110_wr_dat(0x25);
		RM68110_wr_dat(0x22);
		RM68110_wr_dat(0x1F);
		RM68110_wr_dat(0x03);
		RM68110_wr_dat(0x0D);
		RM68110_wr_dat(0x07);
		RM68110_wr_dat(0x00);
		RM68110_wr_dat(0x0A);
		RM68110_wr_dat(0x09);
		RM68110_wr_dat(0x06);
		RM68110_wr_dat(0x0B);
		RM68110_wr_dat(0x03);
		RM68110_wr_dat(0x07);
		
		RM68110_wr_cmd(0xE1);		/* Gamma Command */
		RM68110_wr_dat(0x00);      
		RM68110_wr_dat(0x01);
		RM68110_wr_dat(0x05);
		RM68110_wr_dat(0x25);
		RM68110_wr_dat(0x22);
		RM68110_wr_dat(0x1F);
		RM68110_wr_dat(0x03);
		RM68110_wr_dat(0x0D);
		RM68110_wr_dat(0x07);
		RM68110_wr_dat(0x00);
		RM68110_wr_dat(0x0A);
		RM68110_wr_dat(0x09);
		RM68110_wr_dat(0x06);
		RM68110_wr_dat(0x0B);
		RM68110_wr_dat(0x03);
		RM68110_wr_dat(0x07);
		
		RM68110_wr_cmd(0x36);		/* Memory Data Access Control */
		RM68110_wr_dat(0xC0);     	/* RGB Color Filter Setting */
		
		RM68110_wr_cmd(0x21);

		RM68110_wr_cmd(0xB1); 		/* set gamma ver */
		RM68110_wr_dat(0x03);
		RM68110_wr_dat(0x04);
		RM68110_wr_dat(0x18);
		
		RM68110_wr_cmd(0xF4);
		RM68110_wr_dat(0x00);
		RM68110_wr_dat(0x08);
		
		RM68110_wr_cmd(0xD9);
		RM68110_wr_dat(0x00); 
		
		RM68110_wr_cmd(0xC7);
		RM68110_wr_dat(0x1B);
	#else
		RM68110_wr_cmd(0xC0);		/* Power Control 1 */
		RM68110_wr_dat(0xD6);
		RM68110_wr_dat(0x16);
		
		RM68110_wr_cmd(0xC1);		/* Set BT[2:0] for AVDD & VCL & VGH & VGL */
		RM68110_wr_dat(0x09);
		
		RM68110_wr_cmd(0xB4);		/* Display Inversion Control */
		RM68110_wr_dat(0x07);
		
		RM68110_wr_cmd(0xC5);		/* VCOM Control 1 */
		RM68110_wr_dat(0x0A);
		
		RM68110_wr_cmd(0xC7);		/* VCOM Offset Control */
		RM68110_wr_dat(0x1F);
		
		RM68110_wr_cmd(0xC9); 
		RM68110_wr_dat(0x15);
		
		RM68110_wr_cmd(0xB1);		/* Frame Rate Control (In normal mode/ Full colors) */
		RM68110_wr_dat(0x01);
		RM68110_wr_dat(0x04);
		RM68110_wr_dat(0x18);
		
		RM68110_wr_cmd(0xF8); 
		RM68110_wr_dat(0x01);
		
		/*  Start Gamma Setting */
		RM68110_wr_cmd(0xE0);
		RM68110_wr_dat(0x00);
		RM68110_wr_dat(0x01);
		RM68110_wr_dat(0x05);
		RM68110_wr_dat(0x28);
		RM68110_wr_dat(0x26);
		RM68110_wr_dat(0x1F);
		RM68110_wr_dat(0x07);
		RM68110_wr_dat(0x0B);
		RM68110_wr_dat(0x05);
		RM68110_wr_dat(0x04);
		RM68110_wr_dat(0x05);
		RM68110_wr_dat(0x08);
		RM68110_wr_dat(0x06);
		RM68110_wr_dat(0x0C);
		RM68110_wr_dat(0x04);
		RM68110_wr_dat(0x09);
		
		RM68110_wr_cmd(0xE1);
		RM68110_wr_dat(0x00);
		RM68110_wr_dat(0x01);
		RM68110_wr_dat(0x05);
		RM68110_wr_dat(0x28);
		RM68110_wr_dat(0x26);
		RM68110_wr_dat(0x1F);
		RM68110_wr_dat(0x07);
		RM68110_wr_dat(0x0B);
		RM68110_wr_dat(0x05);
		RM68110_wr_dat(0x04);
		RM68110_wr_dat(0x05);
		RM68110_wr_dat(0x08);
		RM68110_wr_dat(0x06);
		RM68110_wr_dat(0x0C);
		RM68110_wr_dat(0x04);
		RM68110_wr_dat(0x09);
		
		RM68110_wr_cmd(0x36);
		RM68110_wr_dat(0xC0);
		
		RM68110_wr_cmd(0x3A);
		RM68110_wr_dat(0x05);
		
		RM68110_wr_cmd(0xFE);
		RM68110_wr_dat(0x09);
		RM68110_wr_dat(0xB0);
		RM68110_wr_dat(0x10);
		RM68110_wr_dat(0x48);
		
		RM68110_wr_cmd(0xFD);
		RM68110_wr_dat(0x10);
		RM68110_wr_dat(0xDF);
		RM68110_wr_dat(0x60);
		RM68110_wr_dat(0xF0);
		
		RM68110_wr_cmd(0xF4);
		RM68110_wr_dat(0x00);
		RM68110_wr_dat(0x08);
		
		RM68110_wr_cmd(0xD9);
		RM68110_wr_dat(0x00);
		
		RM68110_wr_cmd(0xC2); 	   		/* Power Control 3 (in Normal mode/ Full colors) */
		RM68110_wr_dat(0x02);
		RM68110_wr_dat(0x83);
		
		RM68110_wr_cmd(0xF8); 	   		/* Gamma Adjustment Enble Control */
		RM68110_wr_dat(0x00);
		
		RM68110_wr_cmd(0x20);
	#endif
	
  		RM68110_wr_cmd(0x29);			/* Display On */
		_delay_ms(20);
	}

	else { for(;;);} /* Invalid Device Code!! */

	RM68110_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	RM68110_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		RM68110_wr_gram(COL_RED);
	} while (--n);


	_delay_ms(500);
	for(;;);

#endif

}


/* End Of File ---------------------------------------------------------------*/
