/********************************************************************************/
/*!
	@file			st7789v2.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2024.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -ATM0130B3				(ST7789V2)	(8bit4wire serial only)

    @section HISTORY
		2023.07.01	V1.00	Stable release.
		2023.08.01	V2.00	Revised release.
		2023.09.01	V3.00	Fixed DDRAM address set.
		2024.08.01	V4.00	Fixed unused parameter fix.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "st7789v2.h"
/* check header file version for fool proof */
#if ST7789V2_H != 0x0400
#error "header file version is not correspond!"
#endif

/* Defines -------------------------------------------------------------------*/
#if defined(USE_ATM0130B3)
 #warning "Using ATM0130B3 240x240 module!"
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
inline void ST7789V2_reset(void)
{
#ifdef USE_ST7789V2_TFT
	ST7789V2_RES_SET();							/* RES=H, RD=H, WR=H   	*/
	ST7789V2_RD_SET();
	ST7789V2_WR_SET();
	_delay_ms(1);								/* wait 1ms     		*/

	ST7789V2_RES_CLR();							/* RES=L, CS=L   		*/
	ST7789V2_CS_CLR();

#elif  USE_ST7789V2_SPI_TFT
	ST7789V2_RES_SET();							/* RES=H, CS=H			*/
	ST7789V2_CS_SET();
	ST7789V2_SCLK_SET();						/* SPI MODE3     		*/
	_delay_ms(1);								/* wait 1ms     		*/

	ST7789V2_RES_CLR();							/* RES=L, CS=L   		*/
	ST7789V2_CS_CLR();

#endif

	_delay_ms(20);								/* wait 20ms     		*/
	ST7789V2_RES_SET();							/* RES=H				*/
	_delay_ms(20);				    			/* wait 20ms     		*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_ST7789V2_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ST7789V2_wr_cmd(uint8_t cmd)
{
	ST7789V2_DC_CLR();							/* DC=L		     */
	
	ST7789V2_CMD = cmd;							/* D7..D0=cmd    */
	ST7789V2_WR();								/* WR=L->H       */
	
	ST7789V2_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ST7789V2_wr_dat(uint8_t dat)
{
	ST7789V2_DATA = dat;						/* D7..D0=dat    */
	ST7789V2_WR();								/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void ST7789V2_wr_gram(uint16_t gram)
{
	ST7789V2_DATA = (uint8_t)(gram>>8);			/* upper 8bit data		*/
	ST7789V2_WR();								/* WR=L->H				*/

	ST7789V2_DATA = (uint8_t)gram;				/* lower 8bit data		*/
	ST7789V2_WR();								/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ST7789V2_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		ST7789V2_wr_dat(*p++);
		ST7789V2_wr_dat(*p++);
		ST7789V2_wr_dat(*p++);
		ST7789V2_wr_dat(*p++);
	}
	while (n--) {
		ST7789V2_wr_dat(*p++);
	}
#endif

}

#else /* USE_ST7789V2_SPI_TFT */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void ST7789V2_wr_cmd(uint8_t cmd)
{
	ST7789V2_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	ST7789V2_DC_SET();							/* DC=H		     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void ST7789V2_wr_dat(uint8_t dat)
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
inline void ST7789V2_wr_gram(uint16_t gram)
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
inline uint8_t ST7789V2_rd_cmd(uint8_t cmd)
{
#ifdef ST7789V2_SPI_4WIRE_READID_IGNORE
 #warning "Ingnore ST7789V2 RDID2 check!"
	(void)cmd;
	return 0x85;
#else	
 #warning "ST7789V2 USES SDA(Input&Output Multiplexed) Line!"
 	uint8_t temp;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	ST7789V2_DC_CLR();							/* DC=L		     */
	SendSPI(cmd);
	ST7789V2_DC_SET();							/* DC=L		     */

	Display_ChangeSDA_If(TFT_SDA_READ);			/* Change SDI to Input HI */
	temp  = RecvSPI();
	Display_ChangeSDA_If(TFT_SDA_WRITE);		/* Change SDI to Ouput */
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */
	return temp;
#endif
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void ST7789V2_wr_block(uint8_t *p,unsigned int cnt)
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
inline void ST7789V2_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	ST7789V2_wr_cmd(CASET); 
	ST7789V2_wr_dat((OFS_COL + x)>>8);
	ST7789V2_wr_dat(OFS_COL + x);
	ST7789V2_wr_dat((OFS_COL + width)>>8);
	ST7789V2_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	ST7789V2_wr_cmd(RASET);
	ST7789V2_wr_dat((OFS_RAW + y)>>8);
	ST7789V2_wr_dat(OFS_RAW + y); 
	ST7789V2_wr_dat((OFS_RAW + height)>>8);
	ST7789V2_wr_dat(OFS_RAW + height);
	
	/* Write RAM */
	ST7789V2_wr_cmd(RAMWR);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void ST7789V2_clear(void)
{
	volatile uint32_t n;

	ST7789V2_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		ST7789V2_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void ST7789V2_init(void)
{
	uint8_t devicetype;

	Display_IoInit_If();

	ST7789V2_reset();
	
	/* Read device code */
	devicetype = ST7789V2_rd_cmd(RDID2);

	if(devicetype == 0x85)
	{
		/* Initialize ST7789V2 */
		ST7789V2_wr_cmd(SWRESET);   	/* Software setting */
		_delay_ms(10);
		
		ST7789V2_wr_cmd(SLPOUT);		/* Sleep out */
		_delay_ms(120);
		
		ST7789V2_wr_cmd(FRMCTR1);		/* ST7789V2 Frame Rate */
		ST7789V2_wr_dat(0x0C);
		ST7789V2_wr_dat(0x0C);
		ST7789V2_wr_dat(0x00);
		ST7789V2_wr_dat(0x33);
		ST7789V2_wr_dat(0x33);
		_delay_ms(2);
		
		ST7789V2_wr_cmd(GATECTRL);		/* Gate setting */
		ST7789V2_wr_dat(0x75);
		_delay_ms(2);
		
		ST7789V2_wr_cmd(VDVVRHEN);		/* VDV and VRH command enable */
		ST7789V2_wr_dat(0x01);
		ST7789V2_wr_dat(0xFF);
		_delay_ms(2);
		
		ST7789V2_wr_cmd(VRHS);			/* VRH set */
		ST7789V2_wr_dat(0x10);
		_delay_ms(2);
		
		ST7789V2_wr_cmd(VDVSET);		/* VDV set */
		ST7789V2_wr_dat(0x20);
		_delay_ms(2);
		
		ST7789V2_wr_cmd(FRCTR2);		/* Frame control2 */
		ST7789V2_wr_dat(0x0F);
		
		ST7789V2_wr_cmd(RAMCTRL);		/* RAM control */
		ST7789V2_wr_dat(0x00);
		ST7789V2_wr_dat(0xF0);			/* RGB565 */
		_delay_ms(2);
		
		ST7789V2_wr_cmd(PWCTRL1);		/* RAM control */
		ST7789V2_wr_dat(0xA4);
		ST7789V2_wr_dat(0xA1);
		_delay_ms(2);
		
		/* Gamma settings */
		ST7789V2_wr_cmd(INVON);			/* Display inversion on */
		_delay_ms(2);
		
		ST7789V2_wr_cmd(VCOMS);			/* VCOM setting */
		ST7789V2_wr_dat(0x3B);
		
		ST7789V2_wr_cmd(PVGAMCTRL);		/* Set positive gamma */
		ST7789V2_wr_dat(0xF0);
		ST7789V2_wr_dat(0x0B);
		ST7789V2_wr_dat(0x11);
		ST7789V2_wr_dat(0x0E);
		ST7789V2_wr_dat(0x0D);
		ST7789V2_wr_dat(0x19);
		ST7789V2_wr_dat(0x36);
		ST7789V2_wr_dat(0x33);
		ST7789V2_wr_dat(0x4B);
		ST7789V2_wr_dat(0x07);
		ST7789V2_wr_dat(0x14);
		ST7789V2_wr_dat(0x14);
		ST7789V2_wr_dat(0x2C);
		ST7789V2_wr_dat(0x2E);
		_delay_ms(2);
		
		ST7789V2_wr_cmd(NVGAMCTRL);		/* Set negative gamma */
		ST7789V2_wr_dat(0xF0);
		ST7789V2_wr_dat(0x0D);
		ST7789V2_wr_dat(0x12);
		ST7789V2_wr_dat(0x0B);
		ST7789V2_wr_dat(0x09);
		ST7789V2_wr_dat(0x03);
		ST7789V2_wr_dat(0x32);
		ST7789V2_wr_dat(0x44);
		ST7789V2_wr_dat(0x48);
		ST7789V2_wr_dat(0x39);
		ST7789V2_wr_dat(0x16);
		ST7789V2_wr_dat(0x16);
		ST7789V2_wr_dat(0x2D);
		ST7789V2_wr_dat(0x30);
		_delay_ms(2);
		
		ST7789V2_wr_cmd(CASET);			/* ST7789V2 Column Address Init */
		ST7789V2_wr_dat(0x00);
		ST7789V2_wr_dat(0x00);
		ST7789V2_wr_dat(0x00);
		ST7789V2_wr_dat(0xEF);
		
		ST7789V2_wr_cmd(RASET);			/* ST7789V2 Raw Address Init */
		ST7789V2_wr_dat(0x00);
		ST7789V2_wr_dat(0x00);
		ST7789V2_wr_dat(0x00);
		ST7789V2_wr_dat(0xEF);
		
		ST7789V2_wr_cmd(COLMOD); 		/* ST7789V2 Colour Mode */
		ST7789V2_wr_dat(0x55); 
	 
		ST7789V2_wr_cmd(DISPON);		/* Display on */
		_delay_ms(10); 
	}

	else { for(;;);} /* Invalid Device Code!! */

	ST7789V2_clear();					/* Clear GRAM */

#if 0	/* test code RED */
	volatile uint32_t n;

	ST7789V2_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		ST7789V2_wr_gram(COL_RED);
	} while (--n);


	_delay_ms(500);
	for(;;);

#endif
}

/* End Of File ---------------------------------------------------------------*/
