/********************************************************************************/
/*!
	@file			hx8340bn.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive BTL221722-276L TFT module(3-Wire,9bitSerial only).

    @section HISTORY
		2011.12.01	V1.00	Stable Release.
		2012.05.25  V2.00	Added Hardware 9-bitSerial Handling.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8340bn.h"
/* check header file version for fool proof */
#if HX8340BN_H != 0x0400
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
inline void HX8340BN_reset(void)
{
#ifdef USE_HX8340BN_TFT
	HX8340BN_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	HX8340BN_RD_SET();
	HX8340BN_WR_SET();
	_delay_ms(5);								/* wait 5ms     			*/

	HX8340BN_RES_CLR();							/* RES=L, CS=L   			*/
	HX8340BN_CS_CLR();

#elif  USE_HX8340BN_SPI_TFT
	HX8340BN_RES_SET();							/* RES=H, CS=H				*/
	HX8340BN_CS_SET();
	HX8340BN_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(5);								/* wait 5ms     			*/

	HX8340BN_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(10);								/* wait 10ms     			*/
	HX8340BN_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait at least 120ms     	*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_HX8340BN_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8340BN_wr_cmd(uint8_t cmd)
{
	HX8340BN_DC_CLR();							/* DC=L						*/
	
	HX8340BN_CMD = cmd;							/* cmd(8bit)				*/
	HX8340BN_WR();								/* WR=L->H					*/
	
	HX8340BN_DC_SET();							/* DC=H 					*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void HX8340BN_wr_dat(uint8_t dat)
{
	HX8340BN_DATA = dat;						/* data(8bit_Low or 16bit)	*/
	HX8340BN_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8340BN_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8340BN_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	HX8340BN_WR();								/* WR=L->H					*/
	HX8340BN_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	HX8340BN_DATA = gram;						/* 16bit data 				*/
#endif
	HX8340BN_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8340BN_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		HX8340BN_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		HX8340BN_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t HX8340BN_rd_cmd(uint8_t cmd)
{
	uint16_t val;

	HX8340BN_wr_cmd(cmd);
	HX8340BN_WR_SET();

    ReadLCDData(val);

	val &= 0x00FF;

	return (uint8_t)val;
}


#elif USE_HX8340BN_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8340BN_wr_cmd(uint8_t cmd)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */

	DNC_CMD();
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
static inline void HX8340BN_wr_sdat(uint8_t dat)
{	
	DNC_DAT();
#if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
	SendSPID(dat);
#else
	SendSPI(dat);
#endif
}
inline void HX8340BN_wr_dat(uint8_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	HX8340BN_wr_sdat(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8340BN_wr_gram(uint16_t gram)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	HX8340BN_wr_sdat((uint8_t)(gram>>8));
	HX8340BN_wr_sdat((uint8_t)gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8340BN_wr_block(uint8_t *p,unsigned int cnt)
{
	int n;
	
	n = cnt % 4;
	cnt /= 4;

	DISPLAY_ASSART_CS();						/* CS=L		     */

	while (cnt--) {
		HX8340BN_wr_sdat(*p++);
		HX8340BN_wr_sdat(*p++);
		HX8340BN_wr_sdat(*p++);
		HX8340BN_wr_sdat(*p++);
	}
	while (n--) {
		HX8340BN_wr_sdat(*p++);
	}

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline  uint8_t HX8340BN_rd_cmd(uint8_t cmd)
{
#if 0
	uint8_t val;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI(cmd);
	
	SendSPI(START_RD_DATA);
	val = RecvSPI();

	DISPLAY_NEGATE_CS();						/* CS=H		     */
#endif 
	return 0x49;								/* Read Function Does not support... */
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8340BN_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	HX8340BN_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	HX8340BN_wr_dat((OFS_COL + x)>>8);
	HX8340BN_wr_dat(OFS_COL + x);
	HX8340BN_wr_dat((OFS_COL + width)>>8);
	HX8340BN_wr_dat(OFS_COL + width);

	HX8340BN_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	HX8340BN_wr_dat((OFS_RAW + y)>>8);
	HX8340BN_wr_dat(OFS_RAW + y);
	HX8340BN_wr_dat((OFS_RAW + height)>>8);
	HX8340BN_wr_dat(OFS_RAW + height);

	HX8340BN_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8340BN_clear(void)
{
	volatile uint32_t n;

	HX8340BN_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8340BN_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8340BN_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	HX8340BN_reset();

	/* Check Device Code */
	devicetype = HX8340BN_rd_cmd(0x93);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x49)
	{
		/* Initialize HX8340BN */
		/* Driving ability Setting */
		HX8340BN_wr_cmd(0xC1); 
		HX8340BN_wr_dat(0xFF);
		HX8340BN_wr_dat(0x83);
		HX8340BN_wr_dat(0x40); 
		HX8340BN_wr_cmd(0x11); 
		_delay_ms(100);
		HX8340BN_wr_cmd(0xCA); 
		HX8340BN_wr_dat(0x70);
		HX8340BN_wr_dat(0x00);
		HX8340BN_wr_dat(0xD9); 
		HX8340BN_wr_dat(0x01);
		HX8340BN_wr_dat(0x11); 

		HX8340BN_wr_cmd(0xC9); 
		HX8340BN_wr_dat(0x90);
		HX8340BN_wr_dat(0x49);
		HX8340BN_wr_dat(0x10); 
		HX8340BN_wr_dat(0x28);
		HX8340BN_wr_dat(0x28); 
		HX8340BN_wr_dat(0x10); 
		HX8340BN_wr_dat(0x00); 
		HX8340BN_wr_dat(0x06);
		_delay_ms(20);
		HX8340BN_wr_cmd(0xC2); 
		HX8340BN_wr_dat(0x60);
		HX8340BN_wr_dat(0x71);
		HX8340BN_wr_dat(0x01); 
		HX8340BN_wr_dat(0x0E);
		HX8340BN_wr_dat(0x05); 
		HX8340BN_wr_dat(0x02); 
		HX8340BN_wr_dat(0x09); 
		HX8340BN_wr_dat(0x31);
		HX8340BN_wr_dat(0x0A);

		HX8340BN_wr_cmd(0xC3); 
		HX8340BN_wr_dat(0x67);
		HX8340BN_wr_dat(0x30);
		HX8340BN_wr_dat(0x61); 
		HX8340BN_wr_dat(0x17);
		HX8340BN_wr_dat(0x48); 
		HX8340BN_wr_dat(0x07); 
		HX8340BN_wr_dat(0x05); 
		HX8340BN_wr_dat(0x33); 
		_delay_ms(10);
		HX8340BN_wr_cmd(0xB5); 
		HX8340BN_wr_dat(0x35);
		HX8340BN_wr_dat(0x20);
		HX8340BN_wr_dat(0x45); 

		HX8340BN_wr_cmd(0xB4); 
		HX8340BN_wr_dat(0x33);
		HX8340BN_wr_dat(0x25);
		HX8340BN_wr_dat(0x4C); 
		_delay_ms(10);
		HX8340BN_wr_cmd(0x3A); 
		HX8340BN_wr_dat(0x05);
		HX8340BN_wr_cmd(0x29); 
		_delay_ms(10);

	}

	else { for(;;);} /* Invalid Device Code!! */

	HX8340BN_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8340BN_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		HX8340BN_wr_gram(COL_RED);
	} while (--n);

	_delay_ms(500);
	for(;;);

#endif

}


/* End Of File ---------------------------------------------------------------*/
