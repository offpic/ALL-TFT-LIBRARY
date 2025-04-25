/********************************************************************************/
/*!
	@file			ili9481.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        16.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -S95517-AAA				(ILI9481)	16bit mode.			@n
					 -S95517-A035				(ILI9481)	8/16bit mode.		@n
					 -TFT1P2797-E				(ILI9481)	8/16bit mode.		@n
					 -XC-T0353701				(HX8357B)	8/16bit mode.		@n
					 -FTN35P02N-01				(ILI9481)	8/16bit mode &		@n
					                                        3,4Wire-SPI mode.   @n
					 -CNKT0350T37-11001A		(R61581B0)	8/16bit mode.		@n
					 -WK35039V0					(RM68042)	8/16bit mode.		@n
					 -TM035PDZ48				(ILI9486L)	8/16bit mode.		@n
					 -PRD35200P					(HX8357C)	8bit mode.			@n
					 -HTHV3802					(ILI9486L)	8/16bit mode.		@n
					 -TS7235Q(IMx Customized)	(HX8357D)	16bit mode.			@n
					 -INANBO-T35CST-ILI9488-V1	(ILI9488)   8/16bit mode.

    @section HISTORY
		2010.10.01	V1.00	Stable Release.
		2010.12.31	V2.00	Cleanup SourceCode.
		2011.03.10	V3.00	C++ Ready.
		2011.10.25	V4.00	Added DMA TransactionSupport.
		2012.01.02	V5.00	Added TFT1P2797-E Support.
		2012.07.20  V6.00	Added HX8357B Support.
		2012.11.30  V7.00	Added Serial Handling.
		2013.01.02  V8.00	Added R61581/B0 Devices Support.
							Added RM68042 Devices Support.
		2013.04.06  V9.00	Added ILI9486L Devices Support.
		2013.10.10 V10.00	Added HX8357C Devices Support.
		2013.12.30 V11.00	Added ILI9486L Another Devices Support.
		2014.05.01 V12.00	Added HX8357D Devices Support.
		2014.09.01 V13.00	Added ILI9488 Devices Support.
		2016.11.04 V14.00	Fixed DeviceID Read Command on spi mode.
		2023.05.01 V15.00	Removed unused delay function.
		2023.08.01 V16.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili9481.h"
/* check header file version for fool proof */
#if ILI9481_H != 0x1600
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
inline void ILI9481_reset(void)
{
#ifdef USE_ILI9481_TFT
	ILI9481_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI9481_RD_SET();
	ILI9481_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9481_RES_CLR();							/* RES=L, CS=L   			*/
	ILI9481_CS_CLR();
#elif  USE_ILI9481_SPI_TFT
	ILI9481_RES_SET();							/* RES=H, CS=H				*/
	ILI9481_CS_SET();
	ILI9481_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(10);								/* wait 10ms     			*/

	ILI9481_RES_CLR();							/* RES=L		   			*/
#endif

	_delay_ms(10);								/* wait 10ms     			*/
	ILI9481_RES_SET();						  	/* RES=H					*/
	_delay_ms(100);				    			/* wait 100ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_ILI9481_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9481_wr_cmd(uint8_t cmd)
{
	ILI9481_DC_CLR();							/* DC=L						*/

	ILI9481_CMD = cmd;							/* cmd(8bit)				*/
	ILI9481_WR();								/* WR=L->H					*/

	ILI9481_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void ILI9481_wr_dat(uint8_t dat)
{
	ILI9481_DATA = dat;							/* data						*/
	ILI9481_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ILI9481_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	ILI9481_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	ILI9481_WR();								/* WR=L->H					*/
	ILI9481_DATA = (uint8_t)gram;				/* lower 8bit data			*/
#else
	ILI9481_DATA = gram;						/* 16bit data 				*/
#endif
	ILI9481_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9481_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI9481_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		ILI9481_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}



/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI9481_rd_cmd(uint8_t cmd)
{
	uint8_t temp,i;
	uint16_t val;


	ILI9481_wr_cmd(cmd);
	ILI9481_WR_SET();

	for(i=0;i<4;i++){
		ReadLCDData(temp);
	}

    ReadLCDData(val);

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
static uint16_t ILI9486_rd_id(uint8_t cmd)
{
	uint8_t temp,i;
	uint16_t val;


	ILI9481_wr_cmd(cmd);
	ILI9481_WR_SET();

	for(i=0;i<3;i++){
		ReadLCDData(temp);
	}

    ReadLCDData(val);

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
static uint16_t HX8357C_rd_id(uint8_t cmd)
{
	uint16_t val;
	
 	ILI9481_wr_cmd(0xB9);
	ILI9481_wr_dat(0xFF); 
	ILI9481_wr_dat(0x83); 
	ILI9481_wr_dat(0x57);

	ILI9481_wr_cmd(cmd);
	ILI9481_WR_SET();

    ReadLCDData(val);							/* Dummy     */
    ReadLCDData(val);							/* Read Data */

	val &= 0x00FF;

	return val;
}

#elif USE_ILI9481_SPI_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9481_wr_cmd(uint8_t cmd)
{
	ILI9481_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	DNC_CMD();
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	ILI9481_DC_SET();							/* DC=H   	     */
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
static inline void ILI9481_wr_sdat(uint8_t dat)
{	
	DNC_DAT();
#if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
	SendSPID(dat);
#else
	SendSPI(dat);
#endif
}
inline void ILI9481_wr_dat(uint8_t dat)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */

	ILI9481_wr_sdat(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ILI9481_wr_gram(uint16_t gram)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */

	ILI9481_wr_sdat(gram<<3);					/* R */
	ILI9481_wr_sdat((gram>>5)<<2);				/* G */
	ILI9481_wr_sdat((gram>>11)<<3);				/* B */

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9481_wr_block(uint8_t *p,unsigned int cnt)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		ILI9481_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
		ILI9481_wr_gram(*(p+1)|*(p)<<8);
		p++;p++;
	}

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI9481_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint32_t idtemp;
	
	ILI9481_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(cmd);

	ILI9481_DC_SET();							/* DC=H		     */
	
	for(int i=0;i<3;i++){
		idtemp = RecvSPI();
	}

	idtemp <<=8;
	idtemp  |= RecvSPI();
	idtemp <<=8;
	idtemp  |= RecvSPI();
    
	val = 0xFFFF & (idtemp >>7);

	DISPLAY_NEGATE_CS();						/* CS=H		     */

	return val;
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
static uint16_t ILI9481_rd_reg(uint8_t cmd)
{
	uint16_t val;

	DISPLAY_ASSART_CS();						/* CS=L		    */
	ILI9481_DC_CLR();							/* DC=L			*/

	SendSPI(cmd);

	ILI9481_DC_SET();							/* DC=H			*/
	val = RecvSPI();							/* Dummy Read 	*/

	DISPLAY_NEGATE_CS();						/* CS=H		    */

	return val;
}

/**************************************************************************/
/*! 
    Read ID ILI9486.
*/
/**************************************************************************/
static uint16_t ILI9486_rd_id(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	ILI9481_wr_cmd(0xFB);						/* SPI Register Read Command */
	ILI9481_wr_dat(0x10);    					/* Read Mode Enable,1st Byte */
	temp = ILI9481_rd_reg(cmd);					/* Dummy Read 	*/

	ILI9481_wr_cmd(0xFB);						/* SPI Register Read Command */
	ILI9481_wr_dat(0x11);    					/* Read Mode Enable,2nd Byte */
	temp = ILI9481_rd_reg(cmd);					/* Dummy Read 	*/

	ILI9481_wr_cmd(0xFB);						/* SPI Register Read Command */
	ILI9481_wr_dat(0x12);    					/* Read Mode Enable,3rd Byte */
	temp = ILI9481_rd_reg(cmd);					/* Upper Read 	*/
	
	ILI9481_wr_cmd(0xFB);						/* SPI Register Read Command */
	ILI9481_wr_dat(0x13);    					/* Read Mode Enable,4th Byte */
	val  = ILI9481_rd_reg(cmd);					/* Lower Read	*/

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;

	return val;
}
/**************************************************************************/
/*! 
    Read ID R61581/B.
*/
/**************************************************************************/
static uint16_t R61581_rd_id(uint8_t cmd)
{
	uint16_t val,temp;

	ILI9481_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */

	SendSPI(cmd);

	ILI9481_DC_SET();							/* DC=H		     */
	
	for(int i=0;i<4;i++){
		temp = RecvSPI();
	}

	val  = RecvSPI();

	val &= 0x00FF;
	val |= temp<<8;

	DISPLAY_NEGATE_CS();						/* CS=H		     */

	return val;
}
/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
static uint16_t HX8357C_rd_id(uint8_t cmd)
{
	uint16_t val;
	
 	ILI9481_wr_cmd(0xB9);
	ILI9481_wr_dat(0xFF); 
	ILI9481_wr_dat(0x83); 
	ILI9481_wr_dat(0x57);
	
	ILI9481_wr_cmd(0xB3);
	ILI9481_wr_dat(0x80);						/* Enable SDO */
	ILI9481_wr_dat(0x00);
	ILI9481_wr_dat(0x00);
	ILI9481_wr_dat(0x11);

	ILI9481_wr_cmd(0xFE);						/* SPI Register Read Command Index */
	ILI9481_wr_dat(cmd);    					/* Read Command */

	ILI9481_wr_cmd(0xFF);						/* SPI Register Read Command Data */
	DISPLAY_ASSART_CS();						/* CS=L		    */

	val = RecvSPI();							/* Dummy Read 	*/

	DISPLAY_NEGATE_CS();						/* CS=H		    */

	val &= 0x00FF;

	return val;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void ILI9481_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	ILI9481_wr_cmd(0x2A);				/* Horizontal RAM Start ADDR */
	ILI9481_wr_dat((OFS_COL + x)>>8);
	ILI9481_wr_dat(OFS_COL + x);
	ILI9481_wr_dat((OFS_COL + width)>>8);
	ILI9481_wr_dat(OFS_COL + width);

	ILI9481_wr_cmd(0x2B);				/* Horizontal RAM Start ADDR */
	ILI9481_wr_dat((OFS_RAW + y)>>8);
	ILI9481_wr_dat(OFS_RAW + y);
	ILI9481_wr_dat((OFS_RAW + height)>>8);
	ILI9481_wr_dat(OFS_RAW + height);

	ILI9481_wr_cmd(0x2C);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI9481_clear(void)
{
	volatile uint32_t n;

	ILI9481_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9481_wr_gram(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ILI9481_init(void)
{
	volatile uint16_t devicetype,id9486l,ihx8357c;
#if   defined(USE_ILI9481_SPI_TFT)
	volatile uint16_t id61581;
#endif

	Display_IoInit_If();

	ILI9481_reset();

	/* Enable ALL Manufactutre Command For R61581xx */
	ILI9481_wr_cmd(0xB0);
	ILI9481_wr_dat(0x00);

	/* Check Device Code */
	devicetype = ILI9481_rd_cmd(0xBF);  	/* Confirm Vaild LCD Controller */
#if   defined(USE_ILI9481_TFT)
	id9486l    = ILI9486_rd_id(0xD3);  		/* Confirm Vaild LCD Controller for ILI9486L */
	ihx8357c   = HX8357C_rd_id(0xD0);  		/* Confirm Vaild LCD Controller for HX8357C/D */
#elif defined(USE_ILI9481_SPI_TFT)
	id61581    = R61581_rd_id(0xBF);  		/* Confirm Vaild LCD Controller for R61581/B Serial Interface */
	id9486l    = ILI9486_rd_id(0xD3);  		/* Confirm Vaild LCD Controller for ILI9486L Serial Interface */
	ihx8357c   = HX8357C_rd_id(0xD0);  		/* Confirm Vaild LCD Controller for HX8357C/D Serial Interface */
#endif

	if(devicetype == 0x9481)
	{
		/* Initialize ILI9481 */
		ILI9481_wr_cmd(0x11);				/* Exit Sleep Mode */
		_delay_ms(150);						/* Wait Stability */
		
		ILI9481_wr_cmd(0x13);				/* Entering Nomal Displaymode */
		
		ILI9481_wr_cmd(0xD0);				/* LCD Power Settings */
		ILI9481_wr_dat(0x07);
		ILI9481_wr_dat(0x40);/* 41 */   
		ILI9481_wr_dat(0x1C);/* 1e */
		
		ILI9481_wr_cmd(0xD1);				/* VCOM Control */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x18);
		ILI9481_wr_dat(0x1D);
		
		ILI9481_wr_cmd(0xD2);				/* Power_Setting for Normal Mode */
		ILI9481_wr_dat(0x01);
		ILI9481_wr_dat(0x11);
		
		ILI9481_wr_cmd(0xC0);				/* Panel Driving Settings */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x3B);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x11);
		
		ILI9481_wr_cmd(0xC1);				/* Display_Timing_Setting for Normal Mode */
		ILI9481_wr_dat(0x10);
		ILI9481_wr_dat(0x0B);
		ILI9481_wr_dat(0x88);
		
		ILI9481_wr_cmd(0xC5);				/* Frame Rate and Inversion Control */
		ILI9481_wr_dat(0x01);

#if !defined(USE_TFT1P2797_E)
		ILI9481_wr_cmd(0xC8);				/* Gamma Settings */
		ILI9481_wr_dat(0x00);                       
		ILI9481_wr_dat(0x30);
		ILI9481_wr_dat(0x36);
		ILI9481_wr_dat(0x45);
		ILI9481_wr_dat(0x04);
		ILI9481_wr_dat(0x16);
		ILI9481_wr_dat(0x37);
		ILI9481_wr_dat(0x75);
		ILI9481_wr_dat(0x77);
		ILI9481_wr_dat(0x54);
		ILI9481_wr_dat(0x0F);
		ILI9481_wr_dat(0x00);
#endif

		ILI9481_wr_cmd(0xE4);				/* ??? */
		ILI9481_wr_dat(0xA0);
		
		ILI9481_wr_cmd(0xF0);				/* ??? */
		ILI9481_wr_dat(0x01);
		
		ILI9481_wr_cmd(0xF3);				/* ??? */
		ILI9481_wr_dat(0x40);
		ILI9481_wr_dat(0x0A);
		
		ILI9481_wr_cmd(0xF7);				/* ??? */
		ILI9481_wr_dat(0x80);
		
		ILI9481_wr_cmd(0xB4);
		ILI9481_wr_dat(0x00);

#ifdef USE_ILI9481_TFT
		ILI9481_wr_cmd(0x36);				/* Set_address_mode */
		ILI9481_wr_dat(0x0A);				/* BGR-order,Holizontal-Flip */

		ILI9481_wr_cmd(0x3A);				/* Set_pixel_format */
		ILI9481_wr_dat(0x55);				/* RGB565(16dpp) */
#else
		ILI9481_wr_cmd(0x36);				/* Set_address_mode */
		ILI9481_wr_dat(0x02);				/* RGB-order,Holizontal-Flip */

		ILI9481_wr_cmd(0x3A);				/* Set_pixel_format */
		ILI9481_wr_dat(0x66);				/* 16dpp mode CANNOT set in Serial Mode */
#endif

		/*
		ILI9481_wr_cmd(0xB3);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x11);
		_delay_ms(120);
		*/
#if defined(USE_TFT1P2797_E)
		ILI9481_wr_cmd(0x21);				/* Invert Mode On */
#endif
		ILI9481_wr_cmd(0x29);				/* Display ON */
	}

	else if(devicetype == 0x8357)
	{
		/* Initialize HX8357B */
		ILI9481_wr_cmd(0x11);		/* Sleep Out */
		_delay_ms(150);
		
		ILI9481_wr_cmd(0xB4);		/* Set RM, DM */
		ILI9481_wr_dat(0x00);		/* MPU Mode */
		
		ILI9481_wr_cmd(0x36);		/* Set_address_mode */
		ILI9481_wr_dat(0x03);		/* BGR-order,Holizontal-Flip */
		
		ILI9481_wr_cmd(0x3A);		/* Set_pixel_format */
		ILI9481_wr_dat(0x55);		/* RGB565(16dpp) */
		
		ILI9481_wr_cmd(0xC0);		/* Set PANEL */
		ILI9481_wr_dat(0x14);
		ILI9481_wr_dat(0x3B);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x11);
		
		ILI9481_wr_cmd(0xC8);		/* Set Gamma */
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x25);
		ILI9481_wr_dat(0x01);
		ILI9481_wr_dat(0x23);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x67);
		ILI9481_wr_dat(0x25);
		ILI9481_wr_dat(0x57);
		ILI9481_wr_dat(0x32);
		ILI9481_wr_dat(0x04);
		ILI9481_wr_dat(0x04);
		
		ILI9481_wr_cmd(0xD0);		/* Set Power */
		ILI9481_wr_dat(0x44);		/* DDVDH */
		ILI9481_wr_dat(0x41);
		ILI9481_wr_dat(0x06);		/* VREG1 */
		
		ILI9481_wr_cmd(0xD1);		/* Set VCOM */
		ILI9481_wr_dat(0x53);		/* VCOMH */
		ILI9481_wr_dat(0x0F);		/* VCOML */
		
		ILI9481_wr_cmd(0xE9);		/* Set Panel */
		ILI9481_wr_dat(0x01);
		
		ILI9481_wr_cmd(0xEE);		/* Set EQ */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0xED);		/* Set DIR TIM */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0xA2);
		ILI9481_wr_dat(0xA2);
		ILI9481_wr_dat(0xA2);
		ILI9481_wr_dat(0xA2);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0xAE);
		ILI9481_wr_dat(0xAE);
		ILI9481_wr_dat(0x01);
		ILI9481_wr_dat(0xA2);
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0x29);		/* Display On */
		_delay_ms(5);
	}

#if   defined(USE_ILI9481_TFT)
	else if(devicetype == 0x1581)
#elif defined(USE_ILI9481_SPI_TFT)
	else if(id61581 == 0x1581)
#endif
	{
		/* Initialize R61581xx */
		ILI9481_wr_cmd(0x11);		/* Sleep Out */
		_delay_ms(150);
		
		ILI9481_wr_cmd(0xB4);		/* Set RM, DM */
		ILI9481_wr_dat(0x00);		/* MPU Mode */
		
		ILI9481_wr_cmd(0x38);		/* exit idle mode */
		
		ILI9481_wr_cmd(0x36);		/* Set_address_mode */
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0x3A);		/* Set_pixel_format */
#if   defined(USE_ILI9481_TFT)
		ILI9481_wr_dat(0x55);		/* RGB565(16dpp) */
#elif defined(USE_ILI9481_SPI_TFT)
		ILI9481_wr_dat(0x66);		/* RGB888(24dpp) */
#endif

		ILI9481_wr_cmd(0xB3);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0xC6);
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0xC0);
#ifndef USE_TFT1P7134_E
		ILI9481_wr_dat((1<<4)|(0<<3)|(0<<2)|(1<<1)|(1<<0)); /* BGR-order,Holizontal-Flip */
#else
		ILI9481_wr_dat((0<<4)|(0<<3)|(0<<2)|(1<<1)|(1<<0)); /* RGB-order,Holizontal-Flip */
#endif
		ILI9481_wr_dat(0x3B);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x01);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x43);
		
		ILI9481_wr_cmd(0xC1);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x12);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x08);
		
		ILI9481_wr_cmd(0xC4);
		ILI9481_wr_dat(0x11);
		ILI9481_wr_dat(0x07);
		ILI9481_wr_dat(0x03);
		ILI9481_wr_dat(0x03);
		
		ILI9481_wr_cmd(0xC8);
		ILI9481_wr_dat(0x09);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x10);
		ILI9481_wr_dat(0x85);
		ILI9481_wr_dat(0x07);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x16);
		ILI9481_wr_dat(0x05);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x32);
		ILI9481_wr_dat(0x05);
		ILI9481_wr_dat(0x16);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x88);
		ILI9481_wr_dat(0x09);
		ILI9481_wr_dat(0x10);
		ILI9481_wr_dat(0x09);
		ILI9481_wr_dat(0x04);
		ILI9481_wr_dat(0x32);
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0x35);
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0x44);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x01);
		
		ILI9481_wr_cmd(0xD0);
		ILI9481_wr_dat(0x07);
		ILI9481_wr_dat(0x07);
		ILI9481_wr_dat(0x1E);
		ILI9481_wr_dat(0x03);
		
		ILI9481_wr_cmd(0xD1);
		ILI9481_wr_dat(0x03);
		ILI9481_wr_dat(0x3A);
		ILI9481_wr_dat(0x0A);
		
		ILI9481_wr_cmd(0xD2);
		ILI9481_wr_dat(0x03);
		ILI9481_wr_dat(0x24);
		
		ILI9481_wr_cmd(0x29);
		_delay_ms(10);
	}

	else if(devicetype == 0x6804)
	{
		/* Initialize RM68040 */
		ILI9481_wr_cmd(0x11);
		_delay_ms(20);
		
		ILI9481_wr_cmd(0xD0);		 /* VCI1  VCL  VGH  VGL DDVDH VREG1OUT power amplitude setting */
		ILI9481_wr_dat(0x07); 
		ILI9481_wr_dat(0x42); 
		ILI9481_wr_dat(0x1D);
		
		ILI9481_wr_cmd(0xD1);		 /* VCOMH VCOM_AC amplitude setting */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x1A);
		ILI9481_wr_dat(0x09);
		
		ILI9481_wr_cmd(0xD2);		/* Operational Amplifier Circuit Constant Current Adjust , charge pump frequency setting */
		ILI9481_wr_dat(0x01);
		ILI9481_wr_dat(0x22);
		
		ILI9481_wr_cmd(0xC0);		/* REV SM GS */
		ILI9481_wr_dat((1<<4)|(0<<3)|(0<<2));
		ILI9481_wr_dat(0x3B);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x11);
		
		ILI9481_wr_cmd(0xC5);		/* Frame rate setting = 72HZ  when setting 0x03 */
		ILI9481_wr_dat(0x03);
		
		ILI9481_wr_cmd(0xC8);		/* Gamma setting */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x25);
		ILI9481_wr_dat(0x21);
		ILI9481_wr_dat(0x05);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x0A);
		ILI9481_wr_dat(0x65);
		ILI9481_wr_dat(0x25);
		ILI9481_wr_dat(0x77);
		ILI9481_wr_dat(0x50);
		ILI9481_wr_dat(0x0F);
		ILI9481_wr_dat(0x00);	  
						  
   		ILI9481_wr_cmd(0xF8);
		ILI9481_wr_dat(0x01);	 
		
 		ILI9481_wr_cmd(0xFE);
 		ILI9481_wr_dat(0x00);
 		ILI9481_wr_dat(0x02);
		
		ILI9481_wr_cmd(0x20);		/* Exit invert mode */
		
		ILI9481_wr_cmd(0x36);		/* Set_address_mode */
		ILI9481_wr_dat((0<<7)|(0<<6)|(0<<5)|(0<<4)|(1<<3)|(1<<1)|(0<<0));	
		
		ILI9481_wr_cmd(0x3A);		/* Set_pixel_format */
		ILI9481_wr_dat(0x55);		/* RGB565(16dpp) */
		
		ILI9481_wr_cmd(0x29);		/* Display On */
		_delay_ms(120);
	}

	else if(id9486l == 0x9486)
	{
		/* Initialize ILI9486L */
		ILI9481_wr_cmd(0x01);		/* Soft Reset */
		_delay_ms(10);				/* 5 +@ mSec */
		
		ILI9481_wr_cmd(0x11);		/* Sleep Out */
		_delay_ms(130);				/* 120 + 5 +@ mSec */
		
		ILI9481_wr_cmd(0xC0);		/* Power Control1 */
		ILI9481_wr_dat(0x10);
		ILI9481_wr_dat(0x0D);

		ILI9481_wr_cmd(0xC1);		/* Power Control2 */
		ILI9481_wr_dat(0x41);
		ILI9481_wr_dat(0x00);	/* Pwoer Control3 */
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0xC5);		/* Set VCOM */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x5A);
		ILI9481_wr_dat(0x80);
		
		ILI9481_wr_cmd(0xB1);		/* Frame Rate */
		ILI9481_wr_dat(0xB0);		/* 81Hz */
		ILI9481_wr_dat(0x11);
		
		ILI9481_wr_cmd(0xB4);		/* Display Colour Invertion */
		ILI9481_wr_dat(0x02);
		
		ILI9481_wr_cmd(0xB6);		/* Display Function Control */
		ILI9481_wr_dat(0x00);		/* System Interface */
		ILI9481_wr_dat((1<<6)|(0<<5)|(0<<4)|0x02);		
		ILI9481_wr_dat(0x3B);

#if defined(USE_TM035PDZ48)
		ILI9481_wr_cmd(0xE0);		/* Positive GANMMA */
		ILI9481_wr_dat(0x0F);
		ILI9481_wr_dat(0x21);
		ILI9481_wr_dat(0x20);
		ILI9481_wr_dat(0x0B);
		ILI9481_wr_dat(0x0F);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x4F);
		ILI9481_wr_dat(0xF1);
		ILI9481_wr_dat(0x3F);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x0D);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0xE1);		/* Negative GANMMA */
		ILI9481_wr_dat(0x0F);
		ILI9481_wr_dat(0x3F);
		ILI9481_wr_dat(0x3F);
		ILI9481_wr_dat(0x0F);
		ILI9481_wr_dat(0x12);
		ILI9481_wr_dat(0x07);
		ILI9481_wr_dat(0x40);
		ILI9481_wr_dat(0x0E);
		ILI9481_wr_dat(0x30);
		ILI9481_wr_dat(0x07);
		ILI9481_wr_dat(0x10);
		ILI9481_wr_dat(0x04);
		ILI9481_wr_dat(0x20);
		ILI9481_wr_dat(0x1E);
		ILI9481_wr_dat(0x00);
#else 	/* defined(USE_HTHV3802) */
		ILI9481_wr_cmd(0xE0);		/* Positive GANMMA */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x2C);
		ILI9481_wr_dat(0x2C);
		ILI9481_wr_dat(0x0B);
		ILI9481_wr_dat(0x0C);
		ILI9481_wr_dat(0x04);
		ILI9481_wr_dat(0x4C);
		ILI9481_wr_dat(0x64);
		ILI9481_wr_dat(0x36);
		ILI9481_wr_dat(0x03);
		ILI9481_wr_dat(0x0E);
		ILI9481_wr_dat(0x01);
		ILI9481_wr_dat(0x10);
		ILI9481_wr_dat(0x01);
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0xE1);		/* Negative GANMMA */
		ILI9481_wr_dat(0x0F);
		ILI9481_wr_dat(0x37);
		ILI9481_wr_dat(0x37);
		ILI9481_wr_dat(0x0C);
		ILI9481_wr_dat(0x0F);
		ILI9481_wr_dat(0x05);
		ILI9481_wr_dat(0x50);
		ILI9481_wr_dat(0x32);
		ILI9481_wr_dat(0x36);
		ILI9481_wr_dat(0x04);
		ILI9481_wr_dat(0x0B);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x19);
		ILI9481_wr_dat(0x14);
		ILI9481_wr_dat(0x0F);
#endif
		ILI9481_wr_cmd(0xF2);		/* ??? */
		ILI9481_wr_dat(0x18);
		ILI9481_wr_dat(0xA3);
		ILI9481_wr_dat(0x12);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0xB2);
		ILI9481_wr_dat(0x12);
		ILI9481_wr_dat(0xFF);
		ILI9481_wr_dat(0x10);
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0xF8);		/* ??? */
		ILI9481_wr_dat(0x21);
		ILI9481_wr_dat(0x04);
		
		ILI9481_wr_cmd(0xF9);		/* ??? */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x08);
		
		ILI9481_wr_cmd(0x20);		/* Display Invertion OFF */
		
		ILI9481_wr_cmd(0x13);		/* Enter Nomal Mode */
		
		ILI9481_wr_cmd(0x38);		/* Idle mode OFF */
		
		ILI9481_wr_cmd(0x3A);		/* Set_pixel_format */
#if   defined(USE_ILI9481_TFT)
		ILI9481_wr_dat(0x55);		/* RGB565(16dpp) */
#elif defined(USE_ILI9481_SPI_TFT)
		ILI9481_wr_dat(0x66);		/* RGB888(24dpp) */
#endif

		ILI9481_wr_cmd(0x36);		/* Set_address_mode */
		ILI9481_wr_dat((1<<7)|(1<<6)|(0<<5)|(0<<4)|(1<<3)|(0<<2)|(0<<0));	
		
		ILI9481_wr_cmd(0x29);		/* Display On */
		_delay_ms(10);
	}

	else if(ihx8357c == 0x90)
	{
		/* Initialize HX8357C */
		ILI9481_wr_cmd(0xB9);
		ILI9481_wr_dat(0xFF);
		ILI9481_wr_dat(0x83);
		ILI9481_wr_dat(0x57);
		_delay_ms(5);
		
		ILI9481_wr_cmd(0xB6);
		ILI9481_wr_dat(0x21);
		
		ILI9481_wr_cmd(0x11);
		_delay_ms(150);
		
		/* Set_POWER */
		ILI9481_wr_cmd(0xCC);
		ILI9481_wr_dat(0x09);
		
		ILI9481_wr_cmd(0xB3);
		ILI9481_wr_dat(0x00);     /* MCU Mode */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x06);
		ILI9481_wr_dat(0x06);
		
		ILI9481_wr_cmd(0xB1);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x15);
		ILI9481_wr_dat(0x1C);
		ILI9481_wr_dat(0x1C);
		ILI9481_wr_dat(0x83);
		ILI9481_wr_dat(0xAA);
		
		ILI9481_wr_cmd(0xC0);
		ILI9481_wr_dat(0x24);
		ILI9481_wr_dat(0x24);
		ILI9481_wr_dat(0x01);
		ILI9481_wr_dat(0x3C);
		ILI9481_wr_dat(0x1E);
		ILI9481_wr_dat(0x08);
		
		ILI9481_wr_cmd(0xB4);
		ILI9481_wr_dat(0x01);
		ILI9481_wr_dat(0x40);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x2A);
		ILI9481_wr_dat(0x2A);
		ILI9481_wr_dat(0x0D);
		ILI9481_wr_dat(0x4F);
		
		ILI9481_wr_cmd(0xE0);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x11);
		ILI9481_wr_dat(0x23);
		ILI9481_wr_dat(0x2C);
		ILI9481_wr_dat(0x40);
		ILI9481_wr_dat(0x4A);
		ILI9481_wr_dat(0x52);
		ILI9481_wr_dat(0x48);
		ILI9481_wr_dat(0x41);
		ILI9481_wr_dat(0x3C);
		ILI9481_wr_dat(0x33);
		ILI9481_wr_dat(0x2E);
		ILI9481_wr_dat(0x28);
		ILI9481_wr_dat(0x27);
		ILI9481_wr_dat(0x1B);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x11);
		ILI9481_wr_dat(0x23);
		ILI9481_wr_dat(0x2C);
		ILI9481_wr_dat(0x40);
		
		ILI9481_wr_dat(0x4A);
		ILI9481_wr_dat(0x52);
		ILI9481_wr_dat(0x48);
		ILI9481_wr_dat(0x41);
		ILI9481_wr_dat(0x3C);
		ILI9481_wr_dat(0x33);
		ILI9481_wr_dat(0x2E);
		ILI9481_wr_dat(0x28);
		ILI9481_wr_dat(0x27);
		ILI9481_wr_dat(0x1B);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x01);
		
		ILI9481_wr_cmd(0x36);		/* Set_address_mode */
		ILI9481_wr_dat(0x03);		/* BGR-order,Holizontal-Flip */
		
		ILI9481_wr_cmd(0x3A);		/* Set_pixel_format */
		ILI9481_wr_dat(0x55);		/* RGB565(16dpp) */
		
		/* Display On */
		ILI9481_wr_cmd(0x29);
		_delay_ms(25);
	}

	else if(ihx8357c == 0x99)
	{
		/* Initialize HX8357D */
		ILI9481_wr_cmd(0xB9);
		ILI9481_wr_dat(0xFF);
		ILI9481_wr_dat(0x83);
		ILI9481_wr_dat(0x57);
		_delay_ms(5);
		
		ILI9481_wr_cmd(0x11);
		_delay_ms(150);
		
		/* Set_POWER */
		ILI9481_wr_cmd(0xCC);
		ILI9481_wr_dat(0x09);
		
		ILI9481_wr_cmd(0xB2);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x83);
		ILI9481_wr_dat(0x3B);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x20);
		
		ILI9481_wr_cmd(0xB3);
		ILI9481_wr_dat(0x00);     /* MCU Mode */
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x06);
		ILI9481_wr_dat(0x06);
		
		ILI9481_wr_cmd(0xC0);
		ILI9481_wr_dat(0x73);
		ILI9481_wr_dat(0x50);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x3C);
		ILI9481_wr_dat(0xC4);
		ILI9481_wr_dat(0x08);
		
		ILI9481_wr_cmd(0xB4);
		ILI9481_wr_dat(0x01);
		ILI9481_wr_dat(0x40);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x2A);
		ILI9481_wr_dat(0x2A);
		ILI9481_wr_dat(0x0D);
		ILI9481_wr_dat(0x4F);
		
		ILI9481_wr_cmd(0xE0);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x11);
		ILI9481_wr_dat(0x23);
		ILI9481_wr_dat(0x2C);
		ILI9481_wr_dat(0x40);
		ILI9481_wr_dat(0x4A);
		ILI9481_wr_dat(0x52);
		ILI9481_wr_dat(0x48);
		ILI9481_wr_dat(0x41);
		ILI9481_wr_dat(0x3C);
		ILI9481_wr_dat(0x33);
		ILI9481_wr_dat(0x2E);
		ILI9481_wr_dat(0x28);
		ILI9481_wr_dat(0x27);
		ILI9481_wr_dat(0x1B);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x11);
		ILI9481_wr_dat(0x23);
		ILI9481_wr_dat(0x2C);
		ILI9481_wr_dat(0x40);
		
		ILI9481_wr_dat(0x4A);
		ILI9481_wr_dat(0x52);
		ILI9481_wr_dat(0x48);
		ILI9481_wr_dat(0x41);
		ILI9481_wr_dat(0x3C);
		ILI9481_wr_dat(0x33);
		ILI9481_wr_dat(0x2E);
		ILI9481_wr_dat(0x28);
		ILI9481_wr_dat(0x27);
		ILI9481_wr_dat(0x1B);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x01);
		
		ILI9481_wr_cmd(0x36);		/* Set_address_mode */
		ILI9481_wr_dat(0x00);		/* BGR-order,Holizontal-Flip */
		
		ILI9481_wr_cmd(0x3A);		/* Set_pixel_format */
		ILI9481_wr_dat(0x55);		/* RGB565(16dpp) */
		
		ILI9481_wr_cmd(0x29);
		_delay_ms(25);
	}

	else if(id9486l == 0x9488)
	{
		/* Initialize ILI9488 */
		ILI9481_wr_cmd(0xE0);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x0E);
		ILI9481_wr_dat(0x15);
		ILI9481_wr_dat(0x06);
		ILI9481_wr_dat(0x13);
		ILI9481_wr_dat(0x09);
		ILI9481_wr_dat(0x3A);
		ILI9481_wr_dat(0xAC);
		ILI9481_wr_dat(0x4F);
		ILI9481_wr_dat(0x05);
		ILI9481_wr_dat(0x0D);
		ILI9481_wr_dat(0x0B);
		ILI9481_wr_dat(0x33);
		ILI9481_wr_dat(0x3B);
		ILI9481_wr_dat(0x0F);
		
		ILI9481_wr_cmd(0xE1);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x0E);
		ILI9481_wr_dat(0x16);
		ILI9481_wr_dat(0x05);
		ILI9481_wr_dat(0x13);
		ILI9481_wr_dat(0x08);
		ILI9481_wr_dat(0x3B);
		ILI9481_wr_dat(0x9A);
		ILI9481_wr_dat(0x50);
		ILI9481_wr_dat(0x0A);
		ILI9481_wr_dat(0x13);
		ILI9481_wr_dat(0x0F);
		ILI9481_wr_dat(0x31);
		ILI9481_wr_dat(0x36);
		ILI9481_wr_dat(0x0F);
		
		ILI9481_wr_cmd(0xC0);
		ILI9481_wr_dat(0x10);
		ILI9481_wr_dat(0x10);
		
		ILI9481_wr_cmd(0xC1);
		ILI9481_wr_dat(0x44);
		
		ILI9481_wr_cmd(0xC5);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x10);
		ILI9481_wr_dat(0x80);
		
		ILI9481_wr_cmd(0x36);
		ILI9481_wr_dat(0x08);
		
		ILI9481_wr_cmd(0x3A);	/* Color Pixel Format */
		ILI9481_wr_dat(0x55);
		
		ILI9481_wr_cmd(0xB0);	/* Interface Mode Control */
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0xB1);	/* Frame rate 70Hz */
		ILI9481_wr_dat(0xB0);
		
		ILI9481_wr_cmd(0xB4);
		ILI9481_wr_dat(0x02);
		
		ILI9481_wr_cmd(0xB6);
		ILI9481_wr_dat(0x02);
		ILI9481_wr_dat(0x22);
		ILI9481_wr_dat(0xC6);
		
		ILI9481_wr_cmd(0xBE);
		ILI9481_wr_dat(0x00);
		ILI9481_wr_dat(0x04);
		
		ILI9481_wr_cmd(0xE9);
		ILI9481_wr_dat(0x00);
		
		ILI9481_wr_cmd(0xF7);    
		ILI9481_wr_dat(0xA9); 
		ILI9481_wr_dat(0x51); 
		ILI9481_wr_dat(0x2C); 
		ILI9481_wr_dat(0x82);
		
		ILI9481_wr_cmd(0x11);
		_delay_ms(120);
		
		ILI9481_wr_cmd(0x21);	/* display on */
		ILI9481_wr_cmd(0x29); 
	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI9481_clear();

#if 0 	/* test code RED */
	volatile uint32_t n;

	ILI9481_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		ILI9481_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
