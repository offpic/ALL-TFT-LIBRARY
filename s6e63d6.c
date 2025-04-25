/********************************************************************************/
/*!
	@file			s6e63d6.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        6.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available OLED Modules are listed below.					@n
					 -BL024ACRNB$				(S6E63D6)	8/16bit,spi mode.	@n
					 -C0283QGLH-T				(S6E63D6)	8/16bit,spi mode.	@n

    @section HISTORY
		2011.05.22	V1.00	Stable Release.
		2011.10.25	V2.00	Added DMA TransactionSupport.
		2011.12.23	V3.00	Optimize Some Codes.
		2014.10.15	V4.00	Fixed 8-bit access bug.
		2023.05.01	V5.00	Removed unused delay function.
		2023.08.01	V6.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s6e63d6.h"
/* check header file version for fool proof */
#if S6E63D6_H != 0x0600
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
inline void S6E63D6_reset(void)
{
#ifdef USE_S6E63D6_OLED
	S6E63D6_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S6E63D6_RD_SET();
	S6E63D6_WR_SET();
	_delay_ms(100);								/* wait 100ms     			*/

	S6E63D6_RES_CLR();							/* RES=L, CS=L   			*/
	S6E63D6_CS_CLR();

#elif  USE_S6E63D6_SPI_OLED
	S6E63D6_RES_SET();							/* RES=H, CS=H				*/
	S6E63D6_CS_SET();
	S6E63D6_SCK_SET();							/* SPI MODE3     			*/
	_delay_ms(100);								/* wait 100ms     			*/

	S6E63D6_RES_CLR();							/* RES=L		   			*/

#endif

	_delay_ms(10);								/* wait 10ms     			*/
	S6E63D6_RES_SET();						  	/* RES=H					*/
	_delay_ms(50);				    			/* wait 50ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_S6E63D6_OLED
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6E63D6_wr_cmd(uint8_t cmd)
{
	S6E63D6_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6E63D6_CMD = 0;
	S6E63D6_WR();
#endif

	S6E63D6_CMD = (uint8_t)cmd;					/* cmd(8bit_Low or 16bit)	*/
	S6E63D6_WR();								/* WR=L->H					*/

	S6E63D6_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void S6E63D6_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6E63D6_DATA = (uint8_t)(dat>>8);			/* upper 8bit data		*/
	S6E63D6_WR();								/* WR=L->H				*/
	S6E63D6_DATA = (uint8_t)dat;				/* lower 8bit data		*/
#else
	S6E63D6_DATA = dat;							/* 16bit data			*/
#endif
    S6E63D6_WR();								/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6E63D6_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		S6E63D6_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		S6E63D6_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t S6E63D6_rd_cmd(uint8_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	S6E63D6_wr_cmd(cmd);
	S6E63D6_WR_SET();

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
    ReadLCDData(temp);
#endif

    ReadLCDData(val);

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	val &= 0x00FF;
	val |= temp<<8;
#endif

	return val;
}


#elif USE_S6E63D6_SPI_OLED
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6E63D6_wr_cmd(uint8_t cmd)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI16(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Data.
*/
/**************************************************************************/
inline void S6E63D6_wr_dat(uint16_t dat)
{	
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_DATA);
	SendSPI16(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6E63D6_wr_block(uint8_t *p,unsigned int cnt)
{

	DISPLAY_ASSART_CS();						/* CS=L		     */
	SendSPI(START_WR_DATA);

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
inline  uint16_t S6E63D6_rd_cmd(uint8_t cmd)
{
	uint16_t val;
	uint8_t temp;

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_WR_CMD);
	SendSPI16(cmd);
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */
	

	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	SendSPI(START_RD_DATA);
	temp = RecvSPI();							/* Upper Read    */
	val  = RecvSPI();							/* Lower Read    */

	val &= 0x00FF;
	val |= (uint16_t)temp<<8;
	
	DISPLAY_NEGATE_CS();						/* CS=H		     */

	return val;
}
#endif


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void S6E63D6_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	S6E63D6_wr_cmd(0x37);				/* Horizontal RAM Start and End ADDR */
	S6E63D6_wr_dat(((OFS_COL + x)<<8)|(OFS_COL + width));

	S6E63D6_wr_cmd(0x35);				/* Vertical RAM Start ADDR */
	S6E63D6_wr_dat(OFS_RAW + y);
	S6E63D6_wr_cmd(0x36);				/* Vertical End ADDR */
	S6E63D6_wr_dat(OFS_RAW + height);

	S6E63D6_wr_cmd(0x20);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	S6E63D6_wr_dat(OFS_COL + x);
	S6E63D6_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	S6E63D6_wr_dat(OFS_RAW + y);

	S6E63D6_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S6E63D6_clear(void)
{
	volatile uint32_t n;

	S6E63D6_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6E63D6_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S6E63D6_init(void)
{
	volatile uint16_t devicetype;
	
	Display_IoInit_If();

	S6E63D6_reset();

	/* Check Device Code */
	devicetype = S6E63D6_rd_cmd(0x0F);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x63D6)
	{
		/* Initialize S6E63D6 */
	#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
		S6E63D6_wr_cmd(0x23);
	#else
		S6E63D6_wr_cmd(0x24);	
	#endif
	
		S6E63D6_wr_cmd(0x10);						/* Sleep Off */
		S6E63D6_wr_dat(0x0000); 	
		_delay_ms(120);								/* wait at least 120mSec */
		
		S6E63D6_wr_cmd(0x02);
		S6E63D6_wr_dat(0x0000); 					/* system interface */
		
		S6E63D6_wr_cmd(0x03);
		S6E63D6_wr_dat((0<<15)|(0<<14)|(0<<13)|(0<<12)|(1<<8)|(1<<5)|(1<<4)|(0<<0));
		
		S6E63D6_wr_cmd(0x70);						/* Diaplay Gamma */
		S6E63D6_wr_dat(0x2000);
		S6E63D6_wr_cmd(0x71);
		S6E63D6_wr_dat(0x2180);
		S6E63D6_wr_cmd(0x72);
		S6E63D6_wr_dat(0x2D80);
		S6E63D6_wr_cmd(0x73);
		S6E63D6_wr_dat(0x1512);
		S6E63D6_wr_cmd(0x74);
		S6E63D6_wr_dat(0x1C10);
		S6E63D6_wr_cmd(0x75);
		S6E63D6_wr_dat(0x2216);
		S6E63D6_wr_cmd(0x76); 
		S6E63D6_wr_dat(0x1813); 
		S6E63D6_wr_cmd(0x77);
		S6E63D6_wr_dat(0x2219); 
		S6E63D6_wr_cmd(0x78);
		S6E63D6_wr_dat(0x2014);
		
		S6E63D6_wr_cmd(0x18); 					/* Frame Rate=80Hz */
		S6E63D6_wr_dat(0x0028);
		
		S6E63D6_wr_cmd(0xF8);						/* VGH 6 VGL Settings */
		S6E63D6_wr_dat(0x000F);
		S6E63D6_wr_cmd(0xF9);
		S6E63D6_wr_dat(0x000F);
		
		S6E63D6_wr_cmd(0x05); 					/* Display control1, display on */
		S6E63D6_wr_dat(0x0001);
		
		S6E63D6_wr_cmd(0x22);
	}

	else { for(;;);} /* Invalid Device Code!! */

	S6E63D6_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	S6E63D6_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6E63D6_wr_dat(COL_BLUE);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
