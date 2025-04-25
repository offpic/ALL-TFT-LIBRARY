/********************************************************************************/
/*!
	@file			ili9341_rgb.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				 	 	 @n
					Available TFT-LCM are listed below.							 	 	 @n
					 -STM32F429I-Discovery		(ILI9341)	4-Wire,8bitSerial-1 and
															RGB-Interface

    @section HISTORY
		2014.03.11	V1.00	First Release.
		2014.05.29  V2.00	Fixed ILI9341 Initialization.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili9341_rgb.h"
/* check header file version for fool proof */
#if ILI9341_RGB_H != 0x0400
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
inline void ILI934x_reset(void)
{
	ILI934x_RES_SET();							/* RES=H, CS=H				*/
	ILI934x_CS_SET();
	ILI934x_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	ILI934x_RES_CLR();							/* RES=L		   			*/

	_delay_ms(10);								/* wait 10ms     			*/
	ILI934x_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait over 120ms     		*/
}

/* SPI Control Only */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI934x_wr_cmd(uint8_t cmd)
{
	ILI934x_DC_CLR();							/* DC=L			 */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	ILI934x_DC_SET();							/* DC=H			 */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void ILI934x_wr_dat(uint8_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM Data.
*/
/**************************************************************************/
inline void ILI934x_wr_gram(uint16_t gram)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI16(gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI934x_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */

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

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}


/**************************************************************************/
/*! 
    Read ID ILI934x.
*/
/**************************************************************************/
static uint16_t ILI934x_rd_id(uint8_t cmd)
{
#ifndef USE_32F429IDISCOVERY
	uint16_t val;
	uint16_t temp;

	ILI934x_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI934x_wr_dat(0x11);    					/* Read Mode Enable,1st Byte */
	temp = ILI934x_rd_cmd(cmd);					/* Dummy Read 	*/
	
	ILI934x_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI934x_wr_dat(0x12);    					/* Read Mode Enable,2nd Byte */
	temp = ILI934x_rd_cmd(cmd);					/* Upper Read 	*/
	
	ILI934x_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI934x_wr_dat(0x13);    					/* Read Mode Enable,3rd Byte */
	val  = ILI934x_rd_cmd(cmd);					/* Lower Read	*/
		   ILI934x_rd_cmd(cmd);					/* Dummy Read 	*/

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;

	return val;
#else
	return 0x9341;
#endif
}


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ILI934x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ILI934x_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	ILI934x_wr_dat((OFS_COL + x)>>8);
	ILI934x_wr_dat(OFS_COL + x);
	ILI934x_wr_dat((OFS_COL + width)>>8);
	ILI934x_wr_dat(OFS_COL + width);

	ILI934x_wr_cmd(0x2B);				/* Vertical RAM Start ADDR */
	ILI934x_wr_dat((OFS_RAW + y)>>8);
	ILI934x_wr_dat(OFS_RAW + y);
	ILI934x_wr_dat((OFS_RAW + height)>>8);
	ILI934x_wr_dat(OFS_RAW + height);

	ILI934x_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI934x_clear(void)
{
	volatile uint32_t n;

	ILI934x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI934x_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ILI934x_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	ILI934x_reset();

	/* Check Device Code */
	devicetype = ILI934x_rd_id(0xD3);	/* Confirm Vaild LCD Controller Serial Interface */

	if(devicetype == 0x9341)
	{
		/* Initialize ILI9341 */
		ILI934x_wr_cmd(0xCA); 			/*  ILI9341 Internal Timings Fine Adjustments */
		ILI934x_wr_dat(0xC3);
		ILI934x_wr_dat(0x08);
		ILI934x_wr_dat(0x50);
		
		ILI934x_wr_cmd(0xEF); 			/*  ILI9341 Internal Timings Fine Adjustments */
		ILI934x_wr_dat(0x03);
		ILI934x_wr_dat(0x80);
		ILI934x_wr_dat(0x02);
		
		ILI934x_wr_cmd(0xCF);			/* Power control B register */
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0xC1);
		ILI934x_wr_dat(0x30);
		
		ILI934x_wr_cmd(0xED);			/* Power on sequence register */
		ILI934x_wr_dat(0x67);
		ILI934x_wr_dat(0x03);
		ILI934x_wr_dat(0x12);
		ILI934x_wr_dat(0x81);
		
		ILI934x_wr_cmd(0xE8);			/* Driver timing control A */
		ILI934x_wr_dat(0x85); 
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x78);
		
		ILI934x_wr_cmd(0xCB); 			/* Power control A register */
		ILI934x_wr_dat(0x39);
		ILI934x_wr_dat(0x2C);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x34);
		ILI934x_wr_dat(0x02);
		
		ILI934x_wr_cmd(0xF7);			/* Pump ratio control register */
		ILI934x_wr_dat(0x20);
		
		ILI934x_wr_cmd(0xEA); 			/* Driver timing control B */
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0xB1);			/* Frame Rate Control register */
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x1B);
		
		ILI934x_wr_cmd(0xC0);			/* Power control 1 */
		ILI934x_wr_dat(0x10);
		
		ILI934x_wr_cmd(0xC1);			/* Power control 2 */
		ILI934x_wr_dat(0x10);
		
		ILI934x_wr_cmd(0xC5);			/* Vcomh & Vcoml control */
		ILI934x_wr_dat(0x45);
		ILI934x_wr_dat(0x15);
		
		ILI934x_wr_cmd(0xC7);			/* vcom adjust control */
		ILI934x_wr_dat(0x90);
		
		ILI934x_wr_cmd(0x36);			/* Memory Access Control */
		ILI934x_wr_dat(0xC8);
		
		ILI934x_wr_cmd(0xF2);			/* 3Gamma Function Disable */ 
        ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0xB0);			/* RGB Interface control */
        ILI934x_wr_dat(0xC2);
		
		ILI934x_wr_cmd(0xB6);			/* Display Function Control register */
		ILI934x_wr_dat(0x0A);
		ILI934x_wr_dat(0xA7);
		ILI934x_wr_dat(0x27);
		ILI934x_wr_dat(0x04);
		
		ILI934x_wr_cmd(0x3A); 			/* RGB & CPU 18bit 0x66 / 16bit 0x55 */
        ILI934x_wr_dat(0x55);
		
		ILI934x_wr_cmd(0x2A);			/* Row Address */
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0xEF);
		
		ILI934x_wr_cmd(0x3B);			/* Column Address */
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x01);
		ILI934x_wr_dat(0x3F);

		ILI934x_wr_cmd(0xF6);			/* Interface control */
        ILI934x_wr_dat(0x01);			/* Select RGB + VSYNC Interface */
        ILI934x_wr_dat(0x00);
        ILI934x_wr_dat(0x06);
		
		ILI934x_wr_cmd(0xB4); 			/* Inversion Nomal */          
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0x26); 			/* Gamma select G2.2 */
		ILI934x_wr_dat(0x01);
		
		ILI934x_wr_cmd(0xE0);			/* Positive  gamma */
		ILI934x_wr_dat(0x0F);
		ILI934x_wr_dat(0x29);
		ILI934x_wr_dat(0x24);
		ILI934x_wr_dat(0x0C);
		ILI934x_wr_dat(0x0E);
		ILI934x_wr_dat(0x09);
		ILI934x_wr_dat(0x4E);
		ILI934x_wr_dat(0x78);
		ILI934x_wr_dat(0x3C);
		ILI934x_wr_dat(0x09);
		ILI934x_wr_dat(0x13);
		ILI934x_wr_dat(0x05);
		ILI934x_wr_dat(0x17);
		ILI934x_wr_dat(0x11);
		ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0xE1);			/* Negative  gamma */
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x16);
		ILI934x_wr_dat(0x1B);
		ILI934x_wr_dat(0x04);
		ILI934x_wr_dat(0x11);
		ILI934x_wr_dat(0x07);
		ILI934x_wr_dat(0x31);
		ILI934x_wr_dat(0x33);
		ILI934x_wr_dat(0x42);
		ILI934x_wr_dat(0x05);
		ILI934x_wr_dat(0x0C);
		ILI934x_wr_dat(0x0A);
		ILI934x_wr_dat(0x28);
		ILI934x_wr_dat(0x2F);
		ILI934x_wr_dat(0x0F);
		
		ILI934x_wr_cmd(0x11);			/* Sleep out */
		_delay_ms(120);
		
		ILI934x_wr_cmd(0x29);			/* Display on */
		
		ILI934x_wr_cmd(0x2C);			/* Drive RGB Interface */
	}

	else { for(;;);} /* Invalid Device Code!! */

	/* Set LCD-Controller to RGB Interface */
	Display_RGBIF_Init();

	/* Flush Display */
	Display_clear_if();

#if 0	/* test code RED */
	Display_FillRect_If(0,MAX_X-1,0,MAX_Y-1,COL_RED);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
