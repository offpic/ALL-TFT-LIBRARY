/********************************************************************************/
/*!
	@file			lgdp4551.c
	@author         Nemui Trinomius (http://nemuisan.blog.bai.ne.jp)
    @version        3.00
    @date           2023.08.01
	@brief          Based on Chan's MCI_OLED@LPC23xx-demo thanks!				@n
					It can drive WS-TFT30L05T-3B TFT module(8bit mode).

    @section HISTORY
		2014.08.03	V1.00	Stable Release
		2023.05.01	V2.00	Removed unused delay function.
		2023.08.01	V3.00	Revised release.

    @section LICENSE
		BSD License. See Copyright.txt
*/
/********************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "lgdp4551.h"
/* check header file version for fool proof */
#if LGDP4551_H != 0x0300
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
inline void LGDP4551_reset(void)
{
	LGDP4551_RES_SET();							/* RES=H, RD=H, WR=H   		*/
	LGDP4551_RD_SET();
	LGDP4551_WR_SET();
	_delay_ms(10);								/* wait 10ms     			*/

	LGDP4551_RES_CLR();							/* RES=L, CS=L   			*/
	LGDP4551_CS_CLR();
	_delay_ms(10);								/* wait 10ms     			*/
	
	LGDP4551_RES_SET();						  	/* RES=H					*/
	_delay_ms(100);				    			/* wait 100ms     			*/
}

/**************************************************************************/
/*! 
    Write LCD Command.
*/
/**************************************************************************/
inline void LGDP4551_wr_cmd(uint8_t cmd)
{
	LGDP4551_DC_CLR();							/* DC=L						*/

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	LGDP4551_CMD = 0;
	LGDP4551_WR();
#endif

	LGDP4551_CMD = cmd;							/* cmd(8bit)				*/
	LGDP4551_WR();								/* WR=L->H					*/

	LGDP4551_DC_SET();							/* DC=H						*/
}

/**************************************************************************/
/*! 
    Write LCD Data and GRAM.
*/
/**************************************************************************/
inline void LGDP4551_wr_dat(uint16_t dat)
{
#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	LGDP4551_DATA = (uint8_t)(dat>>8);			/* upper 8bit data			*/
	LGDP4551_WR();								/* WR=L->H					*/
	LGDP4551_DATA = (uint8_t)dat;				/* lower 8bit data			*/
#else
	LGDP4551_DATA = dat;						/* 16bit data 				*/
#endif
	LGDP4551_WR();								/* WR=L->H					*/
}

/**************************************************************************/
/*! 
    Write LCD Block Data.
*/
/**************************************************************************/
inline void LGDP4551_wr_block(uint8_t *p, unsigned int cnt)
{

#ifdef  USE_DISPLAY_DMA_TRANSFER
   DMA_TRANSACTION(p, cnt);
#else

	cnt /= 4;
	
	while (cnt--) {
		/* avoid -Wsequence-point's warning */
		LGDP4551_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
		LGDP4551_wr_dat(*(p+1)|*(p)<<8);
		p++;p++;
	}
#endif

}

/**************************************************************************/
/*! 
    Set Rectangle.
*/
/**************************************************************************/
inline void LGDP4551_rect(uint32_t x, uint32_t width, uint32_t y, uint32_t height)
{

	LGDP4551_wr_cmd(0x50);				/* Horizontal RAM Start ADDR */
	LGDP4551_wr_dat(OFS_COL + x);
	LGDP4551_wr_cmd(0x51);				/* Horizontal RAM End ADDR */
	LGDP4551_wr_dat(OFS_COL + width);
	LGDP4551_wr_cmd(0x52);				/* Vertical RAM Start ADDR */
	LGDP4551_wr_dat(OFS_RAW + y);
	LGDP4551_wr_cmd(0x53);				/* Vertical End ADDR */
	LGDP4551_wr_dat(OFS_RAW + height);

	LGDP4551_wr_cmd(0x20);				/* GRAM Vertical/Horizontal ADDR Set(AD0~AD7) */
	LGDP4551_wr_dat(OFS_COL + x);
	LGDP4551_wr_cmd(0x21);				/* GRAM Vertical/Horizontal ADDR Set(AD8~AD16) */
	LGDP4551_wr_dat(OFS_RAW + y);

	LGDP4551_wr_cmd(0x22);				/* Write Data to GRAM */

}

/**************************************************************************/
/*! 
    Clear Display.
*/
/**************************************************************************/
inline void LGDP4551_clear(void)
{
	volatile uint32_t n;

	LGDP4551_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);

	do {
		LGDP4551_wr_dat(COL_BLACK);
	} while (--n);

}


/**************************************************************************/
/*! 
    Read LCD Register.
*/
/**************************************************************************/
inline uint16_t LGDP4551_rd_cmd(uint16_t cmd)
{
	uint16_t val;

#if defined(GPIO_ACCESS_8BIT) | defined(BUS_ACCESS_8BIT)
	uint16_t temp;
#endif

	LGDP4551_wr_cmd(cmd);
	LGDP4551_WR_SET();

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
    TFT-LCD Module Initialize.
*/
/**************************************************************************/
void LGDP4551_init(void)
{
	uint16_t devicetype;

	Display_IoInit_If();

	LGDP4551_reset();

	/* Check Device Code */
	devicetype = LGDP4551_rd_cmd(0x00);  			/* Confirm Vaild LCD Controller */

	if(devicetype == 0x4551)
	{
		/* Initialize LGDP4551 */
		LGDP4551_wr_cmd(0x15);
		LGDP4551_wr_dat(0x7040);
		LGDP4551_wr_cmd(0x11);
		LGDP4551_wr_dat(0x0110);
		LGDP4551_wr_cmd(0x10);
		LGDP4551_wr_dat(0x3628);
		LGDP4551_wr_cmd(0x12);
		LGDP4551_wr_dat(0x0002);    
		LGDP4551_wr_cmd(0x13);
		LGDP4551_wr_dat(0x0D28);
		_delay_ms(40);
		
		LGDP4551_wr_cmd(0x12);
		LGDP4551_wr_dat(0x0012);	
		_delay_ms(40);
		
		LGDP4551_wr_cmd(0x10);
		LGDP4551_wr_dat(0x3620);
		LGDP4551_wr_cmd(0x13);
		LGDP4551_wr_dat(0x2D26);
		_delay_ms(20);
		
		LGDP4551_wr_cmd(0x30);
		LGDP4551_wr_dat(0x0007);
		LGDP4551_wr_cmd(0x31);
		LGDP4551_wr_dat(0x0502);
		LGDP4551_wr_cmd(0x32);
		LGDP4551_wr_dat(0x0307);
		LGDP4551_wr_cmd(0x33);
		LGDP4551_wr_dat(0x0303);
		LGDP4551_wr_cmd(0x34);
		LGDP4551_wr_dat(0x0004);
		LGDP4551_wr_cmd(0x35);
		LGDP4551_wr_dat(0x0401);
		LGDP4551_wr_cmd(0x36);
		LGDP4551_wr_dat(0x0007);
		LGDP4551_wr_cmd(0x37);
		LGDP4551_wr_dat(0x0403);
		LGDP4551_wr_cmd(0x38);
		LGDP4551_wr_dat(0x0E1E);
		LGDP4551_wr_cmd(0x39);
		LGDP4551_wr_dat(0x0E1E);
		
		LGDP4551_wr_cmd(0x01);
		LGDP4551_wr_dat(0x0100);
		LGDP4551_wr_cmd(0x02);
		LGDP4551_wr_dat(0x0300);
		LGDP4551_wr_cmd(0x03);
		LGDP4551_wr_dat(0x1030);
		
		LGDP4551_wr_cmd(0x08);
		LGDP4551_wr_dat(0x0808);
		
		LGDP4551_wr_cmd(0x0A);
		LGDP4551_wr_dat(0x0008);
		LGDP4551_wr_cmd(0x60);
		LGDP4551_wr_dat(0x3100);
		LGDP4551_wr_cmd(0x61);
		LGDP4551_wr_dat(0x0001); 
		LGDP4551_wr_cmd(0x90);
		LGDP4551_wr_dat(0x0044);
		LGDP4551_wr_cmd(0x92);
		LGDP4551_wr_dat(0x010F);
		LGDP4551_wr_cmd(0x93);
		LGDP4551_wr_dat(0x0701);
		LGDP4551_wr_cmd(0x9A);
		LGDP4551_wr_dat(0x0007);
		LGDP4551_wr_cmd(0xA3);
		LGDP4551_wr_dat(0x0010);
		
		/* display on sequence */
		LGDP4551_wr_cmd(0x07);
		LGDP4551_wr_dat(0x0001);
		LGDP4551_wr_cmd(0x07);
		LGDP4551_wr_dat(0x0021);
		LGDP4551_wr_cmd(0x07);
		LGDP4551_wr_dat(0x0023);
		LGDP4551_wr_cmd(0x07);
		LGDP4551_wr_dat(0x0033);
		LGDP4551_wr_cmd(0x07);
		LGDP4551_wr_dat(0x0133);
	}

	else { for(;;);} /* Invalid Device Code!! */

	LGDP4551_clear();

#if 0	/* test code RED */
	volatile uint32_t n;

	LGDP4551_rect(0,MAX_X-1,0,MAX_Y-1);
	n = (uint32_t)(MAX_X) * (MAX_Y);
	do {
		LGDP4551_wr_dat(COL_RED);
	} while (--n);
	
	_delay_ms(500);
	for(;;);
#endif

}


/* End Of File ---------------------------------------------------------------*/
