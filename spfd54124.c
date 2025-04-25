/********************************************************************************/
/*!
	@file			spfd54124.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        7.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -H179IT01-V1			(3-wire & 9-bit serial only!)		@n
					 -Nokia Modules			(3-wire & 9-bit serial only!)		@n
					  C1-01,166x,1610,1800,5030									@n
					  610x,5200,6060,6070,6080,6125,7360						@n
					 -TXDT180A-17			(8bit mode only!)

    @section HISTORY
		2011.12.23	V1.00	Stable Release.
		2012.04.22  V2.00	Added Nokia LCD-Module Supports.
		2012.05.25  V3.00	Added Hardware 9-bitSerial Handling.
		2014.04.05  V4.00	Added TXDT180A-17 Module Support.
		2014.10.15	V5.00	Fixed 8-bit access bug.
		2023.05.01	V6.00	Removed unused delay function.
		2023.08.01	V7.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "spfd54124.h"
/* check header file version for fool proof */
#if SPFD54124_H != 0x0700
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#if defined(USE_NOKIA_C101_166x_1610_1800_5030)
 #warning "You Select NOKIA_C1-01_166x_1610_1800_5030 Module(SPFD54124_128x160)!"
#elif  defined(USE_NOKIA_160x_5200_6060_6080_6125_7360)
 #warning "You Select NOKIA_160x_5200_6060_6080_6125_7360 Module(SPFD54124_132x162)!"
#elif  defined(USE_TXDT180A_17)
 #warning "You Select USE_TXDT180A_17 Module(SPFD54124_128x160)!"
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
inline void SPFD54124_reset(void)
{
#ifdef USE_SPFD54124_TFT
	SPFD54124_RES_SET();						/* RES=H, RD=H, WR=H   		*/
	SPFD54124_RD_SET();
	SPFD54124_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	SPFD54124_RES_CLR();						/* RES=L, CS=L   			*/
	SPFD54124_CS_CLR();

#elif  USE_SPFD54124_SPI_TFT
	SPFD54124_RES_SET();						/* RES=H, CS=H				*/
	SPFD54124_CS_SET();
	SPFD54124_SCLK_SET();						/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 1ms     			*/

	SPFD54124_RES_CLR();						/* RES=L, CS=L   			*/
	SPFD54124_CS_CLR();

#endif

	_delay_ms(20);								/* wait 20ms     			*/
	SPFD54124_RES_SET();						/* RES=H					*/
	_delay_ms(20);				    			/* wait 20ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_SPFD54124_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SPFD54124_wr_cmd(uint8_t cmd)
{
	SPFD54124_DC_CLR();							/* DC=L		    	 	*/
	
	SPFD54124_CMD = cmd;						/* D7..D0=cmd   		*/
	SPFD54124_WR();								/* WR=L->H      		*/
	
	SPFD54124_DC_SET();							/* DC=H   	     		*/
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void SPFD54124_wr_dat(uint8_t dat)
{
	SPFD54124_DATA = dat;						/* D7..D0=dat    	*/
	SPFD54124_WR();								/* WR=L->H       	*/
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void SPFD54124_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	SPFD54124_DATA = (uint8_t)(gram>>8);	/* upper 8bit data		*/
	SPFD54124_WR();							/* WR=L->H				*/
	SPFD54124_DATA = (uint8_t)gram;			/* lower 8bit data		*/
#else
	SPFD54124_DATA = gram;					/* 16bit data			*/
#endif
	SPFD54124_WR();							/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t SPFD54124_rd_cmd(uint8_t cmd)
{
	uint16_t val,temp;

	SPFD54124_wr_cmd(cmd);
	SPFD54124_WR_SET();

    ReadLCDData(temp);						/* Dummy Read				*/
    ReadLCDData(temp);						/* Upper Read				*/
    ReadLCDData(val);						/* Lower Read				*/

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SPFD54124_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		SPFD54124_wr_dat(*p++);
		SPFD54124_wr_dat(*p++);
		SPFD54124_wr_dat(*p++);
		SPFD54124_wr_dat(*p++);
	}
	while (n--) {
		SPFD54124_wr_dat(*p++);
	}
#endif

}

#else /* USE_SPFD54124_SPI_TFT */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void SPFD54124_wr_cmd(uint8_t cmd)
{
	SPFD54124_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	DNC_CMD();
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	SPFD54124_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
static inline void SPFD54124_wr_sdat(uint8_t dat)
{	
	DNC_DAT();
#if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
	SendSPID(dat);
#else
	SendSPI(dat);
#endif
}
inline void SPFD54124_wr_dat(uint8_t dat)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SPFD54124_wr_sdat(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void SPFD54124_wr_gram(uint16_t gram)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SPFD54124_wr_sdat((uint8_t)(gram>>8));
	SPFD54124_wr_sdat((uint8_t)gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t SPFD54124_rd_cmd(uint8_t cmd)
{
	/* Read Function was NOT implemented in 9-bit SPI-MODE! */
	return 0x0614;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void SPFD54124_wr_block(uint8_t *p,unsigned int cnt)
{
	int n;
	
	n = cnt % 4;
	cnt /= 4;

	DISPLAY_ASSART_CS();						/* CS=L		     */

	while (cnt--) {
		SPFD54124_wr_sdat(*p++);
		SPFD54124_wr_sdat(*p++);
		SPFD54124_wr_sdat(*p++);
		SPFD54124_wr_sdat(*p++);
	}
	while (n--) {
		SPFD54124_wr_sdat(*p++);
	}

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}
#endif



/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void SPFD54124_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	SPFD54124_wr_cmd(CASET); 
	SPFD54124_wr_dat(0);
	SPFD54124_wr_dat(OFS_COL + x);
	SPFD54124_wr_dat(0);
	SPFD54124_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	SPFD54124_wr_cmd(RASET);
	SPFD54124_wr_dat(0);
	SPFD54124_wr_dat(OFS_RAW + y); 
	SPFD54124_wr_dat(0);
	SPFD54124_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	SPFD54124_wr_cmd(RAMWR);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void SPFD54124_clear(void)
{
	volatile uint32_t n;

	SPFD54124_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		SPFD54124_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void SPFD54124_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	SPFD54124_reset();

	/* Check Device Code */
	devicetype = SPFD54124_rd_cmd(RDID4);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0614)
	{
		/* Initialize SPFD54124 */
		SPFD54124_wr_cmd(SWRESET);		/* Sofeware setting */
		_delay_ms(10);
		SPFD54124_wr_cmd(SLPOUT);		/* Sleep out */
		_delay_ms(20);
		
		SPFD54124_wr_cmd(PWCTR1);		/* SPFD54124 Power Sequence */
		SPFD54124_wr_dat(0x07);   		/* default value  LCM=0 set t)e GVDD voltage=4.65  */
		SPFD54124_wr_dat(0x05);
		_delay_ms(10);
		
		SPFD54124_wr_cmd(VMCTR1); 
		SPFD54124_wr_dat(0xB4);			/* VCOM voltage set 4.300V  AGO IS C8H */
		SPFD54124_wr_dat(0x44);  
		_delay_ms(10); 
		
		SPFD54124_wr_cmd(VMCTR2);       
		SPFD54124_wr_dat(0x06);     	/* VCOMAC voltage set 5.550V  AGO IS 1FH */
		_delay_ms(10); 
		
		SPFD54124_wr_cmd(GAMCTRP1);		/* SPFD54124 Gamma Sequence */
		SPFD54124_wr_dat(0x00);
		SPFD54124_wr_dat(0x00);
		SPFD54124_wr_dat(0x00);
		SPFD54124_wr_dat(0x2E);
		SPFD54124_wr_dat(0x2C);
		SPFD54124_wr_dat(0x05);
		SPFD54124_wr_dat(0x1F);
		SPFD54124_wr_dat(0x02);
		SPFD54124_wr_dat(0x00);
		SPFD54124_wr_dat(0x06);
		SPFD54124_wr_dat(0x0F);   
		SPFD54124_wr_dat(0x0E);
		SPFD54124_wr_dat(0x01);
		SPFD54124_wr_dat(0x05);
		SPFD54124_wr_dat(0x05);
		SPFD54124_wr_dat(0x08);
		
		SPFD54124_wr_cmd(GAMCTRN1);		/* SPFD54124 Gamma Sequence */
		SPFD54124_wr_dat(0x00);
		SPFD54124_wr_dat(0x22);
		SPFD54124_wr_dat(0x24);
		SPFD54124_wr_dat(0x0A);
		SPFD54124_wr_dat(0x0A);
		SPFD54124_wr_dat(0x05);
		SPFD54124_wr_dat(0x02);
		SPFD54124_wr_dat(0x1F);
		SPFD54124_wr_dat(0x08);
		SPFD54124_wr_dat(0x05);
		SPFD54124_wr_dat(0x05);
		SPFD54124_wr_dat(0x01);
		SPFD54124_wr_dat(0x0E);
		SPFD54124_wr_dat(0x0F);
		SPFD54124_wr_dat(0x06);   
		SPFD54124_wr_dat(0x00);
		
		SPFD54124_wr_cmd(COLMOD); 		/* SPFD54124 Colour Mode */
		SPFD54124_wr_dat(0x05); 
		_delay_ms(10);
		
		SPFD54124_wr_cmd(MADCTL);		/* MX, MY, RGB mode */
		SPFD54124_wr_dat(0x00);
	 
		SPFD54124_wr_cmd(DISPON);		/* Display on */
		_delay_ms(10); 
	}

	else { for(;;);} /* Invalid Device Code!! */

	SPFD54124_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	SPFD54124_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		SPFD54124_wr_gram(COL_RED);
	} while (--n);


	_delay_ms(500);
	for(;;);

#endif

}


/* End Of File ---------------------------------------------------------------*/
