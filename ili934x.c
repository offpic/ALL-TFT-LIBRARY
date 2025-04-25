/********************************************************************************/
/*!
	@file			ili934x.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        14.00
    @date           2023.09.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				 	 	 @n
					Available TFT-LCM are listed below.							 	 	 @n
					 -SDT028ATFT				(ILI9341)	8/16bit & 4-Wire,8bitSerial. @n
					 -SDT022ATFT				(ILI9340)	8/16bit & 4-Wire,8bitSerial. @n
					 -NHD-2.4-240320SF-CTXI#-T1 (ILI9340)	8/16bit mode.				 @n
					 -DJN 15-12378-18251		(ILI9338B)	8/16bit mode.				 @n
					 -STM32F429I-Discovery		(ILI9341)	4-Wire,8bitSerial-1 only.	 @n
					 -FGD240P3735				(NT35702)	8/16bit mode.				 @n
					 -DJN 15-12406-10481		(NT39116B)	8/16bit mode.

    @section HISTORY
		2011.11.10	V1.00	First Release.
		2011.12.23	V2.00	Data Write Bug Fix.
		2012.01.15	V3.00	Data Write Bug Fix in i8080-bus access.
						    Added NHD-2.4-240320SF-CTXI#-T1 support.
		2012.08.01	V4.00	Improved Register Read Function in Serial Interface.
		2012.11.30	V5.00	Added DJN 15-12378-18251(ILI9338B) support.
		2013.03.20  V6.00	Fixed ILI934x DeviceID Read Commands.
		2013.11.30  V7.00	Adopted STM32F429I-Discovery.
		2014.05.29  V8.00	Fixed ILI9341 Initialization.
		2014.10.15	V9.00	Fixed 8-bit access bug.
		2014.11.01 V10.00	Added FGD240P3735(NT35702) support.
		2016.11.04 V11.00	Fixed DeviceID Read Command on spi mode.
		2023.05.01 V12.00	Removed unused delay function.
		2023.08.01 V13.00	Revised release.
		2023.09.01 V14.00	Added DJN 15-12406-10481(NT39116B) support.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili934x.h"
/* check header file version for fool proof */
#if ILI934X_H != 0x1400
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
#ifdef USE_ILI934x_TFT
	ILI934x_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI934x_RD_SET();
	ILI934x_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	ILI934x_RES_CLR();							/* RES=L, CS=L   			*/
	ILI934x_CS_CLR();

#elif  USE_ILI934x_SPI_TFT
	ILI934x_RES_SET();							/* RES=H, CS=H				*/
	ILI934x_CS_SET();
	ILI934x_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	ILI934x_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(10);								/* wait 10ms     			*/
	ILI934x_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait over 120ms     		*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_ILI934x_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI934x_wr_cmd(uint8_t cmd)
{
	ILI934x_DC_CLR();						/* DC=L						*/

	ILI934x_CMD = cmd;						/* cmd(8bit)				*/
	ILI934x_WR();							/* WR=L->H					*/

	ILI934x_DC_SET();						/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ILI934x_wr_dat(uint8_t dat)
{
	ILI934x_DATA = dat;						/* data(8bit_Low or 16bit)	*/
	ILI934x_WR();							/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ILI934x_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI934x_DATA = (uint8_t)(gram>>8);		/* upper 8bit data		*/
	ILI934x_WR();							/* WR=L->H				*/
	ILI934x_DATA = (uint8_t)gram;			/* lower 8bit data		*/
#else
	ILI934x_DATA = gram;					/* 16bit data			*/
#endif
	ILI934x_WR();							/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI934x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI934x_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		ILI934x_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI934x_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	ILI934x_wr_cmd(cmd);
	ILI934x_WR_SET();

    ReadLCDData(temp);							/* Dummy Read(Invalid Data) */
    ReadLCDData(temp);							/* Dummy Read				*/
    ReadLCDData(temp);							/* Upper Read				*/
    ReadLCDData(val);							/* Lower Read				*/

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}


#elif USE_ILI934x_SPI_TFT
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
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI934x_rd_cmd(uint8_t cmd)
{
	uint16_t val;

	DISPLAY_ASSART_CS();						/* CS=L		    */
	ILI934x_DC_CLR();							/* DC=L			*/

	SendSPI(cmd);

	ILI934x_DC_SET();							/* DC=H			*/
	val = RecvSPI();							/* Dummy Read 	*/

	DISPLAY_NEGATE_CS();						/* CS=H		    */

	return val;
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
	ILI934x_wr_dat(0x10);    					/* Read Mode Enable,1st Byte */
	temp = ILI934x_rd_cmd(cmd);					/* Dummy Read 	*/
	
	ILI934x_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI934x_wr_dat(0x11);    					/* Read Mode Enable,2nd Byte */
	temp = ILI934x_rd_cmd(cmd);					/* Dummy Read 	*/
	
	ILI934x_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI934x_wr_dat(0x12);    					/* Read Mode Enable,3rd Byte */
	temp = ILI934x_rd_cmd(cmd);					/* Upper Read 	*/

	ILI934x_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI934x_wr_dat(0x13);    					/* Read Mode Enable,4th Byte */
	val  = ILI934x_rd_cmd(cmd);					/* Lower Read	*/

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;

	return val;
#else
	return 0x9341;
#endif
}
#endif


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

	ILI934x_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
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
#ifdef USE_ILI934x_TFT
	devicetype = ILI934x_rd_cmd(0xD3);  	/* Confirm Vaild LCD Controller */
#elif USE_ILI934x_SPI_TFT
	devicetype = ILI934x_rd_id(0xD3);  		/* Confirm Vaild LCD Controller Serial Interface */
#endif

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
		
		ILI934x_wr_cmd(0xC0);			/* Power control */
		ILI934x_wr_dat(0x21);			/* VRH[5:0] */
		
		ILI934x_wr_cmd(0xC1);			/* Power control */
		ILI934x_wr_dat(0x11);			/* SAP[2:0];BT[3:0] */
		
		ILI934x_wr_cmd(0xC5);			/* Vcomh & Vcoml control */
		ILI934x_wr_dat(0x45);
		ILI934x_wr_dat(0x15);
		
		ILI934x_wr_cmd(0xC7);			/* vcom adjust control */
		ILI934x_wr_dat(0x90);

		ILI934x_wr_cmd(0x36);			/* Memory Access Control */
#if defined(USE_32F429IDISCOVERY)
		ILI934x_wr_dat((1<<7)|(1<<3));  /*  Vertically Inverted */
#else
		ILI934x_wr_dat(0x48);   
#endif
		ILI934x_wr_cmd(0xF2);			/* 3Gamma Function Disable */ 
        ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0xB0);			/* RGB Interface control */
        ILI934x_wr_dat(0xC2);
		
		ILI934x_wr_cmd(0xB6);			/* Display Function Control register */
		ILI934x_wr_dat(0x0A);
		ILI934x_wr_dat(0x82);			/* 0x02 background black,0x82 background white */
		ILI934x_wr_dat(0x27);
		ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0x3A); 			/* RGB & CPU 18bit 0x66 / 16bit 0x55 */
        ILI934x_wr_dat(0x55);
		
		ILI934x_wr_cmd(0xF6);			/* Interface control */
        ILI934x_wr_dat(0x01);			/* i8080 interface */
        ILI934x_wr_dat(0x30);
        ILI934x_wr_dat(0x00);
		
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
	}

	else if(devicetype == 0x9340)
	{
		/* Initialize ILI9340 */
		ILI934x_wr_cmd(0x11);					/* Sleep out */
		_delay_ms(60); 
		
		ILI934x_wr_cmd(0xEF);
		ILI934x_wr_dat(0x03);
		ILI934x_wr_dat(0x80);
		ILI934x_wr_dat(0x02);
		
		ILI934x_wr_cmd(0xCF);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0xAA);
		ILI934x_wr_dat(0xB0);
		
		ILI934x_wr_cmd(0xED);
		ILI934x_wr_dat(0x67);
		ILI934x_wr_dat(0x03);
		ILI934x_wr_dat(0x12);
		ILI934x_wr_dat(0x81);
		
		ILI934x_wr_cmd(0xE8);
		ILI934x_wr_dat(0x85); 
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x7A);
		
		ILI934x_wr_cmd(0x3A); 					/* RGB & CPU 18bit 0x66 / 16bit 0x55 */
        ILI934x_wr_dat(0x55);
		
		ILI934x_wr_cmd(0xF6);					/* Interface control */
        ILI934x_wr_dat(0x01);
        ILI934x_wr_dat(0x30);
        ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0xCB);
		ILI934x_wr_dat(0x39);
		ILI934x_wr_dat(0x2C);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x34);
		ILI934x_wr_dat(0x02);
		
		ILI934x_wr_cmd(0xF7);
		ILI934x_wr_dat(0x20);
		
		ILI934x_wr_cmd(0xEA);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0xC0);					/* Power control */
		ILI934x_wr_dat(0x23);					/* VRH[5:0] */
		ILI934x_wr_dat(0x08);
		
		ILI934x_wr_cmd(0xC1);					/* Power control */
		ILI934x_wr_dat(0x11);					/* SAP[2:0];BT[3:0] */
		
		ILI934x_wr_cmd(0xC5);					/* Vcomh & Vcoml control */
		ILI934x_wr_dat(0x25);
		ILI934x_wr_dat(0x2B);
		
		ILI934x_wr_cmd(0xC7);					/* vcom adjust control */
		ILI934x_wr_dat(0xC1);
		
		ILI934x_wr_cmd(0x3A);
		ILI934x_wr_dat(0x55);
		
		ILI934x_wr_cmd(0x36);					/* Memory Access Control */
		ILI934x_wr_dat(0x48);
		
		ILI934x_wr_cmd(0xB1);           
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x12);
		ILI934x_wr_cmd(0xB4);           
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_cmd(0xB6);           
		ILI934x_wr_dat(0x0A);
		ILI934x_wr_dat(0x82);					/* 0x02 background black,0x82 background white */
		ILI934x_wr_dat(0x27);
		ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0xF2);					/* 3Gamma Function Disable */ 
        ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0x26); 					/* Gamma select G2.2 */
		ILI934x_wr_dat(0x01);
		
		ILI934x_wr_cmd(0xE0);					/* Positive  gamma */
		ILI934x_wr_dat(0x0F);
		ILI934x_wr_dat(0x17);
		ILI934x_wr_dat(0x16);
		ILI934x_wr_dat(0x0C);
		ILI934x_wr_dat(0x10);
		ILI934x_wr_dat(0x09);
		ILI934x_wr_dat(0x45);
		ILI934x_wr_dat(0xA9);
		ILI934x_wr_dat(0x33);
		ILI934x_wr_dat(0x08);
		ILI934x_wr_dat(0x0D);
		ILI934x_wr_dat(0x03);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0xE1);					/* Negative  gamma */
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x28);
		ILI934x_wr_dat(0x29);
		ILI934x_wr_dat(0x03);
		ILI934x_wr_dat(0x0F);
		ILI934x_wr_dat(0x06);
		ILI934x_wr_dat(0x3A);
		ILI934x_wr_dat(0x56);
		ILI934x_wr_dat(0x4C);
		ILI934x_wr_dat(0x07);
		ILI934x_wr_dat(0x12);
		ILI934x_wr_dat(0x0C);
		ILI934x_wr_dat(0x3F);
		ILI934x_wr_dat(0x3F);
		ILI934x_wr_dat(0x0F);
		
		ILI934x_wr_cmd(0x29);					/* Display on */
	}

	else if((devicetype & 0x00FF) == 0x0038)
	{
		/* Initialize ILI9338B */
		/* Start Initial Sequence */
		ILI934x_wr_cmd(0xCB);					/* AP[2:0] */
		ILI934x_wr_dat(0x01);
		
		ILI934x_wr_cmd(0xC0);					/* Power control */
		ILI934x_wr_dat(0x26);					/* VRH[5:0] */
		ILI934x_wr_dat(0x01);					/* VC[2:0] */
		
		ILI934x_wr_cmd(0xC1);					/* Power control */
		ILI934x_wr_dat(0x14);					/* SAP[2:0];BT[3:0] */
		
		ILI934x_wr_cmd(0xC5);					/* VCM control */
		ILI934x_wr_dat(0x31);					/* 31 */
		ILI934x_wr_dat(0x3C);					/* 3C */
		
		ILI934x_wr_cmd(0x26);					/* Gamma curve selected */
		ILI934x_wr_dat(0x01);
		ILI934x_wr_cmd(0xB1); 
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x13);
		ILI934x_wr_cmd(0xC2); 
		ILI934x_wr_dat(0x00);
		ILI934x_wr_cmd(0xC7); 
		ILI934x_wr_dat(0xBF);
		
		ILI934x_wr_cmd(0xE0);					/* Set Gamma */
		ILI934x_wr_dat(0x0F);
		ILI934x_wr_dat(0x22);
		ILI934x_wr_dat(0x1B);
		ILI934x_wr_dat(0x01);
		ILI934x_wr_dat(0x08);
		ILI934x_wr_dat(0x01);
		ILI934x_wr_dat(0x49);
		ILI934x_wr_dat(0x41);
		ILI934x_wr_dat(0x3D);
		ILI934x_wr_dat(0x01);
		ILI934x_wr_dat(0x18);
		ILI934x_wr_dat(0x04);
		ILI934x_wr_dat(0x13);
		ILI934x_wr_dat(0x0E);
		ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0xE1);					/* Set Gamma */
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x23);
		ILI934x_wr_dat(0x22);
		ILI934x_wr_dat(0x05);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x39);
		ILI934x_wr_dat(0x20);
		ILI934x_wr_dat(0x49);
		ILI934x_wr_dat(0x03);
		ILI934x_wr_dat(0x0B);
		ILI934x_wr_dat(0x0B);
		ILI934x_wr_dat(0x33);
		ILI934x_wr_dat(0x37);
		ILI934x_wr_dat(0x0F);
		
		ILI934x_wr_cmd(0x11);					/* Exit Sleep */
		_delay_ms(10);
		
		ILI934x_wr_cmd(0x36);
		ILI934x_wr_dat((0<<7)|(1<<6)|(0<<5)|(0<<4)|(1<<3)|(0<<2));		/* Set pixel write order: Red, Green, Blue */
		
		ILI934x_wr_cmd(0x3A);
		ILI934x_wr_dat(0x05);
		
		ILI934x_wr_cmd(0x29);					/* display on */
		_delay_ms(10);
	}

	else if(((devicetype & 0xFF00) == 0x7000) || ((devicetype & 0xFF00) == 0x1600)) /* 0xXX0170XX 0r 0xXX011602 */
	{
		/* Initialize NT35702/NT39116B */
		/* Start Initial Sequence */
		ILI934x_wr_cmd(0x01);
		_delay_ms(20);
		
		ILI934x_wr_cmd(0x11);
		_delay_ms(120);
		
		ILI934x_wr_cmd(0xC2);
		ILI934x_wr_dat(0x04);
		ILI934x_wr_dat(0x00);
		
		ILI934x_wr_cmd(0xD7);
		ILI934x_wr_dat(0x40);
		ILI934x_wr_dat(0xE0);
		
		ILI934x_wr_cmd(0xFD);
		ILI934x_wr_dat(0x06);
		ILI934x_wr_dat(0x11);
		
		ILI934x_wr_cmd(0xC5);
		ILI934x_wr_dat(0x16);
		ILI934x_wr_dat(0x3D);
		
		ILI934x_wr_cmd(0xC0);
		ILI934x_wr_dat(0x05);
		
		ILI934x_wr_cmd(0xC7);
		ILI934x_wr_dat(0xC4);
		
		ILI934x_wr_cmd(0x36);
		ILI934x_wr_dat((1<<7)|(1<<6)|(0<<5)|(1<<4)|(1<<3)|(0<<2));		/* Set pixel write order: Red, Green, Blue */
		
		ILI934x_wr_cmd(0x26);
		ILI934x_wr_dat(0x10);
		
		ILI934x_wr_cmd(0xE0);
		ILI934x_wr_dat(0x01);
		ILI934x_wr_dat(0x01);
		ILI934x_wr_dat(0x02);
		ILI934x_wr_dat(0x06);
		ILI934x_wr_dat(0x0C);
		ILI934x_wr_dat(0x13);
		ILI934x_wr_dat(0x25);
		ILI934x_wr_dat(0x9B);
		ILI934x_wr_dat(0x38);
		ILI934x_wr_dat(0x0C);
		ILI934x_wr_dat(0x1D);
		ILI934x_wr_dat(0x24);
		ILI934x_wr_dat(0x09);
		ILI934x_wr_dat(0x0C);
		ILI934x_wr_dat(0x3F);
		
		ILI934x_wr_cmd(0xE1);
		ILI934x_wr_dat(0x00);
		ILI934x_wr_dat(0x33);
		ILI934x_wr_dat(0x36);
		ILI934x_wr_dat(0x0C);
		ILI934x_wr_dat(0x13);
		ILI934x_wr_dat(0x15);
		ILI934x_wr_dat(0x49);
		ILI934x_wr_dat(0x58);
		ILI934x_wr_dat(0x5D);
		ILI934x_wr_dat(0x0C);
		ILI934x_wr_dat(0x23);
		ILI934x_wr_dat(0x29);
		ILI934x_wr_dat(0x3D);
		ILI934x_wr_dat(0x3E);
		ILI934x_wr_dat(0x3E);
		
		ILI934x_wr_cmd(0x3A);
		ILI934x_wr_dat(0x55);
		_delay_ms(15);
		
		ILI934x_wr_cmd(0x29);
	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI934x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ILI934x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI934x_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
