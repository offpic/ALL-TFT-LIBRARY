/********************************************************************************/
/*!
	@file			hx8353x.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        5.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					Available TFT-LCM are listed below.							@n
					 -NHD-1.8-128160TF-CTXI#	(HX8353D)	8bit mode only.		@n
					 -QDTFT1800LA				(HX8353C)	4-Wire Serial Only.	

    @section HISTORY
		2012.11.30	V1.00	Stable Release.
		2013.08.10  V2.00	Added HX8353C Devices.
		2014.10.15	V3.00	Fixed 8-bit access bug.
		2023.05.01	V4.00	Removed unused delay function.
		2023.08.01	V5.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "hx8353x.h"
/* check header file version for fool proof */
#if HX8353X_H != 0x0500
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
inline void HX8353x_reset(void)
{
#ifdef USE_HX8353x_TFT
	HX8353x_RES_SET();						/* RES=H, RD=H, WR=H   		*/
	HX8353x_RD_SET();
	HX8353x_WR_SET();
	_delay_ms(10);							/* wait 10ms     			*/

	HX8353x_RES_CLR();						/* RES=L, CS=L   			*/
	HX8353x_CS_CLR();

#elif  USE_HX8353x_SPI_TFT
	HX8353x_RES_SET();						/* RES=H, CS=H				*/
	HX8353x_CS_SET();
	HX8353x_SCLK_SET();						/* SPI MODE3     			*/
	_delay_ms(10);							/* wait 10ms     			*/

	HX8353x_RES_CLR();						/* RES=L, CS=L   			*/
	HX8353x_CS_CLR();

#endif

	_delay_ms(10);							/* wait 10ms     			*/
	HX8353x_RES_SET();						/* RES=H					*/
	_delay_ms(10);				    		/* wait 10ms     			*/
}

/* Select SPI or Parallel in MAKEFILE */
#ifdef USE_HX8353x_TFT
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8353x_wr_cmd(uint8_t cmd)
{
	HX8353x_DC_CLR();						/* DC=L						*/
	
	HX8353x_CMD = cmd;						/* cmd(8bit)				*/
	HX8353x_WR();							/* WR=L->H					*/
	
	HX8353x_DC_SET();						/* DC=H						*/
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void HX8353x_wr_dat(uint8_t dat)
{
	HX8353x_DATA = dat;						/* D7..D0=dat    */
	HX8353x_WR();							/* WR=L->H       */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8353x_wr_gram(uint16_t gram)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	HX8353x_DATA = (uint8_t)(gram>>8);		/* upper 8bit data		*/
	HX8353x_WR();							/* WR=L->H				*/
	HX8353x_DATA = (uint8_t)gram;			/* lower 8bit data		*/
#else
	HX8353x_DATA = gram;					/* 16bit data			*/
#endif
	HX8353x_WR();							/* WR=L->H				*/
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t HX8353x_rd_cmd(uint8_t cmd)
{
	uint8_t val;

	HX8353x_wr_cmd(cmd);
	HX8353x_WR_SET();

    ReadLCDData(val);				/* Dummy Read				*/
    ReadLCDData(val);				/* Lower Read				*/

	return val;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8353x_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else
	int n;

	n = cnt % 4;
	cnt /= 4;

	while (cnt--) {
		HX8353x_wr_dat(*p++);
		HX8353x_wr_dat(*p++);
		HX8353x_wr_dat(*p++);
		HX8353x_wr_dat(*p++);
	}
	while (n--) {
		HX8353x_wr_dat(*p++);
	}
#endif

}

#else /* USE_HX8353x_SPI_TFT */
/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void HX8353x_wr_cmd(uint8_t cmd)
{
	HX8353x_DC_CLR();							/* DC=L		     */
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	DNC_CMD();
	SendSPI(cmd);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
	HX8353x_DC_SET();							/* DC=H   	     */
}	

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
static inline void HX8353x_wr_sdat(uint8_t dat)
{	
	DNC_DAT();
#if defined(USE_HARDWARE_SPI) && defined(SUPPORT_HARDWARE_9BIT_SPI)
	SendSPID(dat);
#else
	SendSPI(dat);
#endif
}
inline void HX8353x_wr_dat(uint8_t dat)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	HX8353x_wr_sdat(dat);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Write LCD GRAM.
*/
/**************************************************************************/
inline void HX8353x_wr_gram(uint16_t gram)
{
	DISPLAY_ASSART_CS();						/* CS=L		     */
	
	HX8353x_wr_sdat((uint8_t)(gram>>8));
	HX8353x_wr_sdat((uint8_t)gram);

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint8_t HX8353x_rd_cmd(uint8_t cmd)
{
    #warning "HX8353x USES ONLY SDA(Input&Output Multiplexed) Line!"
	/* Read Function was NOT implemented SPI MODE! */
	return 0x53;
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void HX8353x_wr_block(uint8_t *p,unsigned int cnt)
{
	int n;
	
	n = cnt % 4;
	cnt /= 4;

	DISPLAY_ASSART_CS();						/* CS=L		     */

	while (cnt--) {
		HX8353x_wr_sdat(*p++);
		HX8353x_wr_sdat(*p++);
		HX8353x_wr_sdat(*p++);
		HX8353x_wr_sdat(*p++);
	}
	while (n--) {
		HX8353x_wr_sdat(*p++);
	}

	DISPLAY_NEGATE_CS();						/* CS=H		     */
}
#endif



/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void HX8353x_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	/* Set CAS Address */
	HX8353x_wr_cmd(0x2A); 
	HX8353x_wr_dat(0);
	HX8353x_wr_dat(OFS_COL + x);
	HX8353x_wr_dat(0);
	HX8353x_wr_dat(OFS_COL + width);
	
	/* Set RAS Address */
	HX8353x_wr_cmd(0x2B);
	HX8353x_wr_dat(0);
	HX8353x_wr_dat(OFS_RAW + y); 
	HX8353x_wr_dat(0);
	HX8353x_wr_dat(OFS_RAW + height); 
	
	/* Write RAM */
	HX8353x_wr_cmd(0x2C);
}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void HX8353x_clear(void)
{
	volatile uint32_t n;

	HX8353x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (MAX_X) * (MAX_Y);

	do {
		/* 16Bit Colour Access */
		HX8353x_wr_gram(COL_BLACK);
	} while (--n);
}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void HX8353x_init(void)
{
	uint8_t id8353d,id8353c=0;
	
	Display_IoInit_If();

	HX8353x_reset();

	/* Enter Extension Command */
	HX8353x_wr_cmd(0xB9);		
	HX8353x_wr_dat(0xFF);		/* Followed by 3 extension parameters (for HX8353D) */
	HX8353x_wr_dat(0x83);
	HX8353x_wr_dat(0x53);

	/* Check Device Code */
	id8353c = HX8353x_rd_cmd(0xF4);  			/* Confirm Vaild LCD Controller */
	id8353d = HX8353x_rd_cmd(0xD0);  			/* Confirm Vaild LCD Controller */

	if((id8353d == 0x64) || (id8353d == 0x05) || (id8353c == 0x53))
	{
		/* Initialize HX8353C/D */
		HX8353x_wr_cmd(0x01);		/* Soft Reset */
		_delay_ms(150);				/* At least 120mSec */

		HX8353x_wr_cmd(0xB9);		/* Enter Extension Command */
		HX8353x_wr_dat(0xFF);		/* Followed by 3 extension parameters For HX8353D */
		HX8353x_wr_dat(0x83);
		HX8353x_wr_dat(0x53);

		HX8353x_wr_cmd(0x28);		/* Display OFF */

		HX8353x_wr_cmd(0x36);		/* Memory access control */

		if(id8353c == 0x53){ /* HX8353C */
			HX8353x_wr_dat((1<<7)|(1<<6)|(0<<5)|(1<<4)|(0<<3)|(1<<2));
			}
		else { /* HX8353D */
			HX8353x_wr_dat((0<<7)|(0<<6)|(0<<5)|(0<<4)|(1<<3)|(0<<2));
		}

		HX8353x_wr_cmd(0x11);		/* Sleep out */
		_delay_ms(150);				/* At least 120mSec */

		HX8353x_wr_cmd(0xC6);		/* Set UADJ to trimming frequency */
		HX8353x_wr_dat(0x31);

		HX8353x_wr_cmd(0xB5);		/* Set Power Control */
		HX8353x_wr_dat(0x0B);
		HX8353x_wr_dat(0x0B);
		HX8353x_wr_dat(0x07);

		HX8353x_wr_cmd(0xB1);		/* Set Power Control */
		HX8353x_wr_dat(0x00);
		HX8353x_wr_dat(0x00);

		HX8353x_wr_cmd(0xB2);		/* Set Display Control */
		HX8353x_wr_dat(0x00);

		HX8353x_wr_cmd(0xB6);		/* Set power control */
		HX8353x_wr_dat(0x84);
		HX8353x_wr_dat(0x6C);
		HX8353x_wr_dat(0x74);

		HX8353x_wr_cmd(0xE0);		/* Set Gamma */
		HX8353x_wr_dat(0x00);
		HX8353x_wr_dat(0x70);
		HX8353x_wr_dat(0x00);
		HX8353x_wr_dat(0x0D);
		HX8353x_wr_dat(0x3F);
		HX8353x_wr_dat(0x03);
		HX8353x_wr_dat(0x0C);
		HX8353x_wr_dat(0x00);
		HX8353x_wr_dat(0x00);
		HX8353x_wr_dat(0x05);
		HX8353x_wr_dat(0x77);
		HX8353x_wr_dat(0x70);
		HX8353x_wr_dat(0x77);
		HX8353x_wr_dat(0x0C);
		HX8353x_wr_dat(0x33);
		HX8353x_wr_dat(0x0F);
		HX8353x_wr_dat(0x0D);
		HX8353x_wr_dat(0x00);
		HX8353x_wr_dat(0x00);

		HX8353x_wr_cmd(0x2D);		/* RGB LUT Set For 16Bppp Handling */
		int m=0;
		for(int i=1;i<=32;i++)
		{
			HX8353x_wr_dat(m);
			m = m+2;
        }
		m=0;
		for(int i=1;i<=64;i++)
		{
			HX8353x_wr_dat(m);
			++m;
        }
		m=0;
		for(int i=1;i<=32;i++)
		{
			HX8353x_wr_dat(m);
			m = m+2;
        }

		HX8353x_wr_cmd(0x3A);		/* Interface pixel format */
		HX8353x_wr_dat(0x05);		/* 8-bit 8080 mode, 565-16bit */

		HX8353x_wr_cmd(0x29);		/* Display ON */

	}
	else { for(;;);} /* Invalid Device Code!! */

	HX8353x_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	HX8353x_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	
	do {
		HX8353x_wr_gram(COL_RED);
	} while (--n);


	_delay_ms(500);
	for(;;);

#endif

}


/* End Of File ---------------------------------------------------------------*/
