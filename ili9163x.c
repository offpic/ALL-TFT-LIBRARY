/********************************************************************************/
/*!
	@file			ili9163x.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -SGP18T-00		(ILI9163B)	4-Wire,8-bitSerial.				@n
					 -SDT018ATFT	(ILI9163C)	8/16bit & 3-Wire,9bitSerial.	@n
					 -S93235Z		(ILI9163B)	4-Wire,8-bitSerial.				@n
					 -S93160		(ILI9163B)	8bit mode only.

    @section HISTORY
		2011.12.23	V1.00	Revised From ILI9163B Driver.
		2012.01.18	V2.00	Added S93235Z Module Support.
		2012.05.25  V3.00	Added Hardware 9-bitSerial Handling.
		2012.08.31  V4.00	Added S93160 Module Support.
		2023.05.01	V5.00	Removed unused delay function.
		2023.08.01	V6.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ili9163x.h"
/* check header file version for fool proof */
#if ILI9163X_H != 0x0600
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#if defined(USE_S93235Z)
 #warning "You Chose S93235Z V01 Module(ILI9163B)!"
#elif  defined(USE_SGP18T_00)
 #warning "You Chose SGP18T-00 Module(ILI9163B)!"
#elif  defined(USE_SDT018ATFT)
 #warning "You Chose SDT018ATFT Module(ILI9163C)!"
#elif  defined(USE_S93160)
 #warning "You Chose S93160 Module(ILI9163B)!"
#else
 #error "U MUST select ILI9163x LCD Molule Model at first!."
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
inline void ILI9163x_reset(void)
{
#ifdef USE_ILI9163x_TFT
	ILI9163x_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	ILI9163x_RD_SET();
	ILI9163x_WR_SET();
	_delay_ms(1);								/* wait 1ms     			*/

	ILI9163x_RES_CLR();							/* RES=L, CS=L   			*/
	ILI9163x_CS_CLR();

#elif  USE_ILI9163x_SPI_TFT
	ILI9163x_RES_SET();							/* RES=H, CS=H				*/
	ILI9163x_CS_SET();
	ILI9163x_SCLK_SET();						/* SPI MODE3     			*/
	_delay_ms(1);								/* wait 1ms     			*/

	ILI9163x_RES_CLR();							/* RES=L, CS=L   			*/
	ILI9163x_CS_CLR();

#endif

	_delay_ms(20);								/* wait 20ms     			*/
	ILI9163x_RES_SET();						  	/* RES=H					*/
	_delay_ms(125);				    			/* wait at least 120 ms     */
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_ILI9163x_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9163x_wr_cmd(uint8_t cmd)
{
	ILI9163x_DC_CLR();							/* DC=L		     */
	
	ILI9163x_CMD = cmd;							/* D7..D0=cmd    */
	ILI9163x_WR();								/* WR=L->H       */
	
	ILI9163x_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ILI9163x_wr_dat(uint8_t dat)
{
	ILI9163x_DATA = dat;						/* D7..D0=dat    */
	ILI9163x_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ILI9163x_wr_gram(uint16_t gram)
{
	ILI9163x_DATA = (uint8_t)(gram>>8);			/* upper 8bit data			*/
	ILI9163x_WR();								/* WR=L->H					*/

	ILI9163x_DATA = (uint8_t)gram;				/* lower 8bit data			*/
	ILI9163x_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t ILI9163x_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint16_t temp;

	ILI9163x_wr_cmd(cmd);
	ILI9163x_WR_SET();

    ReadLCDData(temp);							/* Dummy Read				*/
    ReadLCDData(temp);							/* Upper Read				*/
    ReadLCDData(val);							/* Lower Read				*/

	val &= 0x00FF;
	val |= temp<<8;

	return val;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9163x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		ILI9163x_wr_dat(*p++);
		ILI9163x_wr_dat(*p++);
		ILI9163x_wr_dat(*p++);
		ILI9163x_wr_dat(*p++);
	}
	while (n--) {
		ILI9163x_wr_dat(*p++);
	}
#endif

}

#else /* USE_ILI9163x_SPI_TFT */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ILI9163x_wr_cmd(uint8_t cmd)
{
	ILI9163x_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	DNC_CMD();
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	ILI9163x_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
static inline void ILI9163x_wr_sdat(uint8_t dat)
{	
	DNC_DAT();
#if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
	SendSPID(dat);
#else
	SendSPI(dat);
#endif
}
inline void ILI9163x_wr_dat(uint8_t dat)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	ILI9163x_wr_sdat(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}


/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ILI9163x_wr_gram(uint16_t gram)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */

#if defined(ILI9163xSPI_4WIREMODE)
	SendSPI16(gram);
#else
	ILI9163x_wr_sdat((uint8_t)(gram>>8));
	ILI9163x_wr_sdat((uint8_t)gram);
#endif
	DISPLAY_NEGATE_CS();						/* CS=H		     */
}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t ILI9163x_rd_cmd(uint8_t cmd)
{
	uint8_t val;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	ILI9163x_DC_CLR();							/* DC=L		     */
	SendSPI(cmd);
	ILI9163x_DC_SET();							/* DC=L		     */

	Display_ChangeSDA_If(TFT_SDA_READ);			/* Change SDI to Input HI */
	val = RecvSPI();							/* Lower Read 	*/
	Display_ChangeSDA_If(TFT_SDA_WRITE);		/* Change SDI to Ouput */
	
	DISPLAY_NEGATE_CS();						/* CS=H		    */

	return val;
}

/**************************************************************************/
/*! 
    Read ID ILI9163x.
*/
/**************************************************************************/
static uint16_t ILI9163x_rd_id(uint8_t cmd)
{
#if defined(ILI9163xSPI_4WIREMODE)
 #ifdef ILI9163x_SPI_4WIRE_READID_IGNORE
  #warning "Ingnore ILI9163x RDID4 check!"
	return 0x9163;
 #else	
  #warning "ILI9163x USES SDA(Input&Output Multiplexed) Line!"
	uint16_t val;
	uint8_t temp;

	ILI9163x_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI9163x_wr_dat(0x10);    					/* Read Mode Enable,1st Byte */
	temp = ILI9163x_rd_cmd(cmd);				/* Dummy Read 	*/
	
	ILI9163x_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI9163x_wr_dat(0x11);    					/* Read Mode Enable,2rd Byte */
	temp = ILI9163x_rd_cmd(cmd);				/* Upper Read 	*/

	ILI9163x_wr_cmd(0xD9);						/* SPI Register Read Command */
	ILI9163x_wr_dat(0x12);    					/* Read Mode Enable,3rd Byte */
	val  = ILI9163x_rd_cmd(cmd);				/* Lower Read	*/

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;

	return val;
 #endif

#else
	/* Read Function was NOT implemented in 9-bit SPI-MODE! */
	return 0x9163;
#endif
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ILI9163x_wr_block(uint8_t *p,unsigned int cnt)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;
	
	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		ILI9163x_wr_sdat(*p++);
		ILI9163x_wr_sdat(*p++);
		ILI9163x_wr_sdat(*p++);
		ILI9163x_wr_sdat(*p++);
	}
	while (n--) {
		ILI9163x_wr_sdat(*p++);
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
inline void ILI9163x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	ILI9163x_wr_cmd(CASET); 
	ILI9163x_wr_dat(0);
	ILI9163x_wr_dat(OFS_COL + x);
	ILI9163x_wr_dat(0);
	ILI9163x_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	ILI9163x_wr_cmd(RASET);
	ILI9163x_wr_dat(0);
	ILI9163x_wr_dat(OFS_RAW + y); 
	ILI9163x_wr_dat(0);
	ILI9163x_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	ILI9163x_wr_cmd(RAMWR);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ILI9163x_clear(void)
{
	volatile uint32_t n;

	ILI9163x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		ILI9163x_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ILI9163x_init(void)
{
	uint16_t devicetype;
	
	Display_IoInit_If();

	ILI9163x_reset();

	/* Check Device Code */
	devicetype = ILI9163x_rd_id(RDID4);  			/* Confirm Vaild LCD Controller */

	if((devicetype == 0x9163) || (devicetype == 0x0121))
	{
		/* Initialize ILI9163x */
		ILI9163x_wr_cmd(SWRESET);		/* Software Reset */
		_delay_ms(10);
		
		ILI9163x_wr_cmd(SLPOUT);		/* Sleep Out */
		_delay_ms(100);
		
		ILI9163x_wr_cmd(COLMOD);		/* Interface Pixel Format */
		ILI9163x_wr_dat(0x05);			/* 16-bit/pixel RGB565 */
		
		ILI9163x_wr_cmd(GAMSET);		/* Set Default GAMMA */
		ILI9163x_wr_dat(0x04);
		
		ILI9163x_wr_cmd(GAM_R_SEL);		/* E0h(GAMCTRP0) & E1h(GAMCTRN0) Command Enable */
		ILI9163x_wr_dat(0x01);
		
		ILI9163x_wr_cmd(GAMCTRP0);		/* Gamma e+fpolarity Correction Characteristics Setting */
		ILI9163x_wr_dat(0x3F);
		ILI9163x_wr_dat(0x25);
		ILI9163x_wr_dat(0x1C);
		ILI9163x_wr_dat(0x1E);
		ILI9163x_wr_dat(0x20);
		ILI9163x_wr_dat(0x12);
		ILI9163x_wr_dat(0x2A);
		ILI9163x_wr_dat(0x90);
		ILI9163x_wr_dat(0x24);
		ILI9163x_wr_dat(0x11);
		ILI9163x_wr_dat(0x00);
		ILI9163x_wr_dat(0x00);
		ILI9163x_wr_dat(0x00);
		ILI9163x_wr_dat(0x00);
		ILI9163x_wr_dat(0x00);
		
		ILI9163x_wr_cmd(GAMCTRN0);		/* Gamma e-fpolarity Correction Characteristics Setting */
		ILI9163x_wr_dat(0x20);
		ILI9163x_wr_dat(0x20);
		ILI9163x_wr_dat(0x20);
		ILI9163x_wr_dat(0x20);
		ILI9163x_wr_dat(0x05);
		ILI9163x_wr_dat(0x00);
		ILI9163x_wr_dat(0x15);
		ILI9163x_wr_dat(0xA7);
		ILI9163x_wr_dat(0x3D);
		ILI9163x_wr_dat(0x18);
		ILI9163x_wr_dat(0x25);
		ILI9163x_wr_dat(0x2A);
		ILI9163x_wr_dat(0x2B);
		ILI9163x_wr_dat(0x2B);  
		ILI9163x_wr_dat(0x3A); 
		
		ILI9163x_wr_cmd(FRMCTR1);		/* Set the frame frequency of the full colors normal mode. */
										/* Frame rate=fosc(200kHz)/((DIVA+4) x (LINE + VPA)) */
		ILI9163x_wr_dat(0x08);			/* Set DIVA */
		ILI9163x_wr_dat(0x08);			/* Set VPA */

		ILI9163x_wr_cmd(INVCTR);		/* Set Display Inversion Control */
		ILI9163x_wr_dat(0x07);			/* Set to Flame inversion */
		
		ILI9163x_wr_cmd(PWCTR1);		/* Power Control 0 */
		ILI9163x_wr_dat(PWCTR1VAL);
		ILI9163x_wr_dat(0x02);
		
		ILI9163x_wr_cmd(PWCTR2);		/* Power Control 1 */
		ILI9163x_wr_dat(0x02);
		
		ILI9163x_wr_cmd(VMCTR1);		/* VCOM Control 1 */
		ILI9163x_wr_dat(VMCTR1VALH);
		ILI9163x_wr_dat(VMCTR1VALL);
		
		ILI9163x_wr_cmd(VMOFCTR);		/* VCOM Offset Control */
		ILI9163x_wr_dat(VMOFCTRVAL);
		
		ILI9163x_wr_cmd(MADCTL);		/* Memory Data Access Control */
										/* -This command defines read/ write scanning direction of frame memory. */
		ILI9163x_wr_dat(MADVAL); 		/* MY=1,MX=1,MV=0,ML=0(LCD vertical refresh Top to Bottom) */
										/* Color selector switch control :RGB color filter panel */
										/* LCD horizontal refresh Left to right */
										
		ILI9163x_wr_cmd(SRCINVCTR);		/* Source Driver Direction Control */
		ILI9163x_wr_dat(0x00);
		
		ILI9163x_wr_cmd(DISPON);		/* Display On */
	}

	else { for(;;);} /* Invalid Device Code!! */

	ILI9163x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	ILI9163x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		ILI9163x_wr_gram(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
