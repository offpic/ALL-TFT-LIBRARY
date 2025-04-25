/********************************************************************************/
/*!
	@file			s6d02a1.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive UT18022B0-00 TFT module(4-Wire,8bitSerial only).

    @section HISTORY
		2013.11.30	V1.00	Stable Release.
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s6d02a1.h"
/* check header file version for fool proof */
#if S6D02A1_H != 0x0300
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#ifdef  USE_S6D02A1_SPI_TFT
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
inline void S6D02A1_reset(void)
{
#ifdef USE_S6D02A1_TFT
	S6D02A1_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S6D02A1_RD_SET();
	S6D02A1_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	S6D02A1_RES_CLR();							/* RES=L, CS=L   			*/
	S6D02A1_CS_CLR();

#elif  USE_S6D02A1_SPI_TFT
	S6D02A1_RES_SET();							/* RES=H, CS=H				*/
	S6D02A1_CS_SET();
	S6D02A1_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 1ms     			*/

	S6D02A1_RES_CLR();							/* RES=L, CS=L   			*/
	S6D02A1_CS_CLR();

#endif

	_delay_ms(20);								/* wait 20ms     			*/
	S6D02A1_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait at least 120 ms     */
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_S6D02A1_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D02A1_wr_cmd(uint8_t cmd)
{
	S6D02A1_DC_CLR();							/* DC=L		     */
	
	S6D02A1_CMD = cmd;							/* D7..D0=cmd    */
	S6D02A1_WR();								/* WR=L->H       */
	
	S6D02A1_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void S6D02A1_wr_dat(uint8_t dat)
{
	S6D02A1_DATA = dat;							/* D7..D0=dat    */
	S6D02A1_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void S6D02A1_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6D02A1_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	S6D02A1_WR();								/* WR=L->H					*/
	S6D02A1_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	S6D02A1_DATA = gram;						/* 16bit data 				*/
#endif
	S6D02A1_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t S6D02A1_rd_cmd(uint16_t cmd)
{
	uint8_t val;

	S6D02A1_wr_cmd(cmd);
	S6D02A1_WR_SET();

    ReadLCDData(val);	/* Dummy */
    ReadLCDData(val);

	return val;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6D02A1_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		S6D02A1_wr_dat(*p++);
		S6D02A1_wr_dat(*p++);
		S6D02A1_wr_dat(*p++);
		S6D02A1_wr_dat(*p++);
	}
	while (n--) {
		S6D02A1_wr_dat(*p++);
	}
#endif

}

#else /* USE_S6D02A1_SPI_TFT */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D02A1_wr_cmd(uint8_t cmd)
{
	S6D02A1_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	S6D02A1_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void S6D02A1_wr_dat(uint8_t dat)
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
inline void S6D02A1_wr_gram(uint16_t gram)
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
inline uint16_t S6D02A1_rd_cmd(uint8_t cmd)
{
#ifdef TFT_SDA_READ
#warning "S6D02A1 USES ONLY SDA(Input&Output Multiplexed) Line!"

	uint8_t val;

	DISPLAY_ASSART_CS();						/* CS=L		     */

	S6D02A1_DC_CLR();							/* DC=L		     */
	SendSPI(cmd);
	S6D02A1_DC_SET();							/* DC=L		     */

	Display_ChangeSDA_If(TFT_SDA_READ);
	val  = RecvSPI();
	Display_ChangeSDA_If(TFT_SDA_WRITE);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
#else
	return val;
#endif
	return 0x5C;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6D02A1_wr_block(uint8_t *p,unsigned int cnt)
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
inline void S6D02A1_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	S6D02A1_wr_cmd(0x2A); 
	S6D02A1_wr_dat(0);
	S6D02A1_wr_dat(OFS_COL + x);
	S6D02A1_wr_dat(0);
	S6D02A1_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	S6D02A1_wr_cmd(0x2B);
	S6D02A1_wr_dat(0);
	S6D02A1_wr_dat(OFS_RAW + y); 
	S6D02A1_wr_dat(0);
	S6D02A1_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	S6D02A1_wr_cmd(0x2C);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S6D02A1_clear(void)
{
	volatile uint32_t n;

	S6D02A1_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		S6D02A1_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S6D02A1_init(void)
{
	uint8_t devicetype;
	
	Display_IoInit_If();

	S6D02A1_reset();

	/* Check Device Code */
	devicetype = S6D02A1_rd_cmd(0xDA);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x5C)
	{
		/* Initialize S6D02A1 */
		S6D02A1_wr_cmd(0xF0);
		S6D02A1_wr_dat(0x5A);
		S6D02A1_wr_dat(0x5A);
		
		S6D02A1_wr_cmd(0xFC);
		S6D02A1_wr_dat(0x5A);
		S6D02A1_wr_dat(0x5A);
		
		S6D02A1_wr_cmd(0x26);
		S6D02A1_wr_dat(0x01);
		
		S6D02A1_wr_cmd(0xFA);
		S6D02A1_wr_dat(0x02);
		S6D02A1_wr_dat(0x1F);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x10);
		S6D02A1_wr_dat(0x22);
		S6D02A1_wr_dat(0x30);
		S6D02A1_wr_dat(0x38);
		S6D02A1_wr_dat(0x3A);
		S6D02A1_wr_dat(0x3A);
		S6D02A1_wr_dat(0x3A);
		S6D02A1_wr_dat(0x3A);
		S6D02A1_wr_dat(0x3A);
		S6D02A1_wr_dat(0x3D);
		S6D02A1_wr_dat(0x02);
		S6D02A1_wr_dat(0x01);
		
		S6D02A1_wr_cmd(0xFB);
		S6D02A1_wr_dat(0x21);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x02);
		S6D02A1_wr_dat(0x04);
		S6D02A1_wr_dat(0x07);
		S6D02A1_wr_dat(0x0A);
		S6D02A1_wr_dat(0x0B);
		S6D02A1_wr_dat(0x0C);
		S6D02A1_wr_dat(0x0C);
		S6D02A1_wr_dat(0x16);
		S6D02A1_wr_dat(0x1E);
		S6D02A1_wr_dat(0x30);
		S6D02A1_wr_dat(0x3F);
		S6D02A1_wr_dat(0x01);
		S6D02A1_wr_dat(0x02);
		
		/* power setting sequence */
		S6D02A1_wr_cmd(0xFD);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x17);
		S6D02A1_wr_dat(0x10);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x01);
		S6D02A1_wr_dat(0x01);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x1F);
		S6D02A1_wr_dat(0x1F);
		
		S6D02A1_wr_cmd(0xF4);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x3F);
		S6D02A1_wr_dat(0x3F);
		S6D02A1_wr_dat(0x07);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x3C);
		S6D02A1_wr_dat(0x36);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x3C);
		S6D02A1_wr_dat(0x36);
		S6D02A1_wr_dat(0x00);
		
		S6D02A1_wr_cmd(0xF5);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x70);
		S6D02A1_wr_dat(0x66);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x6D);
		S6D02A1_wr_dat(0x66);
		S6D02A1_wr_dat(0x06);
		
		S6D02A1_wr_cmd(0xF6);
		S6D02A1_wr_dat(0x02);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x3F);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x02);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x06);
		S6D02A1_wr_dat(0x01);
		S6D02A1_wr_dat(0x00);
		
		S6D02A1_wr_cmd(0xF2);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x01);
		S6D02A1_wr_dat(0x03);
		S6D02A1_wr_dat(0x08);
		S6D02A1_wr_dat(0x08);
		S6D02A1_wr_dat(0x04);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x01);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x04);
		S6D02A1_wr_dat(0x08);
		S6D02A1_wr_dat(0x08);
		
		S6D02A1_wr_cmd(0xF8);
		S6D02A1_wr_dat(0x11);
		
		S6D02A1_wr_cmd(0xF7);
		S6D02A1_wr_dat(0xC8);
		S6D02A1_wr_dat(0x20);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		
		S6D02A1_wr_cmd(0xF3);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		
		S6D02A1_wr_cmd(0x11);
		_delay_ms(50);
		
		S6D02A1_wr_cmd(0xF3);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x01);
		_delay_ms(50);
		
		S6D02A1_wr_cmd(0xF3);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x03);
		_delay_ms(50);
		
		S6D02A1_wr_cmd(0xF3);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x07);
		_delay_ms(50);
		
		S6D02A1_wr_cmd(0xF3);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x0F);
		_delay_ms(50);
		
		S6D02A1_wr_cmd(0xF4);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x04);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x3F);
		S6D02A1_wr_dat(0x3F);
		S6D02A1_wr_dat(0x07);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x3C);
		S6D02A1_wr_dat(0x36);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x3C);
		S6D02A1_wr_dat(0x36);
		S6D02A1_wr_dat(0x00);
		_delay_ms(50);
		
		S6D02A1_wr_cmd(0xF3);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x1F);
		_delay_ms(50);
		
		S6D02A1_wr_cmd(0xF3);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x7F);
		_delay_ms(50);
		
		S6D02A1_wr_cmd(0xF3);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0xFF);
		_delay_ms(50);
		
		S6D02A1_wr_cmd(0xFD);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x17);
		S6D02A1_wr_dat(0x10);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x01);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x16);
		S6D02A1_wr_dat(0x16);
		
		S6D02A1_wr_cmd(0xF4);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x09);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x3F);
		S6D02A1_wr_dat(0x3F);
		S6D02A1_wr_dat(0x07);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x3C);
		S6D02A1_wr_dat(0x36);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_dat(0x3C);
		S6D02A1_wr_dat(0x36);
		S6D02A1_wr_dat(0x00);
		
		S6D02A1_wr_cmd(0x36);   
		S6D02A1_wr_dat(1<<6|1<<7);
		
		S6D02A1_wr_cmd(0x35);
		S6D02A1_wr_dat(0x00);
		S6D02A1_wr_cmd(0x3A);
		S6D02A1_wr_dat(0x05);
		
		_delay_ms(150);
		S6D02A1_wr_cmd(0x29);
		S6D02A1_wr_cmd(0x2C);
	}

	else { for(;;);} /* Invalid Device Code!! */

	S6D02A1_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	S6D02A1_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		S6D02A1_wr_gram(COL_RED);
	} while (--n);


	_delay_ms(500);
	for(;;);

#endif

}


/* End Of File ---------------------------------------------------------------*/
