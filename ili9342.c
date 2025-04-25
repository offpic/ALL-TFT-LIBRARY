/********************************************************************************/
/*!
	@file			ili9342.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -XYL62291B-2B		(ILI9342)	8/16bit mode.				@n
					 -YB020C01-40		(ILI9342C)	3-wire/9-bit serial only!

    @section HISTORY
		2013.01.02	V1.00	Stable Release
		2016.11.04	V2.00	Added 3-Wire/9bit Serial Support.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili9342.h"
/* check header file version for fool proof */
#if ILI9342_H != 0x0400
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#if defined(USE_YB020C01_40)
 #warning "You Select USE_YB020C01_40 Module(9-bit serial)!"
#elif  defined(USE_XYL62291B_2B)
 #warning "You Select USE_XYL62291B_2B Module(8/16bit parallel)!"
#else
 #error "U MUST select LCD Molule Model at first!."
#endif

#ifdef  USE_ILI9342_SPI_TFT
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
inline void ILI9342_reset(void)
{
#ifdef USE_ILI9342_TFT
	ILI9342_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI9342_RD_SET();
	ILI9342_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9342_RES_CLR();							/* RES=L, CS=L   			*/
	ILI9342_CS_CLR();

#elif  USE_ILI9342_SPI_TFT
	ILI9342_RES_SET();							/* RES=H, CS=H				*/
	ILI9342_CS_SET();
	ILI9342_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9342_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(10);								/* wait 10ms     			*/
	ILI9342_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait over 120ms     		*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_ILI9342_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9342_wr_cmd(uint8_t cmd)
{
	ILI9342_DC_CLR();							/* DC=L						*/

	ILI9342_CMD = cmd;							/* cmd(8bit)				*/
	ILI9342_WR();								/* WR=L->H					*/

	ILI9342_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void ILI9342_wr_dat(uint8_t dat)
{
	ILI9342_DATA = dat;							/* data(8bit)				*/
	ILI9342_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ILI9342_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9342_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	ILI9342_WR();								/* WR=L->H					*/
	ILI9342_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	ILI9342_DATA = gram;						/* 16bit data 				*/
#endif
	ILI9342_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9342_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;

	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI9342_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		ILI9342_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI9342_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	ILI9342_wr_cmd(cmd);
	ILI9342_WR_SET();

    ReadLCDData(temp);							/* Dummy Read(Invalid Data) */
    ReadLCDData(temp);							/* Dummy Read				*/
    ReadLCDData(temp);							/* Upper Read				*/
    ReadLCDData(val);							/* Lower Read				*/

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}


#elif USE_ILI9342_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9342_wr_cmd(uint8_t cmd)
{
	ILI9342_DC_CLR();							/* DC=L			 */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	DNC_CMD();
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	ILI9342_DC_SET();							/* DC=H			 */
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
static inline void ILI9342_wr_sdat(uint8_t dat)
{	
	DNC_DAT();
#if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
	SendSPID(dat);
#else
	SendSPI(dat);
#endif
}
inline void ILI9342_wr_dat(uint8_t dat)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	ILI9342_wr_sdat(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ILI9342_wr_gram(uint16_t gram)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	ILI9342_wr_sdat((uint8_t)(gram>>8));
	ILI9342_wr_sdat((uint8_t)gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9342_wr_block(uint8_t *p,unsigned int cnt)
{
	int n;
	
	n = cnt % 4;
	cnt /= 4;

	DISPLAY_ASSART_CS();						/* CS=L		     */

	while (cnt--) {
		ILI9342_wr_sdat(*p++);
		ILI9342_wr_sdat(*p++);
		ILI9342_wr_sdat(*p++);
		ILI9342_wr_sdat(*p++);
	}
	while (n--) {
		ILI9342_wr_sdat(*p++);
	}

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI9342_rd_cmd(uint8_t cmd)
{
	uint16_t val;

	DISPLAY_ASSART_CS();						/* CS=L		     */

	DNC_CMD();
	SendSPI(cmd);
	
	Display_ChangeSDA_If(TFT_SDA_READ);
	val = 	RecvSPI();
	Display_ChangeSDA_If(TFT_SDA_WRITE);

	DISPLAY_NEGATE_CS();						/* CS=H		    */

	return val;
}

/**************************************************************************/
/*! 
    Read ID ILI9342.
*/
/**************************************************************************/
static uint16_t ILI9342_rd_id(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	ILI9342_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI9342_wr_dat(0x10);    					/* Read Mode Enable,1st Byte */
	temp = ILI9342_rd_cmd(cmd);					/* Dummy Read 	*/

	ILI9342_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI9342_wr_dat(0x11);    					/* Read Mode Enable,2nd Byte */
	temp = ILI9342_rd_cmd(cmd);					/* Dummy Read 	*/
	
	ILI9342_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI9342_wr_dat(0x12);    					/* Read Mode Enable,3rd Byte */
	temp = ILI9342_rd_cmd(cmd);					/* Upper Read 	*/

	ILI9342_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI9342_wr_dat(0x13);    					/* Read Mode Enable,4th Byte */
	val = ILI9342_rd_cmd(cmd);					/* Lower Read 	*/

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;

	return val;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ILI9342_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ILI9342_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	ILI9342_wr_dat((OFS_COL + x)>>8);
	ILI9342_wr_dat(OFS_COL + x);
	ILI9342_wr_dat((OFS_COL + width)>>8);
	ILI9342_wr_dat(OFS_COL + width);

	ILI9342_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	ILI9342_wr_dat((OFS_RAW + y)>>8);
	ILI9342_wr_dat(OFS_RAW + y);
	ILI9342_wr_dat((OFS_RAW + height)>>8);
	ILI9342_wr_dat(OFS_RAW + height);

	ILI9342_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI9342_clear(void)
{
	volatile uint32_t n;

	ILI9342_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9342_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ILI9342_init(void)
{
	uint16_t ili9342_id,ili9342c_id;

	Display_IoInit_If();

	ILI9342_reset();

	/* Check Device Code */
#if defined(USE_ILI9342_SPI_TFT)
	/* Set Manufacture command for ILI9342 */
	ILI9342_wr_cmd(0xB9);
	ILI9342_wr_dat(0xFF);
	ILI9342_wr_dat(0x93);
	ILI9342_wr_dat(0x42);
	ili9342_id = ILI9342_rd_id(0xD3);  		/* Confirm Vaild LCD Controller */

	/* Set Manufacture command for ILI9342C */
	ILI9342_wr_cmd(0xC8);
	ILI9342_wr_dat(0xFF);
	ILI9342_wr_dat(0x93);
	ILI9342_wr_dat(0x42);
	ili9342c_id = ILI9342_rd_id(0xD3);  		/* Confirm Vaild LCD Controller */
#else
	/* Set Manufacture command for ILI9342 */
	ILI9342_wr_cmd(0xB9);
	ILI9342_wr_dat(0xFF);
	ILI9342_wr_dat(0x93);
	ILI9342_wr_dat(0x42);
	ili9342_id = ILI9342_rd_cmd(0xD3);  	/* Confirm Vaild LCD Controller */

	/* Set Manufacture command for ILI9342C */
	ILI9342_wr_cmd(0xC8);
	ILI9342_wr_dat(0xFF);
	ILI9342_wr_dat(0x93);
	ILI9342_wr_dat(0x42);
	ili9342c_id = ILI9342_rd_cmd(0xD3);  	/* Confirm Vaild LCD Controller */
#endif

	if((ili9342_id == 0x9342) || (ili9342c_id != 0x9342))
	{
		/* Initialize ILI9342 */
		ILI9342_wr_cmd(0xB9);		/* Set Manufacture command for ILI9342 */
		ILI9342_wr_dat(0xFF);
		ILI9342_wr_dat(0x93);
		ILI9342_wr_dat(0x42);

		ILI9342_wr_cmd(0x21);
		ILI9342_wr_cmd(0x36);
		ILI9342_wr_dat(0xC8);
		
		ILI9342_wr_cmd(0xC0);
		ILI9342_wr_dat(0x28);
		ILI9342_wr_dat(0x0A);
		
		ILI9342_wr_cmd(0xC1);
		ILI9342_wr_dat(0x02);
		
		ILI9342_wr_cmd(0xC5);
		ILI9342_wr_dat(0x2F);
		ILI9342_wr_dat(0x2F);
		
		ILI9342_wr_cmd(0xC7);
		ILI9342_wr_dat(0xC3);
		
		ILI9342_wr_cmd(0xB8);
		ILI9342_wr_dat(0x0B);
		
		ILI9342_wr_cmd(0xE0);
		ILI9342_wr_dat(0x0F);
		ILI9342_wr_dat(0x22);
		ILI9342_wr_dat(0x1D);
		ILI9342_wr_dat(0x0B);
		ILI9342_wr_dat(0x0F);
		ILI9342_wr_dat(0x07);
		ILI9342_wr_dat(0x4C);
		ILI9342_wr_dat(0x76);
		ILI9342_wr_dat(0x3C);
		ILI9342_wr_dat(0x09);
		ILI9342_wr_dat(0x16);
		ILI9342_wr_dat(0x07);
		ILI9342_wr_dat(0x12);
		ILI9342_wr_dat(0x0B);
		ILI9342_wr_dat(0x08);
		
		ILI9342_wr_cmd(0xE1);
		ILI9342_wr_dat(0x08);
		ILI9342_wr_dat(0x1F);
		ILI9342_wr_dat(0x24);
		ILI9342_wr_dat(0x03);
		ILI9342_wr_dat(0x0E);
		ILI9342_wr_dat(0x03);
		ILI9342_wr_dat(0x35);
		ILI9342_wr_dat(0x23);
		ILI9342_wr_dat(0x45);
		ILI9342_wr_dat(0x01);
		ILI9342_wr_dat(0x0B);
		ILI9342_wr_dat(0x07);
		ILI9342_wr_dat(0x2F);
		ILI9342_wr_dat(0x36);
		ILI9342_wr_dat(0x0F);

		ILI9342_wr_cmd(0x11);		/* Exit Sleep */
		_delay_ms(10);
		ILI9342_wr_cmd(0x11);		/* Exit Sleep */
		_delay_ms(80);

		ILI9342_wr_cmd(0x3A);		/* SET 65K COLOR */
		ILI9342_wr_dat(0x55);

		ILI9342_wr_cmd(0xB0);		/* SET EPL,DPL,VSL,HSL */
		ILI9342_wr_dat(0xE0);

		ILI9342_wr_cmd(0xF6);		/* SET Interface Control */
		ILI9342_wr_dat(0x01);
		ILI9342_wr_dat(0x00);
		ILI9342_wr_dat(0x00);

		ILI9342_wr_cmd(0x29);		/* Display ON */
	}
	
	else if((ili9342_id != 0x9342) || (ili9342c_id == 0x9342))
	{
		/* Initialize ILI9342C */
		ILI9342_wr_cmd(0xC8);		/* Set Manufacture command for ILI9342C */
		ILI9342_wr_dat(0xFF);
		ILI9342_wr_dat(0x93);
		ILI9342_wr_dat(0x42);

		ILI9342_wr_cmd(0xC0);
		ILI9342_wr_dat(0x0F);
		ILI9342_wr_dat(0x0F);
		
		ILI9342_wr_cmd(0xC1);
		ILI9342_wr_dat(0x01);
		
		ILI9342_wr_cmd(0xC5);
		ILI9342_wr_dat(0xDB);

		ILI9342_wr_cmd(0x36);
		ILI9342_wr_dat(0xC8);

		ILI9342_wr_cmd(0x3A);		/* SET 65K COLOR */
		ILI9342_wr_dat(0x55);

		ILI9342_wr_cmd(0xB0);
		ILI9342_wr_dat(0xE0);

		ILI9342_wr_cmd(0xB4);
		ILI9342_wr_dat(0x02);

		ILI9342_wr_cmd(0xB7);
		ILI9342_wr_dat(0x07);

		ILI9342_wr_cmd(0xF6);		/* SET Interface Control */
		ILI9342_wr_dat(0x01);
		ILI9342_wr_dat(0x00);
		ILI9342_wr_dat(0x00);

		ILI9342_wr_cmd(0xE0);
		ILI9342_wr_dat(0x00);
		ILI9342_wr_dat(0x05);
		ILI9342_wr_dat(0x08);
		ILI9342_wr_dat(0x02);
		ILI9342_wr_dat(0x1A);
		ILI9342_wr_dat(0x0C);
		ILI9342_wr_dat(0x42);
		ILI9342_wr_dat(0x7A);
		ILI9342_wr_dat(0x54);
		ILI9342_wr_dat(0x08);
		ILI9342_wr_dat(0x0D);
		ILI9342_wr_dat(0x0C);
		ILI9342_wr_dat(0x23);
		ILI9342_wr_dat(0x25);
		ILI9342_wr_dat(0x0F);
		
		ILI9342_wr_cmd(0xE1);
		ILI9342_wr_dat(0x00);
		ILI9342_wr_dat(0x29);
		ILI9342_wr_dat(0x2F);
		ILI9342_wr_dat(0x03);
		ILI9342_wr_dat(0x0F);
		ILI9342_wr_dat(0x05);
		ILI9342_wr_dat(0x42);
		ILI9342_wr_dat(0x55);
		ILI9342_wr_dat(0x53);
		ILI9342_wr_dat(0x06);
		ILI9342_wr_dat(0x0F);
		ILI9342_wr_dat(0x0C);
		ILI9342_wr_dat(0x38);
		ILI9342_wr_dat(0x3A);
		ILI9342_wr_dat(0x0F);

		ILI9342_wr_cmd(0x11);		/* Exit Sleep */
		_delay_ms(120);

		ILI9342_wr_cmd(0x29);		/* Display ON */
	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI9342_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ILI9342_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9342_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
