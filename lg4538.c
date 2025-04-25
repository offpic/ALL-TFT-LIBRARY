/********************************************************************************/
/*!
	@file			lg4538.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				 	 @n
					It can drive SF-TC240G-9366A-N	TFT module(8/16bit & spi mode).

    @section HISTORY
		2011.11.10	V1.00	First Release.
		2014.10.15	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "lg4538.h"
/* check header file version for fool proof */
#if LG4538_H != 0x0400
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
inline void LG4538_reset(void)
{
#ifdef USE_LG4538_TFT
	LG4538_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	LG4538_RD_SET();
	LG4538_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	LG4538_RES_CLR();							/* RES=L, CS=L   			*/
	LG4538_CS_CLR();

#elif  USE_LG4538_SPI_TFT
	LG4538_RES_SET();							/* RES=H, CS=H				*/
	LG4538_CS_SET();
	LG4538_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	LG4538_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(10);								/* wait 10ms     			*/
	LG4538_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait over 120ms     		*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_LG4538_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void LG4538_wr_cmd(uint8_t cmd)
{
	LG4538_DC_CLR();						/* DC=L						*/

	LG4538_CMD = cmd;						/* cmd(8bit)				*/
	LG4538_WR();							/* WR=L->H					*/

	LG4538_DC_SET();						/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void LG4538_wr_dat(uint8_t dat)
{
	LG4538_DATA = dat;						/* data(8bit_Low or 16bit)	*/
	LG4538_WR();							/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void LG4538_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	LG4538_DATA = (uint8_t)(gram>>8);		/* upper 8bit data			*/
	LG4538_WR();							/* WR=L->H					*/
	LG4538_DATA = (uint8_t)gram;			/* lower 8bit data			*/
#else
	LG4538_DATA = gram;						/* 16bit data 				*/
#endif
	LG4538_WR();							/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void LG4538_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		LG4538_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		LG4538_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t LG4538_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	LG4538_wr_cmd(cmd);
	LG4538_WR_SET();

    ReadLCDData(temp);						/* Dummy Read(Invalid Data) */
    ReadLCDData(temp);						/* Dummy Read				*/
    ReadLCDData(temp);						/* Upper Read				*/
    ReadLCDData(val);						/* Lower Read				*/

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}


#elif USE_LG4538_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void LG4538_wr_cmd(uint8_t cmd)
{
	LG4538_DC_CLR();						/* DC=L			 */
	DISPLAY_ASSART_CS();					/* CS=L		     */

#ifdef LG4538SPI_3WIREMODE
	SendSPI(START_WR_CMD);
#endif
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();					/* CS=H		     */
	LG4538_DC_SET();						/* DC=H			 */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void LG4538_wr_dat(uint8_t dat)
{	
	DISPLAY_ASSART_CS();					/* CS=L		     */

#ifdef LG4538SPI_3WIREMODE
	SendSPI(START_WR_DATA);
#endif
	SendSPI(dat);

	DISPLAY_NEGATE_CS();					/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM Data.
*/
/**************************************************************************/
inline void LG4538_wr_gram(uint16_t gram)
{	
	DISPLAY_ASSART_CS();					/* CS=L		     */

#ifdef LG4538SPI_3WIREMODE
	SendSPI(START_WR_DATA);
#endif
	SendSPI16(gram);

	DISPLAY_NEGATE_CS();					/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void LG4538_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */
#ifdef LG4538SPI_3WIREMODE
	SendSPI(START_WR_DATA);
#endif

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt );
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

	DISPLAY_NEGATE_CS();					/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t LG4538_rd_cmd(uint8_t cmd)
{
#if 0
	uint16_t val;
	uint16_t temp;

	DISPLAY_ASSART_CS();					/* CS=L		    */
	LG4538_DC_CLR();						/* DC=L			*/

	SendSPI(cmd);

	LG4538_DC_SET();						/* DC=H			*/
	temp = RecvSPI();						/* Dummy Read 	*/
	temp = RecvSPI();						/* Dummy Read 	*/
	temp = RecvSPI();						/* Upper Read 	*/
	val  = RecvSPI();						/* Lower Read 	*/

	val &= 0x00FF;
	val |= temp<<8;

	DISPLAY_NEGATE_CS();					/* CS=H		    */

	return val;
#else

	/* SF-TC240G-9366A-N has NO SDO Pin... */
	return 0x4538;
#endif
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void LG4538_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	LG4538_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	LG4538_wr_dat((OFS_COL + x)>>8);
	LG4538_wr_dat(OFS_COL + x);
	LG4538_wr_dat((OFS_COL + width)>>8);
	LG4538_wr_dat(OFS_COL + width);

	LG4538_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	LG4538_wr_dat((OFS_RAW + y)>>8);
	LG4538_wr_dat(OFS_RAW + y);
	LG4538_wr_dat((OFS_RAW + height)>>8);
	LG4538_wr_dat(OFS_RAW + height);

	LG4538_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void LG4538_clear(void)
{
	volatile uint32_t n;

	LG4538_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		LG4538_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void LG4538_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	LG4538_reset();

	/* Check Device Code */
	devicetype = LG4538_rd_cmd(0xA1);  		/* Confirm Vaild LCD Controller */

	if(devicetype == 0x4538)
	{
		/* Initialize LG4538 */
		LG4538_wr_cmd(0x36);				/* MADCTRL */
		LG4538_wr_dat((0<<7)|(0<<6)|(1<<3)|(1<<1));
		
		LG4538_wr_cmd(0xB3);
		LG4538_wr_dat((0<<7)|(0<<6)|(0<<0));
		
		LG4538_wr_cmd(0x3A);
		LG4538_wr_dat(0x55);			/* 66=18BIT;55=16BIT */
		
		LG4538_wr_cmd(0x11);
		_delay_ms(40);
		
		LG4538_wr_cmd(0x29);
	}

	else { for(;;);} /* Invalid Device Code!! */

	LG4538_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	LG4538_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		LG4538_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
