/********************************************************************************/
/*!
	@file			s6d0154.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        4.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive TFT1P2181-E TFT module(8/16bit mode).

    @section HISTORY
		2013.01.02	V1.00	Stable Release.
		2014.10.16	V2.00	Fixed 8-bit access bug.
		2023.05.01	V3.00	Removed unused delay function.
		2023.08.01	V4.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "s6d0154.h"
/* check header file version for fool proof */
#if S6D0154_H != 0x0400
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
inline void S6D0154_reset(void)
{
	S6D0154_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	S6D0154_RD_SET();
	S6D0154_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	S6D0154_RES_CLR();							/* RES=L, CS=L   			*/
	S6D0154_CS_CLR();

	_delay_ms(50);								/* wait 10ms     			*/
	S6D0154_RES_SET();						  	/* RES=H					*/
	_delay_ms(10);				    			/* wait 10ms     			*/
}


/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void S6D0154_wr_cmd(uint8_t cmd)
{
	S6D0154_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6D0154_CMD = 0;							/* upper 8bit command		*/
	S6D0154_WR();								/* WR=L->H					*/
#endif

	S6D0154_CMD = cmd;							/* cmd(8bit)				*/
	S6D0154_WR();								/* WR=L->H					*/

	S6D0154_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void S6D0154_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	S6D0154_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	S6D0154_WR();								/* WR=L->H					*/
	S6D0154_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	S6D0154_DATA = dat;							/* 16bit data 				*/
#endif
	S6D0154_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void S6D0154_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		S6D0154_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		S6D0154_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t S6D0154_rd_cmd(uint8_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	S6D0154_wr_cmd(cmd);
	S6D0154_WR_SET();

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


/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void S6D0154_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{
	S6D0154_wr_cmd(0x37);				/* Horizontal RAM Start ADDR */
	S6D0154_wr_dat(OFS_COL + x);
	S6D0154_wr_cmd(0x36);				/* Horizontal RAM End ADDR */
	S6D0154_wr_dat(OFS_COL + width);

	S6D0154_wr_cmd(0x39);				/* Vertical Start ADDR */
	S6D0154_wr_dat(OFS_RAW + y);
	S6D0154_wr_cmd(0x38);				/* Vertical End ADDR */
	S6D0154_wr_dat(OFS_RAW + height);

	S6D0154_wr_cmd(0x20);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	S6D0154_wr_dat(OFS_COL + x);
	S6D0154_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	S6D0154_wr_dat(OFS_RAW + y);

	S6D0154_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void S6D0154_clear(void)
{
	volatile uint32_t n;

	S6D0154_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		S6D0154_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void S6D0154_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	S6D0154_reset();
	
	/* Check Device Code */
	devicetype = S6D0154_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x0154)
	{
		/* Initialize S6D0154 */
		S6D0154_wr_cmd(0x80);
		S6D0154_wr_dat(0x008D);
		S6D0154_wr_cmd(0x92);
		S6D0154_wr_dat(0x0010);
		S6D0154_wr_cmd(0x11);		/* Power Control 2 */
		S6D0154_wr_dat(0x001C);
		S6D0154_wr_cmd(0x12);		/* Power Control 3  BT2-0, DC11-10, DC21-20, DC31-30 */
		S6D0154_wr_dat(0x4112);
		S6D0154_wr_cmd(0x13);		/* Power Control 4  DCR_EX=0, DCR2-0, GVD6-0 */
		S6D0154_wr_dat(0x0053);
		S6D0154_wr_cmd(0x14);		/* Power Control 5  VCOMG=0, VCM6-0, VCMR=0, VML6-0 */
		S6D0154_wr_dat(0x4269);
		S6D0154_wr_cmd(0x10);		/* Power Control 1  current consumption  STB */
		S6D0154_wr_dat(0x0800);
		S6D0154_wr_cmd(0x11);		/* Power Control 2 */
		S6D0154_wr_dat(0x011C);
		_delay_ms(10);
		
		S6D0154_wr_cmd(0x11);		/* Power Control 2 */
		S6D0154_wr_dat(0x031C);
		_delay_ms(10);
		
		S6D0154_wr_cmd(0x11);		/* Power Control 2 */
		S6D0154_wr_dat(0x071C);
		_delay_ms(10);
		
		S6D0154_wr_cmd(0x11);		/* Power Control 2 */
		S6D0154_wr_dat(0x0F1C);
		_delay_ms(20);
		
		S6D0154_wr_cmd(0x11);		/* Power Control 2 */
		S6D0154_wr_dat(0x0F3C);
		_delay_ms(30);
      
		S6D0154_wr_cmd(0x01);		/* Driver Output Control */
		S6D0154_wr_dat(0x0128);
		S6D0154_wr_cmd(0x02);		/* LCD-Driving-Waveform Control */
		S6D0154_wr_dat(0x0100);
		S6D0154_wr_cmd(0x03);		/* Entry Mode */
		S6D0154_wr_dat(0x1030);
		S6D0154_wr_cmd(0x07);		/* Blank Period Control 1 */
		S6D0154_wr_dat(0x1012);
		S6D0154_wr_cmd(0x08);		/* Frame Cycle Control */
		S6D0154_wr_dat(0x0303);
		S6D0154_wr_cmd(0x0B);		/* External Display Interface Control */
		S6D0154_wr_dat(0x1100);
		S6D0154_wr_cmd(0x0C);		/* General Input */
		S6D0154_wr_dat(0x0000);
		S6D0154_wr_cmd(0x0F);		/* Start Oscillation */
		S6D0154_wr_dat(0x1801);
		S6D0154_wr_cmd(0x15);		/* VCI Recycling (R15H) */
		S6D0154_wr_dat(0x0020);
		
		S6D0154_wr_cmd(0x50);		/*  Gamma Control */
		S6D0154_wr_dat(0x0101);
		S6D0154_wr_cmd(0x51);  
		S6D0154_wr_dat(0x0603);
		S6D0154_wr_cmd(0x52);  
		S6D0154_wr_dat(0x0408);
		S6D0154_wr_cmd(0x53);  
		S6D0154_wr_dat(0x0000);
		S6D0154_wr_cmd(0x54);  
		S6D0154_wr_dat(0x0605);
		S6D0154_wr_cmd(0x55);  
		S6D0154_wr_dat(0x0406);
		S6D0154_wr_cmd(0x56);  
		S6D0154_wr_dat(0x0303);
		S6D0154_wr_cmd(0x57);  
		S6D0154_wr_dat(0x0303);
		S6D0154_wr_cmd(0x58);  
		S6D0154_wr_dat(0x0010);
		S6D0154_wr_cmd(0x59);		/*  Gamma Control */
		S6D0154_wr_dat(0x1000);
		S6D0154_wr_cmd(0x07);		/*  Display Control */
		S6D0154_wr_dat(0x0012);
		_delay_ms(40);
       
		S6D0154_wr_cmd(0x07);		/*  GRAM Address Set */
		S6D0154_wr_dat(0x0013);
		S6D0154_wr_cmd(0x07);		/*  Display Control  DISPLAY ON */
		S6D0154_wr_dat(0x0017);
	}

	else { for(;;);} /* Invalid Device Code!! */

	S6D0154_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	S6D0154_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	do {
		S6D0154_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
